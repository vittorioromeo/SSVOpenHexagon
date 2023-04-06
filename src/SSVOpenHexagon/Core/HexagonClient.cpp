// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonClient.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Utils/Match.hpp"
#include "SSVOpenHexagon/Utils/ScopeGuard.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include <SFML/Network/Packet.hpp>

#include <thread>
#include <chrono>

#include <cstdint>

static auto& clog(const char* funcName)
{
    return ::ssvu::lo(::hg::Utils::concat("hg::HexagonClient::", funcName));
}

#define SSVOH_CLOG ::clog(__func__)

#define SSVOH_CLOG_VERBOSE \
    if(_verbose) ::clog(__func__)

#define SSVOH_CLOG_ERROR ::clog(__func__) << "[ERROR] "

#define SSVOH_CLOG_VAR(x) '\'' << #x << "': '" << x << '\''

namespace hg {

template <typename... Ts>
[[nodiscard]] bool HexagonClient::fail(const Ts&... xs)
{
    if constexpr(sizeof...(Ts) > 0)
    {
        auto& stream = SSVOH_CLOG_ERROR;
        (stream << ... << xs);
        stream << '\n';
    }

    return false;
}

[[nodiscard]] bool HexagonClient::initializeTicketSteamID()
{
    SSVOH_CLOG << "Waiting for Steam ID validation...\n";

    int tries = 0;
    while(!_steamManager.got_encrypted_app_ticket_response())
    {
        _steamManager.run_callbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ++tries;

        if(tries > 15)
        {
            return fail("Never got Steam ID validation response");
        }
    }

    if(!_steamManager.got_encrypted_app_ticket())
    {
        return fail("Never got valid Steam encrypted app ticket");
    }

    const std::optional<std::uint64_t> ticketSteamId =
        _steamManager.get_ticket_steam_id();

    if(!ticketSteamId.has_value())
    {
        return fail("No Steam ID received from encrypted app ticket");
    }

    SSVOH_CLOG << "Successfully got validated Steam ID\n";

    _ticketSteamID = ticketSteamId;
    return true;
}

[[nodiscard]] bool HexagonClient::initializeTcpSocket()
{
    if(_socketConnected)
    {
        return fail("Socket already initialized");
    }

    _socket.setBlocking(true);

    SSVOH_CLOG << "Connecting socket to server...\n";

    if(_socket.connect(_serverIp, _serverPort,
           /* timeout */ sf::seconds(0.5)) != sf::Socket::Status::Done)
    {
        SSVOH_CLOG_ERROR << "Failure connecting socket to server\n";

        _socketConnected = false;
        return false;
    }

    _socket.setBlocking(false);
    _socketConnected = true;

    SSVOH_CLOG << "Socket connected to server\n";
    return true;
}

[[nodiscard]] bool HexagonClient::sendPacketRecursive(
    const int tries, sf::Packet& p)
{
    if(tries > 5)
    {
        return fail("Failure sending packet to server, too many tries");
    }

    const auto status = _socket.send(p);

    if(status == sf::Socket::Status::NotReady)
    {
        return fail();
    }

    if(status == sf::Socket::Status::Error)
    {
        return fail("Failure sending packet to server");
    }

    if(status == sf::Socket::Status::Disconnected)
    {
        return fail("Disconnected while sending packet to server");
    }

    if(status == sf::Socket::Status::Done)
    {
        return true;
    }

    SSVOH_ASSERT(status == sf::Socket::Status::Partial);
    return sendPacketRecursive(tries + 1, p);
}

[[nodiscard]] bool HexagonClient::recvPacketRecursive(
    const int tries, sf::Packet& p)
{
    if(tries > 5)
    {
        return fail("Failure receiving packet from server, too many tries");
    }

    const auto status = _socket.receive(p);

    if(status == sf::Socket::Status::NotReady)
    {
        return fail();
    }

    if(status == sf::Socket::Status::Error)
    {
        SSVOH_CLOG_ERROR << "Failure receiving packet from server\n";

        disconnect();
        return fail();
    }

    if(status == sf::Socket::Status::Disconnected)
    {
        SSVOH_CLOG_ERROR << "Disconnected while receiving packet from server\n";

        disconnect();
        return fail();
    }

    if(status == sf::Socket::Status::Done)
    {
        return true;
    }

    SSVOH_ASSERT(status == sf::Socket::Status::Partial);
    return recvPacketRecursive(tries + 1, p);
}

[[nodiscard]] bool HexagonClient::sendPacket(sf::Packet& p)
{
    return sendPacketRecursive(0, p);
}

[[nodiscard]] bool HexagonClient::recvPacket(sf::Packet& p)
{
    return recvPacketRecursive(0, p);
}

template <typename T>
[[nodiscard]] bool HexagonClient::sendUnencrypted(const T& data)
{
    if(!_socketConnected)
    {
        return fail();
    }

    makeClientToServerPacket(_packetBuffer, data);
    return sendPacket(_packetBuffer);
}

template <typename T>
[[nodiscard]] bool HexagonClient::sendEncrypted(const T& data)
{
    if(!_socketConnected)
    {
        return fail();
    }

    if(!_clientRTKeys.has_value())
    {
        return fail("Tried to send encrypted message without RT keys");
    }

    if(!makeClientToServerEncryptedPacket(
           _clientRTKeys->keyTransmit, _packetBuffer, data))
    {
        return fail("Error building encrypted message packet");
    }

    return sendPacket(_packetBuffer);
}

[[nodiscard]] bool HexagonClient::sendHeartbeat()
{
    if(!sendUnencrypted(CTSPHeartbeat{}))
    {
        return fail();
    }

    _lastHeartbeatTime = HRClock::now();
    return true;
}

[[nodiscard]] bool HexagonClient::sendDisconnect()
{
    return sendUnencrypted(CTSPDisconnect{});
}

[[nodiscard]] bool HexagonClient::sendPublicKey()
{
    return sendUnencrypted(CTSPPublicKey{_clientPSKeys.keyPublic});
}

[[nodiscard]] bool HexagonClient::sendRegister(const std::uint64_t steamId,
    const std::string& name, const std::string& passwordHash)
{
    SSVOH_CLOG_VERBOSE << "Sending registration request to server...\n";

    return sendEncrypted(                //
        CTSPRegister{
            .steamId = steamId,          //
            .name = name,                //
            .passwordHash = passwordHash //
        }                                //
    );
}

[[nodiscard]] bool HexagonClient::sendLogin(const std::uint64_t steamId,
    const std::string& name, const std::string& passwordHash)
{
    SSVOH_CLOG_VERBOSE << "Sending login request to server...\n";

    return sendEncrypted(                //
        CTSPLogin{
            .steamId = steamId,          //
            .name = name,                //
            .passwordHash = passwordHash //
        }                                //
    );
}

[[nodiscard]] bool HexagonClient::sendLogout(const std::uint64_t steamId)
{
    SSVOH_CLOG_VERBOSE << "Sending logout request to server...\n";
    return sendEncrypted(CTSPLogout{.steamId = steamId});
}

[[nodiscard]] bool HexagonClient::sendDeleteAccount(
    const std::uint64_t steamId, const std::string& passwordHash)
{
    SSVOH_CLOG_VERBOSE << "Sending delete account request to server...\n";

    return sendEncrypted(                //
        CTSPDeleteAccount{
            .steamId = steamId,          //
            .passwordHash = passwordHash //
        }                                //
    );
}

[[nodiscard]] bool HexagonClient::sendRequestTopScores(
    const std::uint64_t loginToken, const std::string& levelValidator)
{
    SSVOH_CLOG_VERBOSE << "Sending top scores request to server...\n";

    return sendEncrypted(                    //
        CTSPRequestTopScores{
            .loginToken = loginToken,        //
            .levelValidator = levelValidator //
        }                                    //
    );
}

[[nodiscard]] bool HexagonClient::sendRequestOwnScore(
    const std::uint64_t loginToken, const std::string& levelValidator)
{
    SSVOH_CLOG_VERBOSE << "Sending own score request to server...\n";

    return sendEncrypted(                    //
        CTSPRequestOwnScore{
            .loginToken = loginToken,        //
            .levelValidator = levelValidator //
        }                                    //
    );
}

[[nodiscard]] bool HexagonClient::sendRequestTopScoresAndOwnScore(
    const std::uint64_t loginToken, const std::string& levelValidator)
{
    SSVOH_CLOG_VERBOSE
        << "Sending top scores and own score request to server...\n";

    return sendEncrypted(                    //
        CTSPRequestTopScoresAndOwnScore{
            .loginToken = loginToken,        //
            .levelValidator = levelValidator //
        }                                    //
    );
}

[[nodiscard]] bool HexagonClient::sendStartedGame(
    const std::uint64_t loginToken, const std::string& levelValidator)
{
    SSVOH_CLOG_VERBOSE << "Sending started game packet to server...\n";

    return sendEncrypted(                    //
        CTSPStartedGame{
            .loginToken = loginToken,        //
            .levelValidator = levelValidator //
        }                                    //
    );
}

[[nodiscard]] bool HexagonClient::sendCompressedReplay(
    const std::uint64_t loginToken, const std::string& levelValidator,
    const compressed_replay_file& compressedReplayFile)
{
    SSVOH_CLOG_VERBOSE << "Sending compressed replay for level validator '"
                       << levelValidator << "' to server...\n";

    return sendEncrypted(                                //
        CTSPCompressedReplay{
            .loginToken = loginToken,                    //
            .compressedReplayFile = compressedReplayFile //
        }                                                //
    );
}

[[nodiscard]] bool HexagonClient::sendRequestServerStatus(
    const std::uint64_t loginToken)
{
    SSVOH_CLOG_VERBOSE << "Sending status request to server...\n";

    return sendEncrypted(             //
        CTSPRequestServerStatus{
            .loginToken = loginToken, //
        }                             //
    );
}

[[nodiscard]] bool HexagonClient::sendReady(const std::uint64_t loginToken)
{
    SSVOH_CLOG_VERBOSE << "Sending ready to server...\n";

    return sendEncrypted(             //
        CTSPReady{
            .loginToken = loginToken, //
        }                             //
    );
}

bool HexagonClient::connect()
{
    _state = State::Connecting;

    if(_socketConnected)
    {
        return fail("Socket already initialized");
    }

    const auto failEvent = [&](const std::string& reason)
    {
        const std::string errorStr = "Failure connecting, error " + reason;
        SSVOH_CLOG_ERROR << errorStr << '\n';

        addEvent(EConnectionFailure{errorStr});
        _state = State::ConnectionError;

        return false;
    };

    if(!initializeTcpSocket())
    {
        return failEvent("initializing TCP socket");
    }

    SSVOH_CLOG << "Sending first heartbeat...\n";
    if(!sendHeartbeat())
    {
        return failEvent("sending first heartbeat");
    }

    SSVOH_CLOG << "Sending public key...\n";
    if(!sendPublicKey())
    {
        return failEvent("sending public key");
    }

    addEvent(EConnectionSuccess{});
    _state = State::Connected;
    return true;
}

HexagonClient::HexagonClient(Steam::steam_manager& steamManager,
    const sf::IpAddress& serverIp, const unsigned short serverPort)
    : _steamManager{steamManager},
      _ticketSteamID{},
      _serverIp{serverIp},
      _serverPort{serverPort},
      _socket{},
      _socketConnected{false},
      _packetBuffer{},
      _errorOss{},
      _lastHeartbeatTime{},
      _verbose{true},
      _clientPSKeys{generateSodiumPSKeys()},
      _state{State::Disconnected},
      _loginToken{},
      _loginName{},
      _events{}
{
    const auto sKeyPublic = sodiumKeyToString(_clientPSKeys.keyPublic);
    const auto sKeySecret = sodiumKeyToString(_clientPSKeys.keySecret);

    SSVOH_CLOG << "Initializing client...\n"
               << " - " << SSVOH_CLOG_VAR(_serverIp) << '\n'
               << " - " << SSVOH_CLOG_VAR(_serverPort) << '\n'
               << " - " << SSVOH_CLOG_VAR(sKeyPublic) << '\n'
               << " - " << SSVOH_CLOG_VAR(sKeySecret) << '\n';

    if(!initializeTicketSteamID())
    {
        SSVOH_CLOG_ERROR << "Failure initializing client, no ticket Steam ID\n";

        _state = State::InitError;
        return;
    }

    connect();
}

HexagonClient::~HexagonClient()
{
    SSVOH_CLOG << "Uninitializing client...\n";
    disconnect();
    SSVOH_CLOG << "Client uninitialized\n";
}

void HexagonClient::disconnect()
{
    SSVOH_CLOG << "Disconnecting client...\n";

    _socket.setBlocking(true);

    if((_state == State::LoggedIn || _state == State::LoggedIn_Ready) &&
        _ticketSteamID.has_value())
    {
        (void)sendLogout(*_ticketSteamID);
    }

    if(_state == State::Connected || _state == State::LoggedIn ||
        _state == State::LoggedIn_Ready)
    {
        (void)sendDisconnect();
    }

    _socket.disconnect();
    _socketConnected = false;

    SSVOH_CLOG << "Client disconnected\n";

    _state = State::Disconnected;
}

bool HexagonClient::sendHeartbeatIfNecessary()
{
    if(!_socketConnected)
    {
        return true;
    }

    constexpr std::chrono::duration heatbeatInterval = std::chrono::seconds(45);

    if(HRClock::now() - _lastHeartbeatTime > heatbeatInterval)
    {
        if(!sendHeartbeat())
        {
            SSVOH_CLOG_ERROR
                << "Error sending heartbeat, disconnecting client\n";

            disconnect();
            return fail();
        }
    }

    return true;
}

bool HexagonClient::receiveDataFromServer(sf::Packet& p)
{
    if(!_socketConnected)
    {
        return fail();
    }

    if(!recvPacket(p))
    {
        return fail();
    }

    _errorOss.str("");
    const PVServerToClient pv = decodeServerToClientPacket(
        _clientRTKeys.has_value() ? &_clientRTKeys->keyReceive : nullptr,
        _errorOss, p);

    return Utils::match(
        pv,

        [&](const PInvalid&)
        {
            return fail("Error processing packet from server, details: ",
                _errorOss.str());
        },

        [&](const PEncryptedMsg&) {
            return fail(
                "Received non-decrypted encrypted msg packet from server");
        },

        [&](const STCPKick&)
        {
            SSVOH_CLOG << "Received kick packet from server, disconnecting\n";

            addEvent(EKicked{});

            disconnect();
            return true;
        },

        [&](const STCPPublicKey& stcp)
        {
            SSVOH_CLOG << "Received public key packet from server\n";

            if(_serverPublicKey.has_value())
            {
                SSVOH_CLOG << "Already had public key, replacing\n";
            }
            else
            {
                SSVOH_CLOG << "Did not have public key, setting\n";
            }

            _serverPublicKey = stcp.key;

            SSVOH_CLOG << "Server public key: '" << sodiumKeyToString(stcp.key)
                       << "'\n";

            SSVOH_CLOG << "Calculating RT keys\n";
            _clientRTKeys =
                calculateClientSessionSodiumRTKeys(_clientPSKeys, stcp.key);

            if(!_clientRTKeys.has_value())
            {
                SSVOH_CLOG_ERROR << "Failed calculating RT keys, disconnecting "
                                    "from server\n";

                disconnect();
                return fail();
            }

            const auto keyReceive =
                sodiumKeyToString(_clientRTKeys->keyReceive);

            const auto keyTransmit =
                sodiumKeyToString(_clientRTKeys->keyTransmit);

            SSVOH_CLOG << "Calculated RT keys\n"
                       << " - " << SSVOH_CLOG_VAR(keyReceive) << '\n'
                       << " - " << SSVOH_CLOG_VAR(keyTransmit) << '\n';

            SSVOH_CLOG << "Replying with login attempt\n";

            return true;
        },

        [&](const STCPRegistrationSuccess&)
        {
            SSVOH_CLOG << "Successfully registered to server\n";

            addEvent(ERegistrationSuccess{});
            return true;
        },

        [&](const STCPRegistrationFailure& stcp)
        {
            SSVOH_CLOG << "Registration to server failed, error: '"
                       << stcp.error << "'\n";

            addEvent(ERegistrationFailure{stcp.error});
            return true;
        },

        [&](const STCPLoginSuccess& stcp)
        {
            SSVOH_CLOG << "Successfully logged into server, token: '"
                       << stcp.loginToken << "'\n";

            if(_loginToken.has_value())
            {
                SSVOH_CLOG << "Already had login token, replacing\n";
            }
            else
            {
                SSVOH_CLOG << "Did not have login token, setting\n";
            }

            _loginToken = stcp.loginToken;
            _loginName = stcp.loginName;

            _state = State::LoggedIn;

            SSVOH_ASSERT(_loginToken.has_value());
            return sendRequestServerStatus(_loginToken.value());
        },

        [&](const STCPLoginFailure& stcp)
        {
            SSVOH_CLOG << "Login to server failed, error: '" << stcp.error
                       << "'\n";

            addEvent(ELoginFailure{stcp.error});
            return true;
        },

        [&](const STCPLogoutSuccess&)
        {
            SSVOH_CLOG << "Logout from server success\n";

            addEvent(ELogoutSuccess{});
            return true;
        },

        [&](const STCPLogoutFailure&)
        {
            SSVOH_CLOG << "Logout from server failure\n";

            addEvent(ELogoutFailure{});
            return true;
        },

        [&](const STCPDeleteAccountSuccess&)
        {
            SSVOH_CLOG << "Delete account from server success\n";

            addEvent(EDeleteAccountSuccess{});
            return true;
        },

        [&](const STCPDeleteAccountFailure& stcp)
        {
            SSVOH_CLOG << "Delete account from server failure, error: '"
                       << stcp.error << "'\n";

            addEvent(EDeleteAccountFailure{stcp.error});
            return true;
        },

        [&](const STCPTopScores& stcp)
        {
            SSVOH_CLOG << "Received top scores from server, levelValidator: '"
                       << stcp.levelValidator << "', size: '"
                       << stcp.scores.size() << "'\n";

            addEvent(EReceivedTopScores{
                .levelValidator = stcp.levelValidator, .scores = stcp.scores});

            return true;
        },

        [&](const STCPOwnScore& stcp)
        {
            SSVOH_CLOG << "Received own score from server, levelValidator: '"
                       << stcp.levelValidator << "'\n";

            addEvent(EReceivedOwnScore{
                .levelValidator = stcp.levelValidator, .score = stcp.score});

            return true;
        },

        [&](const STCPTopScoresAndOwnScore& stcp)
        {
            SSVOH_CLOG << "Received top scores and own score from server, "
                          "levelValidator: '"
                       << stcp.levelValidator << "'\n";

            addEvent(EReceivedTopScores{
                .levelValidator = stcp.levelValidator, .scores = stcp.scores});

            if(stcp.ownScore.has_value())
            {
                addEvent(
                    EReceivedOwnScore{.levelValidator = stcp.levelValidator,
                        .score = *stcp.ownScore});
            }

            return true;
        },

        [&](const STCPServerStatus& stcp)
        {
            SSVOH_CLOG << "Received server status from server\n";

            const auto& [serverProtocolVersion, serverGameVersion,
                supportedLevelValidatorsVector] = stcp;

            if(serverGameVersion != GAME_VERSION)
            {
                addEvent(EGameVersionMismatch{});
            }

            if(serverProtocolVersion != PROTOCOL_VERSION)
            {
                addEvent(EProtocolVersionMismatch{});
                disconnect();
                return true;
            }

            _levelValidatorsSupportedByServer.insert(
                supportedLevelValidatorsVector.begin(),
                supportedLevelValidatorsVector.end());

            _state = State::LoggedIn_Ready;
            addEvent(ELoginSuccess{});

            SSVOH_ASSERT(_loginToken.has_value());
            return sendReady(_loginToken.value());
        }

        //
    );
}

void HexagonClient::update()
{
    if(!_socketConnected)
    {
        return;
    }

    try
    {
        sendHeartbeatIfNecessary();
        receiveDataFromServer(_packetBuffer);
    }
    catch(const std::runtime_error& e)
    {
        SSVOH_CLOG_ERROR << "Exception: '" << e.what() << "'\n";
    }
    catch(...)
    {
        SSVOH_CLOG_ERROR << "Unknown exception";
    }
}

static std::string saltAndHashPwd(const std::string& password)
{
#if __has_include("SSVOpenHexagon/Online/SecretPasswordSalt.hpp")
    const std::string salt =
#include "SSVOpenHexagon/Online/SecretPasswordSalt.hpp"
        ;
#else
    const std::string salt = "salt";
#endif

    const std::string saltedPassword = salt + password;
    return sodiumHash(saltedPassword);
}

bool HexagonClient::tryRegister(
    const std::string& name, const std::string& password)
{
    if(!connectedAndInState(State::Connected))
    {
        return fail();
    }

    if(name.empty() || name.size() > 32 || password.empty())
    {
        addEvent(
            ERegistrationFailure{"Name or password fields too long or empty"});
        return false;
    }

    SSVOH_ASSERT(_ticketSteamID.has_value());
    return sendRegister(_ticketSteamID.value(), name, saltAndHashPwd(password));
}

bool HexagonClient::tryLogin(
    const std::string& name, const std::string& password)
{
    if(!connectedAndInState(State::Connected))
    {
        return fail();
    }

    if(name.empty() || name.size() > 32 || password.empty())
    {
        addEvent(ELoginFailure{"Name or password fields too long or empty"});
        return false;
    }

    SSVOH_ASSERT(_ticketSteamID.has_value());
    return sendLogin(_ticketSteamID.value(), name, saltAndHashPwd(password));
}

bool HexagonClient::tryLogoutFromServer()
{
    if(!connectedAndInAnyState(State::LoggedIn, State::LoggedIn_Ready))
    {
        return fail();
    }

    _state = State::Connected;
    _loginToken.reset();
    _loginName.reset();

    SSVOH_ASSERT(_ticketSteamID.has_value());
    return sendLogout(_ticketSteamID.value());
}

bool HexagonClient::tryDeleteAccount(const std::string& password)
{
    if(!connectedAndInState(State::Connected))
    {
        return fail();
    }

    SSVOH_ASSERT(_ticketSteamID.has_value());
    return sendDeleteAccount(_ticketSteamID.value(), saltAndHashPwd(password));
}

bool HexagonClient::tryRequestTopScores(const std::string& levelValidator)
{
    if(!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    SSVOH_ASSERT(_loginToken.has_value());
    return sendRequestTopScores(_loginToken.value(), levelValidator);
}

bool HexagonClient::trySendCompressedReplay(const std::string& levelValidator,
    const compressed_replay_file& compressedReplayFile)
{
    if(!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    if(!isLevelSupportedByServer(levelValidator))
    {
        SSVOH_CLOG_VERBOSE << "Not sending compressed replay for level '"
                           << levelValidator << "', unsupported by server\n";

        return true;
    }

    SSVOH_ASSERT(_loginToken.has_value());
    return sendCompressedReplay(
        _loginToken.value(), levelValidator, compressedReplayFile);
}

bool HexagonClient::tryRequestOwnScore(const std::string& levelValidator)
{
    if(!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    SSVOH_ASSERT(_loginToken.has_value());
    return sendRequestOwnScore(_loginToken.value(), levelValidator);
}

bool HexagonClient::tryRequestTopScoresAndOwnScore(
    const std::string& levelValidator)
{
    if(!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    SSVOH_ASSERT(_loginToken.has_value());
    return sendRequestTopScoresAndOwnScore(_loginToken.value(), levelValidator);
}

bool HexagonClient::trySendStartedGame(const std::string& levelValidator)
{
    if(!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    SSVOH_ASSERT(_loginToken.has_value());
    return sendStartedGame(_loginToken.value(), levelValidator);
}

[[nodiscard]] HexagonClient::State HexagonClient::getState() const noexcept
{
    return _state;
}

[[nodiscard]] bool HexagonClient::hasRTKeys() const noexcept
{
    return _clientRTKeys.has_value();
}

[[nodiscard]] const std::optional<std::string>&
HexagonClient::getLoginName() const noexcept
{
    return _loginName;
}

void HexagonClient::addEvent(const Event& e)
{
    _events.push_back(e);
}

[[nodiscard]] bool HexagonClient::connectedAndInState(
    const State s) const noexcept
{
    return _socketConnected && _state == s;
}

[[nodiscard]] bool HexagonClient::connectedAndInAnyState(
    const State s0, const State s1) const noexcept
{
    return _socketConnected && (_state == s0 || _state == s1);
}

[[nodiscard]] std::optional<HexagonClient::Event> HexagonClient::pollEvent()
{
    if(_events.empty())
    {
        return std::nullopt;
    }

    HG_SCOPE_GUARD({ _events.pop_front(); });
    return {_events.front()};
}

[[nodiscard]] bool HexagonClient::isLevelSupportedByServer(
    const std::string& levelValidator) const noexcept
{
    return _levelValidatorsSupportedByServer.contains(levelValidator);
}

} // namespace hg
