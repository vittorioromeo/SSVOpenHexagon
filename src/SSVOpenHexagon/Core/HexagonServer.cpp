// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonServer.hpp"

#include "SSVOpenHexagon/Utils/Match.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/LevelValidator.hpp"
#include "SSVOpenHexagon/Utils/Split.hpp"
#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Online/Database.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include <SFML/Network.hpp>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <optional>
#include <cstdio>
#include <type_traits>
#include <stdexcept>

static auto& slog(const char* funcName)
{
    return ::ssvu::lo(::hg::Utils::concat("hg::HexagonServer::", funcName));
}

#define SSVOH_SLOG ::slog(__func__)

#define SSVOH_SLOG_VERBOSE \
    if(_verbose) ::slog(__func__)

#define SSVOH_SLOG_ERROR ::slog(__func__) << "[ERROR] "

#define SSVOH_SLOG_VAR(x) '\'' << #x << "': '" << x << '\''

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
    return sendEncrypted(c, //
        STCPLoginSuccess{
            .loginToken = static_cast<sf::Uint64>(loginToken), //
            .loginName = loginName                             //
        }                                                      //
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
    return sendEncrypted(c, //
        STCPTopScores{
            .levelValidator = levelValidator, //
            .scores = scores                  //
        }                                     //
    );
}

[[nodiscard]] bool HexagonServer::sendOwnScore(ConnectedClient& c,
    const std::string& levelValidator, const Database::ProcessedScore& score)
{
    return sendEncrypted(c, //
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
    return sendEncrypted(c, //
        STCPTopScoresAndOwnScore{
            .levelValidator = levelValidator, //
            .scores = scores,                 //
            .ownScore = ownScore              //
        }                                     //
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
    SSVOH_SLOG_VERBOSE << "Waiting for clients...\n";

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
}

bool HexagonServer::runIteration_Control()
{
    if(!_socketSelector.isReady(_controlSocket))
    {
        return fail();
    }

    sf::IpAddress senderIp;
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

    SSVOH_SLOG << "Received control packet from '" << senderIp << ':'
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
            _verbose = true;
            return true;
        }

        if(splitted[1] == "false")
        {
            _verbose = false;
            return true;
        }
    }

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

    return true;
}

bool HexagonServer::runIteration_TryAcceptingNewClient()
{
    if(!_socketSelector.isReady(_listener))
    {
        return false;
    }

    SSVOH_SLOG_VERBOSE << "Listener is ready\n";

    ConnectedClient& potentialClient =
        _connectedClients.emplace_back(Clock::now());

    sf::TcpSocket& potentialSocket = potentialClient._socket;
    potentialSocket.setBlocking(true);

    const void* potentialClientAddress = static_cast<void*>(&potentialClient);

    // The listener is ready: there is a pending connection
    if(_listener.accept(potentialSocket) != sf::Socket::Done)
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

    // Add the new client to the selector so that we will  be notified when he
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
        if(clientSocket.receive(_packetBuffer) == sf::Socket::Done)
        {
            SSVOH_SLOG_VERBOSE << "Successfully received data from client '"
                               << clientAddr << "'\n";

            if(processPacket(connectedClient, _packetBuffer))
            {
                connectedClient._lastActivity = Clock::now();
                connectedClient._consecutiveFailures = 0;

                continue;
            }
            else
            {
                ++connectedClient._consecutiveFailures;
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

    const TimePoint now = Clock::now();

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

void HexagonServer::runIteration_PurgeTokens()
{
    if(Clock::now() - _lastTokenPurge < std::chrono::seconds(240))
    {
        return;
    }

    SSVOH_SLOG << "Purging old login tokens\n";
    _lastTokenPurge = Clock::now();

    for(const Database::LoginToken& lt : Database::getAllStaleLoginTokens())
    {
        SSVOH_SLOG << "Found stale token for user '" << lt.userId << "'\n";

        for(auto it = _connectedClients.begin(); it != _connectedClients.end();
            ++it)
        {
            ConnectedClient& c = *it;
            const void* clientAddr = static_cast<void*>(&c);

            if(c._loginData.has_value())
            {
                if(c._loginData->_userId == lt.userId)
                {
                    SSVOH_SLOG << "Kicking stale token client '" << clientAddr
                               << "'\n";

                    kickAndRemoveClient(c);
                    it = _connectedClients.erase(it);
                }
            }
        }
    }

    Database::removeAllStaleLoginTokens();
}

[[nodiscard]] bool HexagonServer::processPacket(
    ConnectedClient& c, sf::Packet& p)
{
    const void* clientAddr = static_cast<void*>(&c);

    const auto validateLogin = [&](const char* context,
                                   const sf::Uint64 ctspLoginToken) -> bool
    {
        if(!c._loginData.has_value())
        {
            SSVOH_SLOG << "Client '" << clientAddr << "', is not logged in for "
                       << context << '\n';

            return false;
        }

        const auto cLoginToken = c._loginData->_loginToken;

        if(cLoginToken != ctspLoginToken)
        {
            SSVOH_SLOG << "Client '" << clientAddr
                       << "' login token mismatch for " << context << '\n';

            return false;
        }

        return true;
    };

    constexpr int topScoresLimit = 12;

    _errorOss.str("");
    const PVClientToServer pv = decodeClientToServerPacket(
        c._rtKeys.has_value() ? &c._rtKeys->keyReceive : nullptr, _errorOss, p);

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

        [&](const CTSPDisconnect&)
        {
            c._mustDisconnect = true;
            c._state = ConnectedClient::State::Disconnected;
            return true;
        },

        [&](const CTSPPublicKey& ctsp)
        {
            SSVOH_SLOG << "Received public key packet from client '"
                       << clientAddr << "'\n";

            if(c._clientPublicKey.has_value())
            {
                SSVOH_SLOG << "Already had public key, replacing\n";
            }
            else
            {
                SSVOH_SLOG << "Did not have public key, setting\n";
            }

            c._clientPublicKey = ctsp.key;

            SSVOH_SLOG << "Client public key: '" << sodiumKeyToString(ctsp.key)
                       << "'\n";

            SSVOH_SLOG << "Calculating RT keys\n";
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

            SSVOH_SLOG << "Calculated RT keys\n"
                       << " - " << SSVOH_SLOG_VAR(keyReceive) << '\n'
                       << " - " << SSVOH_SLOG_VAR(keyTransmit) << '\n';

            SSVOH_SLOG << "Replying with own public key\n";
            return sendPublicKey(c);
        },

        [&](const CTSPRegister& ctsp)
        {
            const auto& [steamId, name, passwordHash] = ctsp;

            SSVOH_SLOG << "Received register packet from client '" << clientAddr
                       << "'\nContents: '" << steamId << ", " << name << ", "
                       << passwordHash << "'\n";

            const auto sendFail = [&](const auto&... xs)
            {
                const std::string errorStr = Utils::concat(xs...);

                SSVOH_SLOG << errorStr << '\n';
                return sendRegistrationFailure(c, errorStr);
            };

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
                    .passwordHash = passwordHash //
                }                                //
            );

            SSVOH_SLOG << "Successfully registered\n";
            return sendRegistrationSuccess(c);
        },

        [&](const CTSPLogin& ctsp)
        {
            const auto& [steamId, name, passwordHash] = ctsp;

            SSVOH_SLOG << "Received login packet from client '" << clientAddr
                       << "'\nContents: '" << steamId << ", " << name << ", "
                       << passwordHash << "'\n";

            const auto sendFail = [&](const auto&... xs)
            {
                const std::string errorStr = Utils::concat(xs...);

                SSVOH_SLOG << errorStr << '\n';
                return sendLoginFailure(c, errorStr);
            };

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

            if(user->passwordHash != passwordHash)
            {
                return sendFail("Invalid password for user matching '", steamId,
                    "' and '", name, '\'');
            }

            SSVOH_SLOG << "Creating login token for user\n";

            const std::uint64_t loginToken = randomUInt64();

            Database::removeAllLoginTokensForUser(user->id);

            static_assert(std::is_same_v<Clock, Database::Clock>);

            Database::addLoginToken( //
                Database::LoginToken{
                    .userId = user->id,
                    .timestamp = Database::nowTimestamp(),
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
            SSVOH_SLOG << "Received logout packet from client '" << clientAddr
                       << "'\nContents: '" << ctsp.steamId << "'\n";

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
            const auto& [steamId, passwordHash] = ctsp;

            SSVOH_SLOG << "Received delete account packet from client '"
                       << clientAddr << "'\nContents: '" << steamId << ", "
                       << passwordHash << "'\n";

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

            if(user->passwordHash != passwordHash)
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
            if(!validateLogin("top scores", ctsp.loginToken))
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
            const TimePoint receiveTime = Clock::now();

            if(!validateLogin("replay", ctsp.loginToken))
            {
                return true;
            }

            const auto discard = [&](const auto&... reason)
            {
                SSVOH_SLOG << "Discarding replay from client '" << clientAddr
                           << "', " << Utils::concat(reason...) << '\n';

                return true;
            };

            if(!c._gameStatus.has_value())
            {
                return discard("no game started");
            }

            const hg::replay_file& rf = ctsp.replayFile;

            const HGAssets& assets = _hexagonGame.getAssets();

            if(!assets.isValidPackId(rf._pack_id))
            {
                return discard("invalid pack id '", rf._pack_id, '\'');
            }

            if(!assets.isValidLevelId(rf._level_id))
            {
                return discard("invalid level id '", rf._level_id, '\'');
            }

            const LevelData& levelData = assets.getLevelData(rf._level_id);

            if(levelData.unscored)
            {
                return discard("unscored level id '", rf._level_id, '\'');
            }

            const std::string levelValidator =
                Utils::getLevelValidator(rf._level_id, rf._difficulty_mult);

            SSVOH_SLOG << "Processing replay from client '" << clientAddr
                       << "' for level '" << levelValidator << "'\n";

            const double score =
                _hexagonGame.runReplayUntilDeathAndGetScore(rf);

            SSVOH_SLOG << "Replay processed, final time: '" << score << "'\n";

            const double elapsedSecs =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                    receiveTime - c._gameStatus->_startTP)
                    .count();

            const double difference = std::fabs(score - elapsedSecs);

            SSVOH_SLOG << "Elapsed request time: " << elapsedSecs << '\n'
                       << "Difference: " << difference << '\n';

            // TODO (P0): doesn't work with levels that have pauses
            if(difference > 3.5) // TODO (P1): something smarter here?
            {
                return discard("difference too large");
            }

            SSVOH_SLOG << "Replay valid, adding to database\n";

            Database::addScore(levelValidator, Database::nowTimestamp(),
                c._loginData->_steamId, score);

            return true;
        },

        [&](const CTSPRequestOwnScore& ctsp)
        {
            if(!validateLogin("own score", ctsp.loginToken))
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
            if(!validateLogin("top scores and own scores", ctsp.loginToken))
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
            if(!validateLogin("started game", ctsp.loginToken))
            {
                return true;
            }

            const std::string& lv = ctsp.levelValidator;

            SSVOH_SLOG << "Client '" << clientAddr
                       << "' started game for level '" << lv << "'\n";

            c._gameStatus = ConnectedClient::GameStatus{
                ._startTP = Clock::now(), //
                ._levelValidator = lv     //
            };

            return true;
        }

        //
    );
}

HexagonServer::HexagonServer(HGAssets& assets, HexagonGame& hexagonGame)
    : _assets{assets},
      _hexagonGame{hexagonGame},
      // TODO (P2): remove dependency on config
      _serverIp{Config::getServerIp()},
      _serverPort{Config::getServerPort()},
      _serverControlPort{Config::getServerControlPort()},
      _listener{},
      _socketSelector{},
      _running{true},
      _verbose{false},
      _serverPSKeys{generateSodiumPSKeys()},
      _lastTokenPurge{Clock::now()}
{
    const auto sKeyPublic = sodiumKeyToString(_serverPSKeys.keyPublic);
    const auto sKeySecret = sodiumKeyToString(_serverPSKeys.keySecret);

    SSVOH_SLOG << "Initializing server...\n"
               << " - " << SSVOH_SLOG_VAR(_serverIp) << '\n'
               << " - " << SSVOH_SLOG_VAR(_serverPort) << '\n'
               << " - " << SSVOH_SLOG_VAR(_serverControlPort) << '\n'
               << " - " << SSVOH_SLOG_VAR(sKeyPublic) << '\n'
               << " - " << SSVOH_SLOG_VAR(sKeySecret) << '\n';

#define SSVOH_SLOG_INIT_ERROR \
    SSVOH_SLOG_ERROR << "Failure initializing server: "

    if(_serverIp == sf::IpAddress::None)
    {
        SSVOH_SLOG_INIT_ERROR << "Invalid IP '" << _serverIp << "'\n";
        return;
    }

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
