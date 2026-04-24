// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/HexagonServer.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Data/LevelData.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/ProtocolVersion.hpp"
#include "SSVOpenHexagon/Global/StringHash.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Online/Database.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/LevelValidator.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVOpenHexagon/Utils/Split.hpp"
#include "SSVOpenHexagon/Utils/StringToCharVec.hpp"
#include "SSVOpenHexagon/Utils/Timestamp.hpp"
#include "SSVOpenHexagon/Utils/VectorToSet.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/MiniPFR.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringStreamOp.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/Vector.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>

static auto& slog(const char* funcName)
{
    return ::hg::lo(::hg::Utils::concat("hg::HexagonServer::", funcName));
}

#define SSVOH_SLOG ::slog(__func__)

#define SSVOH_SLOG_VERBOSE \
    if (_verbose)          \
    ::slog(__func__)

#define SSVOH_SLOG_ERROR ::slog(__func__) << "[ERROR] "

#define SSVOH_SLOG_VAR(x) '\'' << #x << "': '" << (x) << '\''

#define SSVOH_CLOG_VAR_IP(x) '\'' << #x << "': '" << ::sf::IpAddressUtils::toString(x) << '\''

namespace hg
{

HexagonServer::ConnectedClient::ConnectedClient(const Utils::SCTimePoint lastActivity, sf::TcpSocket&& socket) :
    _socket{std::move(socket)},
    _lastActivity{lastActivity},
    _consecutiveFailures{0},
    _mustDisconnect{false},
    _clientPublicKey{},
    _loginData{},
    _state{State::Disconnected}
{
}

template <typename... Ts>
[[nodiscard]] bool HexagonServer::fail(const Ts&... xs)
{
    if constexpr (sizeof...(Ts) > 0)
    {
        auto& stream = SSVOH_SLOG_ERROR;
        (stream << ... << xs);
        stream << '\n';
    }

    return false;
}

[[nodiscard]] bool HexagonServer::isLevelSupported(const sf::base::String& levelValidator) const
{
    return _supportedLevelValidators.contains(levelValidator);
}

[[nodiscard]] bool HexagonServer::initializeControlSocket()
{
    SSVOH_SLOG << "Initializing UDP control socket...\n";

    if (_controlSocket.bind(_serverControlPort, sf::IpAddress::LocalHost) != sf::Socket::Status::Done)
    {
        return fail("Failure binding UDP control socket");
    }

    if (!_socketSelector.add(_controlSocket))
    {
        return fail("Failed to add UDP control to socket selector");
    }

    return true;
}

[[nodiscard]] bool HexagonServer::initializeTcpListener()
{
    SSVOH_SLOG << "Initializing TCP listener...\n";

    _listener = sf::TcpListener::create(_serverPort, true /* isBlocking */);
    if (!_listener.hasValue())
    {
        return fail("Failure initializing TCP listener");
    }

    return true;
}

[[nodiscard]] bool HexagonServer::initializeSocketSelector()
{
    SSVOH_SLOG << "Initializing socket selector...\n";

    if (!_socketSelector.add(*_listener))
    {
        return fail("Failed to add listener to socket selector");
    }

    return true;
}

[[nodiscard]] bool HexagonServer::sendPacket(ConnectedClient& c, sf::Packet& p)
{
    // In non-blocking mode, `send` may return `Partial` if the TCP send buffer
    // could not take the whole packet; the same packet must be retried before
    // any other send to keep the stream uncorrupted.
    for (int tries = 0; tries < 5; ++tries)
    {
        const auto status = c._socket.send(p);

        if (status == sf::Socket::Status::Done)
        {
            return true;
        }

        if (status != sf::Socket::Status::Partial)
        {
            return fail("Failure sending packet");
        }
    }

    return fail("Failure sending packet, too many partial sends");
}

template <typename T>
[[nodiscard]] bool HexagonServer::sendEncrypted(ConnectedClient& c, const T& data)
{
    const void* clientAddr = static_cast<void*>(&c);

    if (!c._rtKeys.hasValue())
    {
        return fail("Tried to send encrypted message without RT keys for client '", clientAddr, '\'');
    }

    if (!makeServerToClientEncryptedPacket(c._rtKeys->keyTransmit, _packetBuffer, data))
    {
        return fail("Error building encrypted message packet for client '", clientAddr, '\'');
    }

    return sendPacket(c, _packetBuffer);
}

[[nodiscard]] bool HexagonServer::sendKick(ConnectedClient& c)
{
    makeServerToClientPacket(_packetBuffer, STCPKick{});
    return sendPacket(c, _packetBuffer);
}

[[nodiscard]] bool HexagonServer::sendPublicKey(ConnectedClient& c)
{
    makeServerToClientPacket(_packetBuffer, STCPPublicKey{.key = _serverPSKeys.keyPublic});

    return sendPacket(c, _packetBuffer);
}

[[nodiscard]] bool HexagonServer::sendRegistrationSuccess(ConnectedClient& c)
{
    return sendEncrypted(c, STCPRegistrationSuccess{});
}

[[nodiscard]] bool HexagonServer::sendRegistrationFailure(ConnectedClient& c, const sf::base::String& error)
{
    return sendEncrypted(c, STCPRegistrationFailure{.error = std::string(error.cStr())});
}

[[nodiscard]] bool HexagonServer::sendLoginSuccess(ConnectedClient&        c,
                                                   const sf::base::U64     loginToken,
                                                   const sf::base::String& loginName)
{
    return sendEncrypted(c, //
                         STCPLoginSuccess{
                             .loginToken = static_cast<sf::base::U64>(loginToken), //
                             .loginName  = std::string(loginName.cStr())           //
                         } //
    );
}

[[nodiscard]] bool HexagonServer::sendLoginFailure(ConnectedClient& c, const sf::base::String& error)
{
    return sendEncrypted(c, STCPLoginFailure{.error = std::string(error.cStr())});
}

[[nodiscard]] bool HexagonServer::sendLogoutSuccess(ConnectedClient& c)
{
    return sendEncrypted(c, STCPLogoutSuccess{});
}

[[nodiscard]] bool HexagonServer::sendLogoutFailure(ConnectedClient& c)
{
    return sendEncrypted(c, STCPLogoutFailure{});
}

[[nodiscard]] bool HexagonServer::sendDeleteAccountSuccess(ConnectedClient& c)
{
    return sendEncrypted(c, STCPDeleteAccountSuccess{});
}

[[nodiscard]] bool HexagonServer::sendDeleteAccountFailure(ConnectedClient& c, const sf::base::String& error)
{
    return sendEncrypted(c, STCPDeleteAccountFailure{.error = std::string(error.cStr())});
}

[[nodiscard]] bool HexagonServer::sendTopScores(ConnectedClient&                                  c,
                                                const sf::base::String&                           levelValidator,
                                                const sf::base::Vector<Database::ProcessedScore>& scores)
{
    return sendEncrypted(c, //
                         STCPTopScores{
                             .levelValidator = std::string(levelValidator.cStr()), //
                             .scores         = scores                              //
                         } //
    );
}

[[nodiscard]] bool HexagonServer::sendOwnScore(ConnectedClient&                c,
                                               const sf::base::String&         levelValidator,
                                               const Database::ProcessedScore& score)
{
    return sendEncrypted(c, //
                         STCPOwnScore{
                             .levelValidator = std::string(levelValidator.cStr()), //
                             .score          = score                               //
                         } //
    );
}

[[nodiscard]] bool HexagonServer::sendTopScoresAndOwnScore(
    ConnectedClient&                                    c,
    const sf::base::String&                             levelValidator,
    const sf::base::Vector<Database::ProcessedScore>&   scores,
    const sf::base::Optional<Database::ProcessedScore>& ownScore)
{
    return sendEncrypted(c, //
                         STCPTopScoresAndOwnScore{
                             .levelValidator = std::string(levelValidator.cStr()), //
                             .scores         = scores,                             //
                             .ownScore       = ownScore                            //
                         } //
    );
}

[[nodiscard]] bool HexagonServer::sendServerStatus(ConnectedClient&                          c,
                                                   const ProtocolVersion&                    protocolVersion,
                                                   const GameVersion&                        gameVersion,
                                                   const sf::base::Vector<sf::base::String>& supportedLevelValidators)
{
    sf::base::Vector<std::string> convertedValidators;
    convertedValidators.reserve(supportedLevelValidators.size());
    for (const auto& s : supportedLevelValidators)
    {
        convertedValidators.emplaceBack(std::string(s.cStr()));
    }

    return sendEncrypted(c, //
                         STCPServerStatus{
                             .protocolVersion          = protocolVersion,    //
                             .gameVersion              = gameVersion,        //
                             .supportedLevelValidators = convertedValidators //
                         } //
    );
}

bool HexagonServer::kickAndRemoveClient(ConnectedClient& c)
{
    (void)sendKick(c);

    if (c._loginData.hasValue())
    {
        Database::removeAllLoginTokensForUser(c._loginData->_userId);
    }

    if (!_socketSelector.remove(c._socket))
    {
        return fail("Failed to remove client socket from socket selector");
    }

    return true;
}

void HexagonServer::run()
{
    while (_running)
    {
        try
        {
            runIteration();
        } catch (const std::runtime_error& e)
        {
            SSVOH_SLOG_ERROR << "Exception: '" << e.what() << "'\n";
        } catch (...)
        {
            SSVOH_SLOG_ERROR << "Unknown exception";
        }
    }
}

void HexagonServer::runIteration()
{
    SSVOH_SLOG_VERBOSE << "New iteration...\n";

    if (_socketSelector.wait(sf::seconds(10)) && _running && _listener.hasValue())
    {
        // A timeout is specified so that we can purge clients even if we didn't
        // receive anything.

        // Note on ordering: `LoopOverSockets` iterates the selector's ready
        // span directly (O(M)), which is invalidated by any subsequent call
        // to `add` / `remove` / `clear`. `runIteration_TryAcceptingNewClient`
        // adds the new client socket to the selector, so it must run AFTER
        // `LoopOverSockets`. `Control` only reads and doesn't touch the
        // selector, so its relative position doesn't matter.
        runIteration_Control();
        runIteration_LoopOverSockets();
        runIteration_TryAcceptingNewClient();
    }

    runIteration_PurgeClients();
    runIteration_PurgeTokens();
    runIteration_FlushLogs();
}

bool HexagonServer::runIteration_Control()
{
    if (!_socketSelector.isReady(_controlSocket))
    {
        return fail();
    }

    sf::base::Optional<sf::IpAddress> senderIp;
    unsigned short                    senderPort;

    if (_controlSocket.receive(_packetBuffer, senderIp, senderPort) != sf::Socket::Status::Done)
    {
        return fail("Failure receiving control packet");
    }

    std::string controlMsg;

    if (!(_packetBuffer >> controlMsg))
    {
        return fail("Failure decoding control packet");
    }

    SSVOH_ASSERT(senderIp.hasValue());

    SSVOH_SLOG << "Received control packet from '" << sf::IpAddressUtils::toString(senderIp.value()) << ':'
               << senderPort << "', contents: '" << controlMsg << "'\n";

    if (controlMsg.empty())
    {
        return true;
    }

    const auto splitted = Utils::split<sf::base::String>(controlMsg);

    if (splitted.empty())
    {
        return true;
    }

    if (splitted[0] == "verbose")
    {
        if (splitted.size() != 2)
        {
            SSVOH_SLOG_ERROR << "'verbose' command must be followed by 'true' or 'false'\n";

            return true;
        }

        if (splitted[1] == "true")
        {
            SSVOH_SLOG << "Enabled verbose mode\n";

            _verbose = true;
            return true;
        }

        if (splitted[1] == "false")
        {
            SSVOH_SLOG << "Disabled verbose mode\n";

            _verbose = false;
            return true;
        }
    }

// TODO (P1): conditionally enable in debug mode
#if 0
    if(splitted[0] == "db")
    {
        if(splitted.size() < 2)
        {
            SSVOH_SLOG_ERROR << "'db' command must be followed by 'exec'\n";

            return true;
        }
        if(splitted[1] == "exec")
        {
            if(splitted.size() < 3)
            {
                SSVOH_SLOG_ERROR << "'db exec' command must be followed by a "
                                    "sqlite command\n";

                return true;
            }

            sf::base::String query = splitted[2];
            for(sf::base::SizeT i = 3; i < splitted.size(); ++i)
            {
                query += ' ';
                query += splitted[i];
            }

            const sf::base::Optional<sf::base::String> executeOutcome =
                Database::execute(query);

            if(executeOutcome.hasValue())
            {
                SSVOH_SLOG_ERROR << "'db exec' error:\n"
                                 << *executeOutcome << '\n';
            }
        }
    }
#endif

    return true;
}

bool HexagonServer::runIteration_TryAcceptingNewClient()
{
    if (!_socketSelector.isReady(*_listener))
    {
        return false;
    }

    SSVOH_SLOG << "Listener is ready, attempting to accept new client\n";

    // TODO (P1): potential hanging spot?
    // The listener is ready: there is a pending connection
    sf::TcpListener::AcceptResult acceptResult = _listener->accept();

    if (acceptResult.status != sf::Socket::Status::Done || !acceptResult.socket.hasValue())
    {
        SSVOH_SLOG << "Listener failed to accept new client\n";
        return false;
    }

    ConnectedClient& potentialClient = _connectedClients.emplace_back(Utils::SCClock::now(), std::move(*acceptResult.socket));

    // Non-blocking: receive() returns NotReady on partial packets instead of
    // hanging the single-threaded loop waiting for the rest of a packet whose
    // sender stopped mid-stream. TcpSocket internally buffers partial data.
    potentialClient._socket.setBlocking(false);

    const void* potentialClientAddress = static_cast<void*>(&potentialClient);

    SSVOH_SLOG << "Listener accepted new client '" << potentialClientAddress << "'\n";

    potentialClient._state = ConnectedClient::State::Connected;

    // Attach `&potentialClient` as the selector's `userData` so the ready
    // list we iterate in `runIteration_LoopOverSockets` can recover the
    // owning `ConnectedClient` via a single `static_cast` — no side-table.
    if (!_socketSelector.add(potentialClient._socket, &potentialClient))
    {
        return fail("Failed to add potential client socket to socket selector");
    }

    return true;
}

void HexagonServer::runIteration_LoopOverSockets()
{
    // Iterate only the sockets the selector actually reported ready (O(M)
    // rather than O(N)). Each client socket was registered with its owning
    // `ConnectedClient*` as `userData`, so we recover the owner with a
    // single cast -- no side-table needed. Entries whose `userData` is
    // null belong to the listener or the control socket; both are handled
    // via `isReady` point-checks elsewhere, so we skip them here.
    for (const sf::SocketSelector::ReadyEntry& entry : _socketSelector.getReadyToReceive())
    {
        if (entry.userData == nullptr)
        {
            continue;
        }

        ConnectedClient& connectedClient = *static_cast<ConnectedClient*>(entry.userData);
        const void*      clientAddr      = static_cast<void*>(&connectedClient);

        SSVOH_SLOG_VERBOSE << "Client '" << clientAddr << "' has sent data\n ";

        const auto status = connectedClient._socket.receive(_packetBuffer); // clears the packet buffer internally

        // Partial packet: `TcpSocket::m_pendingPacket` has buffered what
        // arrived so far; the next selector wakeup will resume where we left
        // off. Do not count as a failure; the inactivity timeout still drops
        // clients that never complete a packet.
        if (status == sf::Socket::Status::NotReady)
        {
            continue;
        }

        if (status == sf::Socket::Status::Done)
        {
            SSVOH_SLOG_VERBOSE << "Successfully received data from client '" << clientAddr << "'\n";

            if (processPacket(connectedClient, _packetBuffer))
            {
                connectedClient._lastActivity        = Utils::SCClock::now();
                connectedClient._consecutiveFailures = 0;

                continue;
            }
        }

        // Disconnected / Error / processPacket failed
        SSVOH_SLOG_VERBOSE << "Failed to receive data from client '" << clientAddr
                           << "' (consecutive failures: " << connectedClient._consecutiveFailures << ")\n";

        ++connectedClient._consecutiveFailures;

        constexpr int maxConsecutiveFailures = 5;
        if (connectedClient._consecutiveFailures == maxConsecutiveFailures)
        {
            SSVOH_SLOG << "Too many consecutive failures for client '" << clientAddr << "', marking for disconnect\n";

            // Mark the client; `runIteration_PurgeClients` will call
            // `kickAndRemoveClient` and erase it from `_connectedClients`.
            // Doing the removal here would invalidate the ready span we are
            // currently iterating.
            connectedClient._mustDisconnect = true;
        }
    }
}

void HexagonServer::runIteration_PurgeClients()
{
    constexpr std::chrono::duration maxInactivity = std::chrono::seconds(60);

    const Utils::SCTimePoint now = Utils::SCClock::now();

    for (auto it = _connectedClients.begin(); it != _connectedClients.end(); ++it)
    {
        ConnectedClient& connectedClient = *it;
        const void*      clientAddr      = static_cast<void*>(&connectedClient);

        if (connectedClient._mustDisconnect)
        {
            SSVOH_SLOG << "Client '" << clientAddr << "' disconnected, removing from list\n";

            if (!kickAndRemoveClient(connectedClient))
            {
                SSVOH_SLOG << "Failed kicking client after must disconnect\n";
            }

            it = _connectedClients.erase(it);
            continue;
        }

        if (now - connectedClient._lastActivity > maxInactivity)
        {
            SSVOH_SLOG << "Client '" << clientAddr << "' timed out, removing from list\n";

            if (!kickAndRemoveClient(connectedClient))
            {
                SSVOH_SLOG << "Failed kicking client after inactivity\n";
            }

            it = _connectedClients.erase(it);
            continue;
        }
    }
}

template <typename TDuration>
[[nodiscard]] static bool checkAndUpdateLastElapsed(Utils::SCTimePoint& last, const TDuration duration)
{
    if (Utils::SCClock::now() - last < duration)
    {
        return false;
    }

    last = Utils::SCClock::now();
    return true;
}

void HexagonServer::runIteration_PurgeTokens()
{
    if (!checkAndUpdateLastElapsed(_lastTokenPurge, std::chrono::seconds(3600) /* 1 hour */))
    {
        return;
    }

    SSVOH_SLOG_VERBOSE << "Purging old login tokens\n";

    for (const Database::LoginToken& lt : Database::getAllStaleLoginTokens())
    {
        SSVOH_SLOG << "Found stale token for user '" << lt.userId << "'\n";

        for (auto it = _connectedClients.begin(); it != _connectedClients.end(); ++it)
        {
            ConnectedClient& c          = *it;
            const void*      clientAddr = static_cast<void*>(&c);

            if (!c._loginData.hasValue())
            {
                continue;
            }

            if (c._loginData->_userId == lt.userId)
            {
                SSVOH_SLOG << "Kicking stale token client '" << clientAddr << "'\n";

                if (!kickAndRemoveClient(c))
                {
                    SSVOH_SLOG << "Failed kicking client with stale token\n";
                }

                it = _connectedClients.erase(it);
            }
        }
    }

    Database::removeAllStaleLoginTokens();
}

void HexagonServer::runIteration_FlushLogs()
{
    if (!checkAndUpdateLastElapsed(_lastLogsFlush, std::chrono::seconds(1)))
    {
        return;
    }

    std::cout.flush();
    std::cerr.flush();
    hg::lo().flush();
}

[[nodiscard]] bool HexagonServer::validateLogin(ConnectedClient& c, const char* context, const sf::base::U64 ctspLoginToken)
{
    const void* clientAddr = static_cast<void*>(&c);

    if (!c._loginData.hasValue())
    {
        SSVOH_SLOG << "Client '" << clientAddr << "', is not logged in for " << context << '\n';

        return false;
    }

    const auto cLoginToken = c._loginData->_loginToken;

    if (cLoginToken != ctspLoginToken)
    {
        SSVOH_SLOG << "Client '" << clientAddr << "' login token mismatch for " << context << '\n';

        return false;
    }

    return true;
}

[[nodiscard]] bool HexagonServer::processReplay(ConnectedClient& c, const sf::base::U64 loginToken, const replay_file& rf)
{
    const void* clientAddr = static_cast<void*>(&c);

    const Utils::SCTimePoint receiveTime = Utils::SCClock::now();

    if (!validateLogin(c, "replay", loginToken))
    {
        return true;
    }

    const auto discard = [&](const auto&... reason)
    {
        SSVOH_SLOG << "Discarding replay from client '" << clientAddr << "', " << Utils::concat(reason...)
                   << ", replay time was " << rf.played_seconds() << "s\n";

        return true;
    };

    if (_assets == nullptr || _hexagonGame == nullptr)
    {
        return discard("server built without replay-processing dependencies");
    }

    if (!c._gameStatus.hasValue())
    {
        return discard("no game started");
    }

    if (!_assets->isValidPackId(rf._pack_id))
    {
        return discard("invalid pack id '", rf._pack_id, '\'');
    }

    if (!_assets->isValidLevelId(rf._level_id))
    {
        return discard("invalid level id '", rf._level_id, '\'');
    }

    const LevelData& levelData = _assets->getLevelData(rf._level_id);

    if (levelData.unscored)
    {
        return discard("unscored level id '", rf._level_id, '\'');
    }

    const auto lv = Utils::getLevelValidator(rf._level_id, rf._difficulty_mult); // TODO

    const sf::base::String levelValidator{lv.data(), lv.size()};

    SSVOH_SLOG << "Processing replay from client '" << clientAddr << "' for level '" << levelValidator << "'\n";

    constexpr int maxProcessingSeconds = 5;

    const sf::base::Optional<HexagonGame::GameExecutionResult>
        ger = _hexagonGame->runReplayUntilDeathAndGetScore(rf, maxProcessingSeconds, 1.f /* timescale */);

    if (!ger.hasValue())
    {
        return discard("max processing time exceeded (", maxProcessingSeconds, "s)");
    }

    const double replayTotalTime  = ger->totalTimeSeconds;
    const double replayPlayedTime = ger->playedTimeSeconds;

    SSVOH_SLOG << "Replay processed, final time: '" << replayTotalTime << "'\n";

    const double elapsedSecs = std::chrono::duration_cast<std::chrono::duration<double>>(receiveTime - c._gameStatus->_startTP)
                                   .count();

    const double difference = std::fabs(replayTotalTime - elapsedSecs);
    const double ratio      = replayTotalTime / elapsedSecs;

    const bool goodDifference = difference < 5.0;
    const bool goodRatio      = ratio > 0.65 && ratio < 1.35;

    const auto printDifferenceAndRatio = [&]
    {
        SSVOH_SLOG << "Elapsed request time: " << elapsedSecs << '\n'
                   << "Difference: " << difference << '\n'
                   << "Ratio: " << ratio << '\n';
    };

    if (!goodDifference)
    {
        printDifferenceAndRatio();
        return discard("difference too large");
    }

    if (!goodRatio)
    {
        printDifferenceAndRatio();
        return discard("bad ratio");
    }

    SSVOH_SLOG << "Replay valid, adding to database\n";

    SSVOH_ASSERT(c._loginData.hasValue());

    Database::addScore(levelValidator, Utils::nowTimestamp(), c._loginData->_steamId, replayPlayedTime);

    return true;
}

template <typename T>
void HexagonServer::printCTSPDataVerbose(ConnectedClient& c, const char* title, const T& ctsp)
{
    if (!_verbose)
    {
        return;
    }

    const auto stringify = []<typename U>(const U& field) -> decltype(auto)
    {
        if constexpr (SFML_BASE_IS_SAME(U, SodiumPublicKeyArray))
        {
            return sodiumKeyToString(field);
        }
        else if constexpr (SFML_BASE_IS_SAME(U, replay_file))
        {
            return "<REPLAY_FILE>";
        }
        else if constexpr (SFML_BASE_IS_SAME(U, compressed_replay_file))
        {
            return "<COMPRESSED_REPLAY_FILE>";
        }
        else if constexpr (SFML_BASE_IS_SAME(U, sf::base::String))
        {
            return field;
        }
        else if constexpr (SFML_BASE_IS_SAME(U, std::string))
        {
            return field;
        }
        else
        {
            return std::to_string(field);
        }
    };

    auto& stream = SSVOH_SLOG;

    const void* clientAddr = static_cast<void*>(&c);

    stream << "Received '" << title << "' packet from client '" << clientAddr << "', contents: {";

    constexpr sf::base::SizeT nFields = sf::base::minipfr::numFields<T>;
    if constexpr (nFields > 0)
    {
        sf::base::SizeT i = 0;
        sf::base::minipfr::forEachField(ctsp,
                                        [&](const auto& field)
        {
            stream << stringify(field);

            if (i != nFields - 1)
            {
                stream << ", ";
            }

            ++i;
        });
    }

    stream << "}\n";
}

[[nodiscard]] bool HexagonServer::processPacket(ConnectedClient& c, sf::Packet& p)
{
    const void* clientAddr = static_cast<void*>(&c);

    constexpr int topScoresLimit = 6;

    _errorOss.setStr("");
    const PVClientToServer pv = decodeClientToServerPacket(c._rtKeys.hasValue() ? &c._rtKeys->keyReceive : nullptr, _errorOss, p);

    const auto checkState = [&](const ConnectedClient::State state)
    {
        if (c._state != state)
        {
            SSVOH_SLOG_VERBOSE << "Invalid client state, expected '" << static_cast<int>(state) << "', state was '"
                               << static_cast<int>(c._state) << "''\n";

            return false;
        }

        return true;
    };

    const auto checkState2 = [&](const ConnectedClient::State state0, const ConnectedClient::State state1)
    {
        if (c._state != state0 && c._state != state1)
        {
            SSVOH_SLOG_VERBOSE << "Invalid client state, expected '" << static_cast<int>(state0) << "' or '"
                               << static_cast<int>(state1) << "', state was '" << static_cast<int>(c._state) << "''\n";

            return false;
        }

        return true;
    };

    return pv.linearMatch( //

        [&](const PInvalid&)
    { return fail("Error processing packet from client '", clientAddr, "', details: ", _errorOss.getString()); },

        [&](const PEncryptedMsg&)
    { return fail("Received non-decrypted encrypted msg packet from client '", clientAddr, '\''); },

        [&](const CTSPHeartbeat&) { return true; },

        [&](const CTSPDisconnect& ctsp)
    {
        printCTSPDataVerbose(c, "disconnect", ctsp);

        c._mustDisconnect = true;
        c._state          = ConnectedClient::State::Disconnected;
        return true;
    },

        [&](const CTSPPublicKey& ctsp)
    {
        printCTSPDataVerbose(c, "public key", ctsp);

        if (c._clientPublicKey.hasValue())
        {
            SSVOH_SLOG_VERBOSE << "Already had public key, replacing\n";
        }
        else
        {
            SSVOH_SLOG_VERBOSE << "Did not have public key, setting\n";
        }

        c._clientPublicKey.emplace(ctsp.key);

        SSVOH_SLOG_VERBOSE << "Client public key: '" << sodiumKeyToString(ctsp.key) << "'\n";

        SSVOH_SLOG_VERBOSE << "Calculating RT keys\n";
        c._rtKeys = calculateServerSessionSodiumRTKeys(_serverPSKeys, ctsp.key);

        if (!c._rtKeys.hasValue())
        {
            SSVOH_SLOG_ERROR << "Failed calculating RT keys, disconnecting client '" << clientAddr << "'\n";

            c._mustDisconnect = true;
            (void)sendKick(c);

            return false;
        }

        const auto keyReceive  = sodiumKeyToString(c._rtKeys->keyReceive);
        const auto keyTransmit = sodiumKeyToString(c._rtKeys->keyTransmit);

        SSVOH_SLOG_VERBOSE << "Calculated RT keys\n"
                           << " - " << SSVOH_SLOG_VAR(keyReceive) << '\n'
                           << " - " << SSVOH_SLOG_VAR(keyTransmit) << '\n';

        SSVOH_SLOG << "Replying with own public key\n";
        return sendPublicKey(c);
    },

        [&](const CTSPRegister& ctsp)
    {
        printCTSPDataVerbose(c, "register", ctsp);

        const auto& [steamId, name, passwordHash] = ctsp;

        const auto sendFail = [&](const auto&... xs)
        {
            const sf::base::String errorStr = Utils::concat(xs...);

            SSVOH_SLOG << errorStr << '\n';
            return sendRegistrationFailure(c, errorStr);
        };

        if (!checkState(ConnectedClient::State::Connected))
        {
            return sendFail("Client not in connected state");
        }

        if (name.size() > 32)
        {
            return sendFail("Name too long, max is 32 characters");
        }

        if (Database::anyUserWithSteamId(steamId))
        {
            return sendFail("User with steamId '", steamId, "' already registered");
        }

        if (Database::anyUserWithName(sf::base::String(name)))
        {
            return sendFail("User with name '", name, "' already registered");
        }

        Database::addUser( //
            Database::User{
                .steamId      = steamId,
                .name         = name,
                .passwordHash = Utils::stringToCharVec(sf::base::String(passwordHash)) //
            } //
        );

        SSVOH_SLOG << "Successfully registered\n";
        return sendRegistrationSuccess(c);
    },

        [&](const CTSPLogin& ctsp)
    {
        printCTSPDataVerbose(c, "login", ctsp);

        const auto& [steamId, name, passwordHash] = ctsp;

        const auto sendFail = [&](const auto&... xs)
        {
            const sf::base::String errorStr = Utils::concat(xs...);

            SSVOH_SLOG << errorStr << '\n';
            return sendLoginFailure(c, errorStr);
        };

        if (!checkState(ConnectedClient::State::Connected))
        {
            return sendFail("Client not in connected state");
        }

        if (name.size() > 32)
        {
            return sendFail("Name too long, max is 32 characters");
        }

        if (!Database::anyUserWithSteamId(steamId))
        {
            return sendFail("No user with steamId '", steamId, "' registered");
        }

        if (!Database::anyUserWithName(sf::base::String(name)))
        {
            return sendFail("No user with name '", name, "' registered");
        }

        const sf::base::Optional<Database::User> user = Database::getUserWithSteamIdAndName(steamId, sf::base::String(name));

        if (!user.hasValue())
        {
            return sendFail("No user matching '", steamId, "' and '", name, "' registered");
        }

        SSVOH_ASSERT(user.hasValue());

        if (user->passwordHash != Utils::stringToCharVec(sf::base::String(passwordHash)))
        {
            return sendFail("Invalid password for user matching '", steamId, "' and '", name, '\'');
        }

        SSVOH_SLOG << "Creating login token for user\n";

        const sf::base::U64 loginToken = randomUInt64();

        Database::removeAllLoginTokensForUser(user->id);

        Database::addLoginToken( //
            Database::LoginToken{
                .userId    = user->id,
                .timestamp = Utils::nowTimestamp(),
                .token     = loginToken //
            });

        c._loginData.emplace(ConnectedClient::LoginData{
            ._userId       = user->id,
            ._steamId      = steamId,
            ._name         = sf::base::String(name),
            ._passwordHash = sf::base::String(passwordHash),
            ._loginToken   = loginToken //
        });

        c._state = ConnectedClient::State::LoggedIn;

        SSVOH_SLOG << "Successfully logged in\n";
        return sendLoginSuccess(c, loginToken, sf::base::String(user->name));
    },

        [&](const CTSPLogout& ctsp)
    {
        printCTSPDataVerbose(c, "logout", ctsp);

        if (!checkState2(ConnectedClient::State::LoggedIn, ConnectedClient::State::LoggedIn_Ready))
        {
            return true;
        }

        const sf::base::Optional<Database::User> user = Database::getUserWithSteamId(ctsp.steamId);

        if (!user.hasValue())
        {
            SSVOH_SLOG << "No user with steamId '" << ctsp.steamId << "'\n";
            return sendLogoutFailure(c);
        }

        SSVOH_ASSERT(user.hasValue());

        Database::removeAllLoginTokensForUser(user->id);

        c._loginData.reset();
        c._state = ConnectedClient::State::Connected;

        return sendLogoutSuccess(c);
    },

        [&](const CTSPDeleteAccount& ctsp)
    {
        printCTSPDataVerbose(c, "delete account", ctsp);

        const auto& [steamId, passwordHash] = ctsp;

        if (!checkState(ConnectedClient::State::Connected))
        {
            return true;
        }

        const auto sendFail = [&](const auto&... xs)
        {
            const sf::base::String errorStr = Utils::concat(xs...);

            SSVOH_SLOG << errorStr << '\n';
            return sendDeleteAccountFailure(c, errorStr);
        };

        if (!Database::anyUserWithSteamId(steamId))
        {
            return sendFail("No user with steamId '", steamId, "' registered");
        }

        const sf::base::Optional<Database::User> user = Database::getUserWithSteamId(ctsp.steamId);

        if (!user.hasValue())
        {
            return sendFail("No user with steamId '", ctsp.steamId, '\'');
        }

        SSVOH_ASSERT(user.hasValue());

        if (user->passwordHash != Utils::stringToCharVec(sf::base::String(passwordHash)))
        {
            return sendFail("Invalid password for user matching '", steamId, '\'');
        }

        Database::removeAllLoginTokensForUser(user->id);
        Database::removeUser(user->id);

        SSVOH_SLOG << "Successfully deleted account\n";
        return sendDeleteAccountSuccess(c);
    },

        [&](const CTSPRequestTopScores& ctsp)
    {
        printCTSPDataVerbose(c, "request top scores", ctsp);

        if (!checkState(ConnectedClient::State::LoggedIn_Ready) || !validateLogin(c, "top scores", ctsp.loginToken))
        {
            return true;
        }

        const sf::base::String lvSfStr(ctsp.levelValidator);

        if (!isLevelSupported(lvSfStr))
        {
            return true;
        }

        SSVOH_SLOG_VERBOSE << "Sending top " << topScoresLimit << " scores to client '" << clientAddr << "'\n";

        return sendTopScores(c, lvSfStr, Database::getTopScores(topScoresLimit, lvSfStr));
    },

        [&](const CTSPReplay& ctsp)
    {
        printCTSPDataVerbose(c, "replay", ctsp);

        if (!checkState(ConnectedClient::State::LoggedIn_Ready))
        {
            return true;
        }

        const auto& [loginToken, rf] = ctsp;
        return processReplay(c, loginToken, rf);
    },

        [&](const CTSPRequestOwnScore& ctsp)
    {
        printCTSPDataVerbose(c, "request own score", ctsp);

        if (!checkState(ConnectedClient::State::LoggedIn_Ready) || !validateLogin(c, "own score", ctsp.loginToken))
        {
            return true;
        }

        const sf::base::String lvSfStr2(ctsp.levelValidator);

        if (!isLevelSupported(lvSfStr2))
        {
            return true;
        }

        const sf::base::Optional<Database::ProcessedScore> ps = Database::getScore(lvSfStr2, c._loginData->_steamId);

        if (!ps.hasValue())
        {
            return true;
        }

        SSVOH_SLOG_VERBOSE << "Sending own score to client '" << clientAddr << "'\n";

        return sendOwnScore(c, lvSfStr2, *ps);
    },

        [&](const CTSPRequestTopScoresAndOwnScore& ctsp)
    {
        printCTSPDataVerbose(c, "request top scores and own score", ctsp);

        if (!checkState(ConnectedClient::State::LoggedIn_Ready) ||
            !validateLogin(c, "top scores and own scores", ctsp.loginToken))
        {
            return true;
        }

        const sf::base::String lv(ctsp.levelValidator);

        if (!isLevelSupported(lv))
        {
            return true;
        }

        SSVOH_SLOG_VERBOSE << "Sending top " << topScoresLimit << " scores and own score to client '" << clientAddr << "'\n";

        return sendTopScoresAndOwnScore(c,
                                        lv,
                                        Database::getTopScores(topScoresLimit, lv),
                                        Database::getScore(lv, c._loginData->_steamId));
    },

        [&](const CTSPStartedGame& ctsp)
    {
        printCTSPDataVerbose(c, "started game", ctsp);

        if (!checkState(ConnectedClient::State::LoggedIn_Ready) || !validateLogin(c, "started game", ctsp.loginToken))
        {
            return true;
        }

        const sf::base::String lv(ctsp.levelValidator);

        SSVOH_SLOG << "Client '" << clientAddr << "' started game for level '" << lv << "'\n";

        c._gameStatus.emplace(ConnectedClient::GameStatus{
            ._startTP        = Utils::SCClock::now(), //
            ._levelValidator = lv                     //
        });

        return true;
    },

        [&](const CTSPCompressedReplay& ctsp)
    {
        printCTSPDataVerbose(c, "compressed replay", ctsp);

        if (!checkState(ConnectedClient::State::LoggedIn_Ready))
        {
            return true;
        }

        const auto& [loginToken, crf] = ctsp;

        const sf::base::Optional<replay_file> rfOpt = decompress_replay_file(crf);

        if (!rfOpt.hasValue())
        {
            SSVOH_SLOG_ERROR << "Failed to decompress replay received from client '" << clientAddr << "'\n";

            return false;
        }

        return processReplay(c, loginToken, rfOpt.value());
    },

        [&](const CTSPRequestServerStatus& ctsp)
    {
        printCTSPDataVerbose(c, "request server status", ctsp);

        if (!checkState(ConnectedClient::State::LoggedIn) || !validateLogin(c, "request server status", ctsp.loginToken))
        {
            return true;
        }

        return sendServerStatus(c, PROTOCOL_VERSION, GAME_VERSION, _supportedLevelValidatorsVector);
    },

        [&](const CTSPReady& ctsp)
    {
        printCTSPDataVerbose(c, "ready", ctsp);

        if (!checkState(ConnectedClient::State::LoggedIn) || !validateLogin(c, "ready", ctsp.loginToken))
        {
            return true;
        }

        c._state = ConnectedClient::State::LoggedIn_Ready;
        return true;
    }

        //
    );
}

[[nodiscard]] static std::unordered_set<sf::base::String> makeSupportedLevelValidators(
    HGAssets*                                   assets,
    const std::unordered_set<sf::base::String>& levelValidatorWhitelist)
{
    std::unordered_set<sf::base::String> result;

    if (assets == nullptr)
    {
        return result;
    }

    for (const auto& [assetId, ld] : assets->getLevelDatas())
    {
        if (ld.unscored)
        {
            continue;
        }

        for (const float dm : ld.difficultyMults)
        {
            if (const sf::base::String& validator = ld.getValidator(dm); levelValidatorWhitelist.contains(validator))
            {
                result.emplace(validator);
            }
        }
    }

    return result;
}

HexagonServer::HexagonServer(HGAssets*                                   assets,
                             HexagonGame*                                hexagonGame,
                             const sf::IpAddress&                        serverIp,
                             const unsigned short                        serverPort,
                             const unsigned short                        serverControlPort,
                             const std::unordered_set<sf::base::String>& serverLevelWhitelist) :
    _assets{assets},
    _hexagonGame{hexagonGame},
    _supportedLevelValidators{makeSupportedLevelValidators(assets, serverLevelWhitelist)},
    _supportedLevelValidatorsVector{Utils::toVector(_supportedLevelValidators)},
    _serverIp{serverIp},
    _serverPort{serverPort},
    _serverControlPort{serverControlPort},
    _controlSocket{sf::UdpSocket::create(true /* isBlocking */).value()},
    _listener{},
    _socketSelector{},
    _running{true},
    _verbose{false},
    _serverPSKeys{generateSodiumPSKeys()},
    _lastTokenPurge{Utils::SCClock::now()}
{
    const auto sKeyPublic = sodiumKeyToString(_serverPSKeys.keyPublic);
    const auto sKeySecret = sodiumKeyToString(_serverPSKeys.keySecret);

    SSVOH_SLOG << "Initializing server...\n"
               << " - " << SSVOH_CLOG_VAR_IP(_serverIp) << '\n'
               << " - " << SSVOH_SLOG_VAR(_serverPort) << '\n'
               << " - " << SSVOH_SLOG_VAR(_serverControlPort) << '\n'
               << " - " << SSVOH_SLOG_VAR(sKeyPublic) << '\n'
               << " - " << SSVOH_SLOG_VAR(sKeySecret) << '\n';

    // ------------------------------------------------------------------------
    // Check initialization failures
#define SSVOH_SLOG_INIT_ERROR SSVOH_SLOG_ERROR << "Failure initializing server: "

    if (!initializeControlSocket())
    {
        SSVOH_SLOG_INIT_ERROR << "Control socket could not be initialized\n";
        return;
    }

    if (!initializeTcpListener())
    {
        SSVOH_SLOG_INIT_ERROR << "TCP listener could not be initialized\n";
        return;
    }

    if (!initializeSocketSelector())
    {
        SSVOH_SLOG_INIT_ERROR << "Socket selector could not be initialized\n";
        return;
    }

#undef SSVOH_SLOG_INIT_ERROR

    // ------------------------------------------------------------------------
    // Print supported (ranked) level validators
    {
        sf::OutStringStream oss;
        oss << "Server initialized!\nSupported levels:\n";

        for (const sf::base::String& levelValidator : _supportedLevelValidators)
        {
            oss << " - " << levelValidator << '\n';
        }

        SSVOH_SLOG << oss.getString() << '\n';
    }
}

void HexagonServer::stop()
{
    _running = false;
    _listener.reset();

    // Closing an FD does not unblock a `select()` already in progress on
    // another thread (the selector keeps sleeping until its timeout). Poke
    // the server's own control socket with a throwaway UDP byte so the
    // selector wakes on an actually-observable event; the next iteration
    // then sees `_running == false` and exits.
    if (const unsigned short controlPort = _controlSocket.getLocalPort(); controlPort != 0)
    {
        if (auto wake = sf::UdpSocket::create(true /* isBlocking */); wake.hasValue())
        {
            const char byte = '\0';
            (void)wake->send(&byte, 1, sf::IpAddress::LocalHost, controlPort);
        }
    }
}

unsigned short HexagonServer::getListenerPort() const
{
    return _listener.hasValue() ? _listener->getLocalPort() : static_cast<unsigned short>(0);
}

sf::base::Vector<SodiumPublicKeyArray> HexagonServer::getConnectedClientPublicKeys() const
{
    sf::base::Vector<SodiumPublicKeyArray> result;
    result.reserve(_connectedClients.size());

    for (const ConnectedClient& c : _connectedClients)
    {
        if (c._clientPublicKey.hasValue())
        {
            result.emplaceBack(*c._clientPublicKey);
        }
    }

    return result;
}

HexagonServer::~HexagonServer()
{
    SSVOH_SLOG << "Uninitializing server...\n";

    for (ConnectedClient& connectedClient : _connectedClients)
    {
        connectedClient._socket.setBlocking(true);

        (void)sendKick(connectedClient);

        connectedClient._socket.disconnect();
    }

    _socketSelector.clear();
    _listener.reset();
}

} // namespace hg
