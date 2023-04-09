// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonServer.hpp"

#include "SSVOpenHexagon/Data/LevelData.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"

#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"

#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/LevelValidator.hpp"
#include "SSVOpenHexagon/Utils/Match.hpp"
#include "SSVOpenHexagon/Utils/Split.hpp"
#include "SSVOpenHexagon/Utils/StringToCharVec.hpp"
#include "SSVOpenHexagon/Utils/Timestamp.hpp"
#include "SSVOpenHexagon/Utils/VectorToSet.hpp"

#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Online/Database.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include <SFML/Network.hpp>

#include <boost/pfr.hpp>

#include <chrono>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <stdexcept>

#include <csignal>
#include <cstdlib>
#include <cstdint>
#include <cstdio>

static auto& slog(const char* funcName)
{
    return ::ssvu::lo(::hg::Utils::concat("hg::HexagonServer::", funcName));
}

#define SSVOH_SLOG ::slog(__func__)

#define SSVOH_SLOG_VERBOSE \
    if(_verbose) ::slog(__func__)

#define SSVOH_SLOG_ERROR ::slog(__func__) << "[ERROR] "

#define SSVOH_SLOG_VAR(x) '\'' << #x << "': '" << (x) << '\''

namespace hg {

template <typename... Ts>
[[nodiscard]] bool HexagonServer::fail(const Ts&... xs)
{
    if constexpr(sizeof...(Ts) > 0)
    {
        auto& stream = SSVOH_SLOG_ERROR;
        (stream << ... << xs);
        stream << '\n';
    }

    return false;
}

[[nodiscard]] bool HexagonServer::isLevelSupported(
    const std::string& levelValidator) const
{
    return _supportedLevelValidators.contains(levelValidator);
}

[[nodiscard]] bool HexagonServer::initializeControlSocket()
{
    SSVOH_SLOG << "Initializing UDP control socket...\n";

    _controlSocket.setBlocking(true);

    if(_controlSocket.bind(_serverControlPort, sf::IpAddress::LocalHost) !=
        sf::Socket::Status::Done)
    {
        return fail("Failure binding UDP control socket");
    }

    _socketSelector.add(_controlSocket);
    return true;
}

[[nodiscard]] bool HexagonServer::initializeTcpListener()
{
    SSVOH_SLOG << "Initializing TCP listener...\n";

    _listener.setBlocking(true);

    if(_listener.listen(_serverPort) == sf::TcpListener::Status::Error)
    {
        return fail("Failure initializing TCP listener");
    }

    return true;
}

[[nodiscard]] bool HexagonServer::initializeSocketSelector()
{
    SSVOH_SLOG << "Initializing socket selector...\n";

    _socketSelector.add(_listener);
    return true;
}

[[nodiscard]] bool HexagonServer::sendPacket(ConnectedClient& c, sf::Packet& p)
{
    if(c._socket.send(p) != sf::Socket::Status::Done)
    {
        return fail("Failure sending packet");
    }

    return true;
}

template <typename T>
[[nodiscard]] bool HexagonServer::sendEncrypted(
    ConnectedClient& c, const T& data)
{
    const void* clientAddr = static_cast<void*>(&c);

    if(!c._rtKeys.has_value())
    {
        return fail(
            "Tried to send encrypted message without RT keys for client '",
            clientAddr, '\'');
    }

    if(!makeServerToClientEncryptedPacket(
           c._rtKeys->keyTransmit, _packetBuffer, data))
    {
        return fail("Error building encrypted message packet for client '",
            clientAddr, '\'');
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
    makeServerToClientPacket(
        _packetBuffer, STCPPublicKey{.key = _serverPSKeys.keyPublic});

    return sendPacket(c, _packetBuffer);
}

[[nodiscard]] bool HexagonServer::sendRegistrationSuccess(ConnectedClient& c)
{
    return sendEncrypted(c, STCPRegistrationSuccess{});
}

[[nodiscard]] bool HexagonServer::sendRegistrationFailure(
    ConnectedClient& c, const std::string& error)
{
    return sendEncrypted(c, STCPRegistrationFailure{.error = error});
}

[[nodiscard]] bool HexagonServer::sendLoginSuccess(ConnectedClient& c,
    const std::uint64_t loginToken, const std::string& loginName)
{
    return sendEncrypted(c,                                       //
        STCPLoginSuccess{
            .loginToken = static_cast<std::uint64_t>(loginToken), //
            .loginName = loginName                                //
        }                                                         //
    );
}

[[nodiscard]] bool HexagonServer::sendLoginFailure(
    ConnectedClient& c, const std::string& error)
{
    return sendEncrypted(c, STCPLoginFailure{.error = error});
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

[[nodiscard]] bool HexagonServer::sendDeleteAccountFailure(
    ConnectedClient& c, const std::string& error)
{
    return sendEncrypted(c, STCPDeleteAccountFailure{.error = error});
}

[[nodiscard]] bool HexagonServer::sendTopScores(ConnectedClient& c,
    const std::string& levelValidator,
    const std::vector<Database::ProcessedScore>& scores)
{
    return sendEncrypted(c,                   //
        STCPTopScores{
            .levelValidator = levelValidator, //
            .scores = scores                  //
        }                                     //
    );
}

[[nodiscard]] bool HexagonServer::sendOwnScore(ConnectedClient& c,
    const std::string& levelValidator, const Database::ProcessedScore& score)
{
    return sendEncrypted(c,                   //
        STCPOwnScore{
            .levelValidator = levelValidator, //
            .score = score                    //
        }                                     //
    );
}

[[nodiscard]] bool HexagonServer::sendTopScoresAndOwnScore(ConnectedClient& c,
    const std::string& levelValidator,
    const std::vector<Database::ProcessedScore>& scores,
    const std::optional<Database::ProcessedScore>& ownScore)
{
    return sendEncrypted(c,                   //
        STCPTopScoresAndOwnScore{
            .levelValidator = levelValidator, //
            .scores = scores,                 //
            .ownScore = ownScore              //
        }                                     //
    );
}

[[nodiscard]] bool HexagonServer::sendServerStatus(ConnectedClient& c,
    const ProtocolVersion& protocolVersion, const GameVersion& gameVersion,
    const std::vector<std::string>& supportedLevelValidators)
{
    return sendEncrypted(c,                                      //
        STCPServerStatus{
            .protocolVersion = protocolVersion,                  //
            .gameVersion = gameVersion,                          //
            .supportedLevelValidators = supportedLevelValidators //
        }                                                        //
    );
}

void HexagonServer::kickAndRemoveClient(ConnectedClient& c)
{
    (void)sendKick(c);

    if(c._loginData.has_value())
    {
        Database::removeAllLoginTokensForUser(c._loginData->_userId);
    }

    _socketSelector.remove(c._socket);
}

void HexagonServer::run()
{
    while(_running)
    {
        try
        {
            runIteration();
        }
        catch(const std::runtime_error& e)
        {
            SSVOH_SLOG_ERROR << "Exception: '" << e.what() << "'\n";
        }
        catch(...)
        {
            SSVOH_SLOG_ERROR << "Unknown exception";
        }
    }
}

void HexagonServer::runIteration()
{
    SSVOH_SLOG_VERBOSE << "New iteration...\n";

    if(_socketSelector.wait(sf::seconds(30)))
    {
        // A timeout is specified so that we can purge clients even if we didn't
        // receive anything.

        runIteration_Control();
        runIteration_TryAcceptingNewClient();
        runIteration_LoopOverSockets();
    }

    runIteration_PurgeClients();
    runIteration_PurgeTokens();
    runIteration_FlushLogs();
}

bool HexagonServer::runIteration_Control()
{
    if(!_socketSelector.isReady(_controlSocket))
    {
        return fail();
    }

    std::optional<sf::IpAddress> senderIp;
    unsigned short senderPort;

    if(_controlSocket.receive(_packetBuffer, senderIp, senderPort) !=
        sf::Socket::Status::Done)
    {
        return fail("Failure receiving control packet");
    }

    std::string controlMsg;

    if(!(_packetBuffer >> controlMsg))
    {
        return fail("Failure decoding control packet");
    }

    SSVOH_ASSERT(senderIp.has_value());

    SSVOH_SLOG << "Received control packet from '" << senderIp.value() << ':'
               << senderPort << "', contents: '" << controlMsg << "'\n";

    if(controlMsg.empty())
    {
        return true;
    }

    const auto splitted = Utils::split<std::string>(controlMsg);

    if(splitted.empty())
    {
        return true;
    }

    if(splitted[0] == "verbose")
    {
        if(splitted.size() != 2)
        {
            SSVOH_SLOG_ERROR
                << "'verbose' command must be followed by 'true' or 'false'\n";

            return true;
        }

        if(splitted[1] == "true")
        {
            SSVOH_SLOG << "Enabled verbose mode\n";

            _verbose = true;
            return true;
        }

        if(splitted[1] == "false")
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

            std::string query = splitted[2];
            for(std::size_t i = 3; i < splitted.size(); ++i)
            {
                query += ' ';
                query += splitted[i];
            }

            const std::optional<std::string> executeOutcome =
                Database::execute(query);

            if(executeOutcome.has_value())
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
    if(!_socketSelector.isReady(_listener))
    {
        return false;
    }

    SSVOH_SLOG << "Listener is ready, attempting to accept new client\n";

    ConnectedClient& potentialClient =
        _connectedClients.emplace_back(Utils::SCClock::now());

    sf::TcpSocket& potentialSocket = potentialClient._socket;
    potentialSocket.setBlocking(true);

    const void* potentialClientAddress = static_cast<void*>(&potentialClient);

    // TODO (P1): potential hanging spot?
    // The listener is ready: there is a pending connection
    if(_listener.accept(potentialSocket) != sf::Socket::Status::Done)
    {
        SSVOH_SLOG << "Listener failed to accept new client '"
                   << potentialClientAddress << "'\n";

        // Error, we won't get a new connection, delete the socket
        _connectedClients.pop_back();
        return false;
    }

    SSVOH_SLOG << "Listener accepted new client '" << potentialClientAddress
               << "'\n";

    potentialClient._state = ConnectedClient::State::Connected;

    // Add the new client to the selector so that we will be notified when he
    // sends something
    _socketSelector.add(potentialSocket);
    return true;
}

void HexagonServer::runIteration_LoopOverSockets()
{
    for(auto it = _connectedClients.begin(); it != _connectedClients.end();
        ++it)
    {
        ConnectedClient& connectedClient = *it;
        const void* clientAddr = static_cast<void*>(&connectedClient);
        sf::TcpSocket& clientSocket = connectedClient._socket;

        if(!_socketSelector.isReady(clientSocket))
        {
            continue;
        }

        SSVOH_SLOG_VERBOSE << "Client '" << clientAddr << "' has sent data\n ";

        // The client has sent some data, we can receive it
        _packetBuffer.clear();

        // TODO (P1): potential hanging spot?
        if(clientSocket.receive(_packetBuffer) == sf::Socket::Status::Done)
        {
            SSVOH_SLOG_VERBOSE << "Successfully received data from client '"
                               << clientAddr << "'\n";

            if(processPacket(connectedClient, _packetBuffer))
            {
                connectedClient._lastActivity = Utils::SCClock::now();
                connectedClient._consecutiveFailures = 0;

                continue;
            }
        }

        // Failed to receive data
        SSVOH_SLOG_VERBOSE << "Failed to receive data from client '"
                           << clientAddr << "' (consecutive failures: "
                           << connectedClient._consecutiveFailures << ")\n";

        ++connectedClient._consecutiveFailures;

        constexpr int maxConsecutiveFailures = 5;
        if(connectedClient._consecutiveFailures == maxConsecutiveFailures)
        {
            SSVOH_SLOG << "Too many consecutive failures for client '"
                       << clientAddr << "', removing from list\n";

            kickAndRemoveClient(connectedClient);
            it = _connectedClients.erase(it);
        }
    }
}

void HexagonServer::runIteration_PurgeClients()
{
    constexpr std::chrono::duration maxInactivity = std::chrono::seconds(60);

    const Utils::SCTimePoint now = Utils::SCClock::now();

    for(auto it = _connectedClients.begin(); it != _connectedClients.end();
        ++it)
    {
        ConnectedClient& connectedClient = *it;
        const void* clientAddr = static_cast<void*>(&connectedClient);

        if(connectedClient._mustDisconnect)
        {
            SSVOH_SLOG << "Client '" << clientAddr
                       << "' disconnected, removing from list\n";

            kickAndRemoveClient(connectedClient);
            it = _connectedClients.erase(it);
            continue;
        }

        if(now - connectedClient._lastActivity > maxInactivity)
        {
            SSVOH_SLOG << "Client '" << clientAddr
                       << "' timed out, removing from list\n";

            kickAndRemoveClient(connectedClient);
            it = _connectedClients.erase(it);
            continue;
        }
    }
}

template <typename TDuration>
[[nodiscard]] static bool checkAndUpdateLastElapsed(
    Utils::SCTimePoint& last, const TDuration duration)
{
    if(Utils::SCClock::now() - last < duration)
    {
        return false;
    }

    last = Utils::SCClock::now();
    return true;
}

void HexagonServer::runIteration_PurgeTokens()
{
    if(!checkAndUpdateLastElapsed(
           _lastTokenPurge, std::chrono::seconds(3600) /* 1 hour */))
    {
        return;
    }

    SSVOH_SLOG_VERBOSE << "Purging old login tokens\n";

    for(const Database::LoginToken& lt : Database::getAllStaleLoginTokens())
    {
        SSVOH_SLOG << "Found stale token for user '" << lt.userId << "'\n";

        for(auto it = _connectedClients.begin(); it != _connectedClients.end();
            ++it)
        {
            ConnectedClient& c = *it;
            const void* clientAddr = static_cast<void*>(&c);

            if(!c._loginData.has_value())
            {
                continue;
            }

            if(c._loginData->_userId == lt.userId)
            {
                SSVOH_SLOG << "Kicking stale token client '" << clientAddr
                           << "'\n";

                kickAndRemoveClient(c);
                it = _connectedClients.erase(it);
            }
        }
    }

    Database::removeAllStaleLoginTokens();
}

void HexagonServer::runIteration_FlushLogs()
{
    if(!checkAndUpdateLastElapsed(_lastLogsFlush, std::chrono::seconds(1)))
    {
        return;
    }

    std::cout.flush();
    std::cerr.flush();
    ssvu::lo().flush();
}

[[nodiscard]] bool HexagonServer::validateLogin(
    ConnectedClient& c, const char* context, const std::uint64_t ctspLoginToken)
{
    const void* clientAddr = static_cast<void*>(&c);

    if(!c._loginData.has_value())
    {
        SSVOH_SLOG << "Client '" << clientAddr << "', is not logged in for "
                   << context << '\n';

        return false;
    }

    const auto cLoginToken = c._loginData->_loginToken;

    if(cLoginToken != ctspLoginToken)
    {
        SSVOH_SLOG << "Client '" << clientAddr << "' login token mismatch for "
                   << context << '\n';

        return false;
    }

    return true;
}

[[nodiscard]] bool HexagonServer::processReplay(
    ConnectedClient& c, const std::uint64_t loginToken, const replay_file& rf)
{
    const void* clientAddr = static_cast<void*>(&c);

    const Utils::SCTimePoint receiveTime = Utils::SCClock::now();

    if(!validateLogin(c, "replay", loginToken))
    {
        return true;
    }

    const auto discard = [&](const auto&... reason)
    {
        SSVOH_SLOG << "Discarding replay from client '" << clientAddr << "', "
                   << Utils::concat(reason...) << ", replay time was "
                   << rf.played_seconds() << "s\n";

        return true;
    };

    if(!c._gameStatus.has_value())
    {
        return discard("no game started");
    }

    if(!_assets.isValidPackId(rf._pack_id))
    {
        return discard("invalid pack id '", rf._pack_id, '\'');
    }

    if(!_assets.isValidLevelId(rf._level_id))
    {
        return discard("invalid level id '", rf._level_id, '\'');
    }

    const LevelData& levelData = _assets.getLevelData(rf._level_id);

    if(levelData.unscored)
    {
        return discard("unscored level id '", rf._level_id, '\'');
    }

    const std::string levelValidator =
        Utils::getLevelValidator(rf._level_id, rf._difficulty_mult);

    SSVOH_SLOG << "Processing replay from client '" << clientAddr
               << "' for level '" << levelValidator << "'\n";

    constexpr int maxProcessingSeconds = 5;

    const std::optional<HexagonGame::GameExecutionResult> ger =
        _hexagonGame.runReplayUntilDeathAndGetScore(
            rf, maxProcessingSeconds, 1.f /* timescale */);

    if(!ger.has_value())
    {
        return discard(
            "max processing time exceeded (", maxProcessingSeconds, "s)");
    }

    const double replayTotalTime = ger->totalTimeSeconds;
    const double replayPlayedTime = ger->playedTimeSeconds;

    SSVOH_SLOG << "Replay processed, final time: '" << replayTotalTime << "'\n";

    const double elapsedSecs =
        std::chrono::duration_cast<std::chrono::duration<double>>(
            receiveTime - c._gameStatus->_startTP)
            .count();

    const double difference = std::fabs(replayTotalTime - elapsedSecs);
    const double ratio = replayTotalTime / elapsedSecs;

    const bool goodDifference = difference < 5.0;
    const bool goodRatio = ratio > 0.65 && ratio < 1.35;

    const auto printDifferenceAndRatio = [&]
    {
        SSVOH_SLOG << "Elapsed request time: " << elapsedSecs << '\n'
                   << "Difference: " << difference << '\n'
                   << "Ratio: " << ratio << '\n';
    };

    if(!goodDifference)
    {
        printDifferenceAndRatio();
        return discard("difference too large");
    }

    if(!goodRatio)
    {
        printDifferenceAndRatio();
        return discard("bad ratio");
    }

    SSVOH_SLOG << "Replay valid, adding to database\n";

    SSVOH_ASSERT(c._loginData.has_value());

    Database::addScore(levelValidator, Utils::nowTimestamp(),
        c._loginData->_steamId, replayPlayedTime);

    return true;
}

template <typename T>
void HexagonServer::printCTSPDataVerbose(
    ConnectedClient& c, const char* title, const T& ctsp)
{
    if(!_verbose)
    {
        return;
    }

    const auto stringify = []<typename U>(const U& field) -> decltype(auto)
    {
        if constexpr(std::is_same_v<U, SodiumPublicKeyArray>)
        {
            return sodiumKeyToString(field);
        }
        else if constexpr(std::is_same_v<U, replay_file>)
        {
            return "<REPLAY_FILE>";
        }
        else if constexpr(std::is_same_v<U, compressed_replay_file>)
        {
            return "<COMPRESSED_REPLAY_FILE>";
        }
        else if constexpr(std::is_same_v<U, std::string>)
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

    stream << "Received '" << title << "' packet from client '" << clientAddr
           << "', contents: {";

    constexpr std::size_t nFields = boost::pfr::tuple_size_v<T>;
    if constexpr(nFields > 0)
    {
        std::size_t i = 0;
        boost::pfr::for_each_field(ctsp,
            [&](const auto& field)
            {
                stream << stringify(field);

                if(i != nFields - 1)
                {
                    stream << ", ";
                }

                ++i;
            });
    }

    stream << "}\n";
}

[[nodiscard]] bool HexagonServer::processPacket(
    ConnectedClient& c, sf::Packet& p)
{
    const void* clientAddr = static_cast<void*>(&c);

    constexpr int topScoresLimit = 6;

    _errorOss.str("");
    const PVClientToServer pv = decodeClientToServerPacket(
        c._rtKeys.has_value() ? &c._rtKeys->keyReceive : nullptr, _errorOss, p);

    const auto checkState = [&](const ConnectedClient::State state)
    {
        if(c._state != state)
        {
            SSVOH_SLOG_VERBOSE << "Invalid client state, expected '"
                               << static_cast<int>(state) << "', state was '"
                               << static_cast<int>(c._state) << "''\n";

            return false;
        }

        return true;
    };

    const auto checkState2 = [&](const ConnectedClient::State state0,
                                 const ConnectedClient::State state1)
    {
        if(c._state != state0 && c._state != state1)
        {
            SSVOH_SLOG_VERBOSE << "Invalid client state, expected '"
                               << static_cast<int>(state0) << "' or '"
                               << static_cast<int>(state1) << "', state was '"
                               << static_cast<int>(c._state) << "''\n";

            return false;
        }

        return true;
    };

    return Utils::match(
        pv,

        [&](const PInvalid&)
        {
            return fail("Error processing packet from client '", clientAddr,
                "', details: ", _errorOss.str());
        },

        [&](const PEncryptedMsg&)
        {
            return fail(
                "Received non-decrypted encrypted msg packet from client '",
                clientAddr, '\'');
        },

        [&](const CTSPHeartbeat&) { return true; },

        [&](const CTSPDisconnect& ctsp)
        {
            printCTSPDataVerbose(c, "disconnect", ctsp);

            c._mustDisconnect = true;
            c._state = ConnectedClient::State::Disconnected;
            return true;
        },

        [&](const CTSPPublicKey& ctsp)
        {
            printCTSPDataVerbose(c, "public key", ctsp);

            if(c._clientPublicKey.has_value())
            {
                SSVOH_SLOG_VERBOSE << "Already had public key, replacing\n";
            }
            else
            {
                SSVOH_SLOG_VERBOSE << "Did not have public key, setting\n";
            }

            c._clientPublicKey = ctsp.key;

            SSVOH_SLOG_VERBOSE << "Client public key: '"
                               << sodiumKeyToString(ctsp.key) << "'\n";

            SSVOH_SLOG_VERBOSE << "Calculating RT keys\n";
            c._rtKeys =
                calculateServerSessionSodiumRTKeys(_serverPSKeys, ctsp.key);

            if(!c._rtKeys.has_value())
            {
                SSVOH_SLOG_ERROR
                    << "Failed calculating RT keys, disconnecting client '"
                    << clientAddr << "'\n";

                c._mustDisconnect = true;
                (void)sendKick(c);

                return false;
            }

            const auto keyReceive = sodiumKeyToString(c._rtKeys->keyReceive);
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
                const std::string errorStr = Utils::concat(xs...);

                SSVOH_SLOG << errorStr << '\n';
                return sendRegistrationFailure(c, errorStr);
            };

            if(!checkState(ConnectedClient::State::Connected))
            {
                return sendFail("Client not in connected state");
            }

            if(name.size() > 32)
            {
                return sendFail("Name too long, max is 32 characters");
            }

            if(Database::anyUserWithSteamId(steamId))
            {
                return sendFail(
                    "User with steamId '", steamId, "' already registered");
            }

            if(Database::anyUserWithName(name))
            {
                return sendFail(
                    "User with name '", name, "' already registered");
            }

            Database::addUser( //
                Database::User{
                    .steamId = steamId,
                    .name = name,
                    .passwordHash = Utils::stringToCharVec(passwordHash) //
                }                                                        //
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
                const std::string errorStr = Utils::concat(xs...);

                SSVOH_SLOG << errorStr << '\n';
                return sendLoginFailure(c, errorStr);
            };

            if(!checkState(ConnectedClient::State::Connected))
            {
                return sendFail("Client not in connected state");
            }

            if(name.size() > 32)
            {
                return sendFail("Name too long, max is 32 characters");
            }

            if(!Database::anyUserWithSteamId(steamId))
            {
                return sendFail(
                    "No user with steamId '", steamId, "' registered");
            }

            if(!Database::anyUserWithName(name))
            {
                return sendFail("No user with name '", name, "' registered");
            }

            const std::optional<Database::User> user =
                Database::getUserWithSteamIdAndName(steamId, name);

            if(!user.has_value())
            {
                return sendFail("No user matching '", steamId, "' and '", name,
                    "' registered");
            }

            SSVOH_ASSERT(user.has_value());

            if(user->passwordHash != Utils::stringToCharVec(passwordHash))
            {
                return sendFail("Invalid password for user matching '", steamId,
                    "' and '", name, '\'');
            }

            SSVOH_SLOG << "Creating login token for user\n";

            const std::uint64_t loginToken = randomUInt64();

            Database::removeAllLoginTokensForUser(user->id);

            Database::addLoginToken( //
                Database::LoginToken{
                    .userId = user->id,
                    .timestamp = Utils::nowTimestamp(),
                    .token = loginToken //
                });

            c._loginData = ConnectedClient::LoginData{
                ._userId = user->id,
                ._steamId = steamId,
                ._name = name,
                ._passwordHash = passwordHash,
                ._loginToken = loginToken //
            };

            c._state = ConnectedClient::State::LoggedIn;

            SSVOH_SLOG << "Successfully logged in\n";
            return sendLoginSuccess(c, loginToken, user->name);
        },

        [&](const CTSPLogout& ctsp)
        {
            printCTSPDataVerbose(c, "logout", ctsp);

            if(!checkState2(ConnectedClient::State::LoggedIn,
                   ConnectedClient::State::LoggedIn_Ready))
            {
                return true;
            }

            const std::optional<Database::User> user =
                Database::getUserWithSteamId(ctsp.steamId);

            if(!user.has_value())
            {
                SSVOH_SLOG << "No user with steamId '" << ctsp.steamId << "'\n";
                return sendLogoutFailure(c);
            }

            SSVOH_ASSERT(user.has_value());

            Database::removeAllLoginTokensForUser(user->id);

            c._loginData.reset();
            c._state = ConnectedClient::State::Connected;

            return sendLogoutSuccess(c);
        },

        [&](const CTSPDeleteAccount& ctsp)
        {
            printCTSPDataVerbose(c, "delete account", ctsp);

            const auto& [steamId, passwordHash] = ctsp;

            if(!checkState(ConnectedClient::State::Connected))
            {
                return true;
            }

            const auto sendFail = [&](const auto&... xs)
            {
                const std::string errorStr = Utils::concat(xs...);

                SSVOH_SLOG << errorStr << '\n';
                return sendDeleteAccountFailure(c, errorStr);
            };

            if(!Database::anyUserWithSteamId(steamId))
            {
                return sendFail(
                    "No user with steamId '", steamId, "' registered");
            }

            const std::optional<Database::User> user =
                Database::getUserWithSteamId(ctsp.steamId);

            if(!user.has_value())
            {
                return sendFail("No user with steamId '", ctsp.steamId, '\'');
            }

            SSVOH_ASSERT(user.has_value());

            if(user->passwordHash != Utils::stringToCharVec(passwordHash))
            {
                return sendFail(
                    "Invalid password for user matching '", steamId, '\'');
            }

            Database::removeAllLoginTokensForUser(user->id);
            Database::removeUser(user->id);

            SSVOH_SLOG << "Successfully deleted account\n";
            return sendDeleteAccountSuccess(c);
        },

        [&](const CTSPRequestTopScores& ctsp)
        {
            printCTSPDataVerbose(c, "request top scores", ctsp);

            if(!checkState(ConnectedClient::State::LoggedIn_Ready) ||
                !validateLogin(c, "top scores", ctsp.loginToken))
            {
                return true;
            }

            if(!isLevelSupported(ctsp.levelValidator))
            {
                return true;
            }

            const std::string& lv = ctsp.levelValidator;

            SSVOH_SLOG_VERBOSE << "Sending top " << topScoresLimit
                               << " scores to client '" << clientAddr << "'\n";

            return sendTopScores(
                c, lv, Database::getTopScores(topScoresLimit, lv));
        },

        [&](const CTSPReplay& ctsp)
        {
            printCTSPDataVerbose(c, "replay", ctsp);

            if(!checkState(ConnectedClient::State::LoggedIn_Ready))
            {
                return true;
            }

            const auto& [loginToken, rf] = ctsp;
            return processReplay(c, loginToken, rf);
        },

        [&](const CTSPRequestOwnScore& ctsp)
        {
            printCTSPDataVerbose(c, "request own score", ctsp);

            if(!checkState(ConnectedClient::State::LoggedIn_Ready) ||
                !validateLogin(c, "own score", ctsp.loginToken))
            {
                return true;
            }

            if(!isLevelSupported(ctsp.levelValidator))
            {
                return true;
            }

            const std::optional<Database::ProcessedScore> ps =
                Database::getScore(ctsp.levelValidator, c._loginData->_steamId);

            if(!ps.has_value())
            {
                return true;
            }

            SSVOH_SLOG_VERBOSE << "Sending own score to client '" << clientAddr
                               << "'\n";

            return sendOwnScore(c, ctsp.levelValidator, *ps);
        },

        [&](const CTSPRequestTopScoresAndOwnScore& ctsp)
        {
            printCTSPDataVerbose(c, "request top scores and own score", ctsp);

            if(!checkState(ConnectedClient::State::LoggedIn_Ready) ||
                !validateLogin(c, "top scores and own scores", ctsp.loginToken))
            {
                return true;
            }

            if(!isLevelSupported(ctsp.levelValidator))
            {
                return true;
            }

            const std::string& lv = ctsp.levelValidator;

            SSVOH_SLOG_VERBOSE << "Sending top " << topScoresLimit
                               << " scores and own score to client '"
                               << clientAddr << "'\n";

            return sendTopScoresAndOwnScore(c, lv,
                Database::getTopScores(topScoresLimit, lv),
                Database::getScore(lv, c._loginData->_steamId));
        },

        [&](const CTSPStartedGame& ctsp)
        {
            printCTSPDataVerbose(c, "started game", ctsp);

            if(!checkState(ConnectedClient::State::LoggedIn_Ready) ||
                !validateLogin(c, "started game", ctsp.loginToken))
            {
                return true;
            }

            const std::string& lv = ctsp.levelValidator;

            SSVOH_SLOG << "Client '" << clientAddr
                       << "' started game for level '" << lv << "'\n";

            c._gameStatus = ConnectedClient::GameStatus{
                ._startTP = Utils::SCClock::now(), //
                ._levelValidator = lv              //
            };

            return true;
        },

        [&](const CTSPCompressedReplay& ctsp)
        {
            printCTSPDataVerbose(c, "compressed replay", ctsp);

            if(!checkState(ConnectedClient::State::LoggedIn_Ready))
            {
                return true;
            }

            const auto& [loginToken, crf] = ctsp;

            const std::optional<replay_file> rfOpt =
                decompress_replay_file(crf);

            if(!rfOpt.has_value())
            {
                SSVOH_SLOG_ERROR
                    << "Failed to decompress replay received from client '"
                    << clientAddr << "'\n";

                return false;
            }

            return processReplay(c, loginToken, rfOpt.value());
        },

        [&](const CTSPRequestServerStatus& ctsp)
        {
            printCTSPDataVerbose(c, "request server status", ctsp);

            if(!checkState(ConnectedClient::State::LoggedIn) ||
                !validateLogin(c, "request server status", ctsp.loginToken))
            {
                return true;
            }

            return sendServerStatus(c, PROTOCOL_VERSION, GAME_VERSION,
                _supportedLevelValidatorsVector);
        },

        [&](const CTSPReady& ctsp)
        {
            printCTSPDataVerbose(c, "ready", ctsp);

            if(!checkState(ConnectedClient::State::LoggedIn) ||
                !validateLogin(c, "ready", ctsp.loginToken))
            {
                return true;
            }

            c._state = ConnectedClient::State::LoggedIn_Ready;
            return true;
        }

        //
    );
}

[[nodiscard]] static std::unordered_set<std::string>
makeSupportedLevelValidators(HGAssets& assets,
    const std::unordered_set<std::string>& levelValidatorWhitelist)
{
    std::unordered_set<std::string> result;

    for(const auto& [assetId, ld] : assets.getLevelDatas())
    {
        if(ld.unscored)
        {
            continue;
        }

        for(const float dm : ld.difficultyMults)
        {
            if(const std::string& validator = ld.getValidator(dm);
                levelValidatorWhitelist.contains(validator))
            {
                result.emplace(validator);
            }
        }
    }

    return result;
}

HexagonServer::HexagonServer(HGAssets& assets, HexagonGame& hexagonGame,
    const sf::IpAddress& serverIp, const unsigned short serverPort,
    const unsigned short serverControlPort,
    const std::unordered_set<std::string>& serverLevelWhitelist)
    : _assets{assets},
      _hexagonGame{hexagonGame},
      _supportedLevelValidators{
          makeSupportedLevelValidators(assets, serverLevelWhitelist)},
      _supportedLevelValidatorsVector{
          Utils::toVector(_supportedLevelValidators)},
      _serverIp{serverIp},
      _serverPort{serverPort},
      _serverControlPort{serverControlPort},
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
               << " - " << SSVOH_SLOG_VAR(_serverIp) << '\n'
               << " - " << SSVOH_SLOG_VAR(_serverPort) << '\n'
               << " - " << SSVOH_SLOG_VAR(_serverControlPort) << '\n'
               << " - " << SSVOH_SLOG_VAR(sKeyPublic) << '\n'
               << " - " << SSVOH_SLOG_VAR(sKeySecret) << '\n';

    // ------------------------------------------------------------------------
    // Check initialization failures
#define SSVOH_SLOG_INIT_ERROR \
    SSVOH_SLOG_ERROR << "Failure initializing server: "

    if(!initializeControlSocket())
    {
        SSVOH_SLOG_INIT_ERROR << "Control socket could not be initialized\n";
        return;
    }

    if(!initializeTcpListener())
    {
        SSVOH_SLOG_INIT_ERROR << "TCP listener could not be initialized\n";
        return;
    }

    if(!initializeSocketSelector())
    {
        SSVOH_SLOG_INIT_ERROR << "Socket selector could not be initialized\n";
        return;
    }

#undef SSVOH_SLOG_INIT_ERROR

    // ------------------------------------------------------------------------
    // Signal handling: exit gracefully on CTRL-C
    {
        static bool& globalRunning = _running;
        static sf::TcpListener& globalListener = _listener;

        // TODO (P2): UB
        std::signal(SIGINT,
            [](int s)
            {
                std::printf("Caught signal %d\n", s);
                globalListener.close();
                globalRunning = false;
            });
    }

    // ------------------------------------------------------------------------
    // Print supported (ranked) level validators
    {
        std::ostringstream oss;
        oss << "Server initialized!\nSupported levels:\n";

        for(const std::string& levelValidator : _supportedLevelValidators)
        {
            oss << " - " << levelValidator << '\n';
        }

        SSVOH_SLOG << oss.str() << '\n';
    }

    run();
}

HexagonServer::~HexagonServer()
{
    SSVOH_SLOG << "Uninitializing server...\n";

    for(ConnectedClient& connectedClient : _connectedClients)
    {
        connectedClient._socket.setBlocking(true);

        (void)sendKick(connectedClient);
        connectedClient._socket.disconnect();
    }

    _socketSelector.clear();
    _listener.close();
    _controlSocket.unbind();
}

} // namespace hg
