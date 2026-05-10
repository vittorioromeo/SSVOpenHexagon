// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonClient.hpp"
#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/ProtocolVersion.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"

#include "SFML/System/Thread.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Algorithm/Erase.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/String.hpp"

#include <stdexcept>


static auto& clog(const char* funcName)
{
    return ::hg::lo(::hg::Utils::concat("hg::HexagonClient::", funcName));
}

#define SSVOH_CLOG ::clog(__func__)

#define SSVOH_CLOG_VERBOSE \
    if (_verbose)          \
    ::clog(__func__)

#define SSVOH_CLOG_ERROR ::clog(__func__) << "[ERROR] "

#define SSVOH_CLOG_VAR(x) '\'' << #x << "': '" << x << '\''

#define SSVOH_CLOG_VAR_IP(x) '\'' << #x << "': '" << ::sf::IpAddressUtils::toString(x) << '\''

namespace hg
{

template <typename... Ts>
[[nodiscard]] bool HexagonClient::fail(const Ts&... xs)
{
    if constexpr (sizeof...(Ts) > 0)
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
    while (!_steamManager.got_encrypted_app_ticket_response())
    {
        _steamManager.run_callbacks();
        sf::ThisThread::sleepFor(sf::milliseconds(50));
        ++tries;

        if (tries > 15)
        {
            return fail("Never got Steam ID validation response");
        }
    }

    if (!_steamManager.got_encrypted_app_ticket())
    {
        return fail("Never got valid Steam encrypted app ticket");
    }

    const sf::base::Optional<sf::base::U64> ticketSteamId = _steamManager.get_ticket_steam_id();

    if (!ticketSteamId.hasValue())
    {
        return fail("No Steam ID received from encrypted app ticket");
    }

    SSVOH_CLOG << "Successfully got validated Steam ID\n";

    _ticketSteamID = ticketSteamId;
    return true;
}

[[nodiscard]] bool HexagonClient::initializeTcpSocket()
{
    if (_socket.hasValue())
    {
        return fail("Socket already initialized");
    }

    _socket = sf::TcpSocket::create(true /* isBlocking */);
    if (!_socket.hasValue())
    {
        return fail("Failure creating TCP socket");
    }

    SSVOH_CLOG << "Connecting socket to server...\n";

    if (_socket->connect(_serverIp,
                         _serverPort,
                         /* timeout */ sf::seconds(0.5)) != sf::Socket::Status::Done)
    {
        SSVOH_CLOG_ERROR << "Failure connecting socket to server\n";

        _socket.reset();
        return false;
    }

    _socket->setBlocking(false);

    SSVOH_CLOG << "Socket connected to server\n";
    return true;
}

[[nodiscard]] bool HexagonClient::sendPacketRecursive(const int tries, sf::Packet& p)
{
    if (tries > 5)
    {
        return fail("Failure sending packet to server, too many tries");
    }

    const auto status = _socket->send(p);

    if (status == sf::Socket::Status::NotReady)
    {
        return fail();
    }

    if (status == sf::Socket::Status::Error)
    {
        return fail("Failure sending packet to server");
    }

    if (status == sf::Socket::Status::Disconnected)
    {
        return fail("Disconnected while sending packet to server");
    }

    if (status == sf::Socket::Status::Done)
    {
        return true;
    }

    SSVOH_ASSERT(status == sf::Socket::Status::Partial);
    return sendPacketRecursive(tries + 1, p);
}

[[nodiscard]] bool HexagonClient::recvPacketRecursive(const int tries, sf::Packet& p)
{
    if (tries > 5)
    {
        return fail("Failure receiving packet from server, too many tries");
    }

    const auto status = _socket->receive(p);

    if (status == sf::Socket::Status::NotReady)
    {
        return fail();
    }

    if (status == sf::Socket::Status::Error)
    {
        SSVOH_CLOG_ERROR << "Failure receiving packet from server\n";

        disconnect();
        return fail();
    }

    if (status == sf::Socket::Status::Disconnected)
    {
        SSVOH_CLOG_ERROR << "Disconnected while receiving packet from server\n";

        disconnect();
        return fail();
    }

    if (status == sf::Socket::Status::Done)
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
    if (!_socket.hasValue())
    {
        return fail();
    }

    makeClientToServerPacket(_packetBuffer, data);
    return sendPacket(_packetBuffer);
}

template <typename T>
[[nodiscard]] bool HexagonClient::sendEncrypted(const T& data)
{
    if (!_socket.hasValue())
    {
        return fail();
    }

    if (!_clientRTKeys.hasValue())
    {
        return fail("Tried to send encrypted message without RT keys");
    }

    if (!makeClientToServerEncryptedPacket(_clientRTKeys->keyTransmit, _packetBuffer, data))
    {
        return fail("Error building encrypted message packet");
    }

    return sendPacket(_packetBuffer);
}

[[nodiscard]] bool HexagonClient::sendHeartbeat()
{
    if (!sendUnencrypted(CTSPHeartbeat{}))
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

[[nodiscard]] bool HexagonClient::sendRegister(const sf::base::U64     steamId,
                                               const sf::base::String& name,
                                               const sf::base::String& passwordHash)
{
    SSVOH_CLOG_VERBOSE << "Sending registration request to server...\n";

    return sendEncrypted( //
        CTSPRegister{
            .steamId      = steamId,     //
            .name         = name,        //
            .passwordHash = passwordHash //
        } //
    );
}

[[nodiscard]] bool HexagonClient::sendLogin(const sf::base::U64     steamId,
                                            const sf::base::String& name,
                                            const sf::base::String& passwordHash)
{
    SSVOH_CLOG_VERBOSE << "Sending login request to server...\n";

    return sendEncrypted( //
        CTSPLogin{
            .steamId      = steamId,     //
            .name         = name,        //
            .passwordHash = passwordHash //
        } //
    );
}

[[nodiscard]] bool HexagonClient::sendLogout(const sf::base::U64 steamId)
{
    SSVOH_CLOG_VERBOSE << "Sending logout request to server...\n";
    return sendEncrypted(CTSPLogout{.steamId = steamId});
}

[[nodiscard]] bool HexagonClient::sendDeleteAccount(const sf::base::U64 steamId, const sf::base::String& passwordHash)
{
    SSVOH_CLOG_VERBOSE << "Sending delete account request to server...\n";

    return sendEncrypted( //
        CTSPDeleteAccount{
            .steamId      = steamId,     //
            .passwordHash = passwordHash //
        } //
    );
}

[[nodiscard]] bool HexagonClient::sendRequestTopScores(const sf::base::U64 loginToken, const sf::base::String& levelValidator)
{
    SSVOH_CLOG_VERBOSE << "Sending top scores request to server...\n";

    return sendEncrypted( //
        CTSPRequestTopScores{
            .loginToken     = loginToken,    //
            .levelValidator = levelValidator //
        } //
    );
}

[[nodiscard]] bool HexagonClient::sendRequestOwnScore(const sf::base::U64 loginToken, const sf::base::String& levelValidator)
{
    SSVOH_CLOG_VERBOSE << "Sending own score request to server...\n";

    return sendEncrypted( //
        CTSPRequestOwnScore{
            .loginToken     = loginToken,    //
            .levelValidator = levelValidator //
        } //
    );
}

[[nodiscard]] bool HexagonClient::sendRequestTopScoresAndOwnScore(const sf::base::U64     loginToken,
                                                                  const sf::base::String& levelValidator)
{
    SSVOH_CLOG_VERBOSE << "Sending top scores and own score request to server...\n";

    return sendEncrypted( //
        CTSPRequestTopScoresAndOwnScore{
            .loginToken     = loginToken,    //
            .levelValidator = levelValidator //
        } //
    );
}

[[nodiscard]] bool HexagonClient::sendRequestReplay(const sf::base::U64     loginToken,
                                                    const sf::base::String& levelValidator,
                                                    const sf::base::U64     scoreTimestamp)
{
    SSVOH_CLOG_VERBOSE << "Sending replay request to server...\n";

    return sendEncrypted( //
        CTSPRequestReplay{
            .loginToken     = loginToken,     //
            .levelValidator = levelValidator, //
            .scoreTimestamp = scoreTimestamp  //
        });
}

[[nodiscard]] bool HexagonClient::sendStartedGame(const sf::base::U64 loginToken, const sf::base::String& levelValidator)
{
    SSVOH_CLOG_VERBOSE << "Sending started game packet to server...\n";

    return sendEncrypted( //
        CTSPStartedGame{
            .loginToken     = loginToken,    //
            .levelValidator = levelValidator //
        } //
    );
}

[[nodiscard]] bool HexagonClient::sendCompressedReplay(const sf::base::U64           loginToken,
                                                       const sf::base::String&       levelValidator,
                                                       const compressed_replay_file& compressedReplayFile)
{
    SSVOH_CLOG_VERBOSE << "Sending compressed replay for level validator '" << levelValidator << "' to server...\n";

    return sendEncrypted( //
        CTSPCompressedReplay{
            .loginToken           = loginToken,          //
            .compressedReplayFile = compressedReplayFile //
        } //
    );
}

[[nodiscard]] bool HexagonClient::sendRequestServerStatus(const sf::base::U64 loginToken)
{
    SSVOH_CLOG_VERBOSE << "Sending status request to server...\n";

    return sendEncrypted( //
        CTSPRequestServerStatus{
            .loginToken = loginToken, //
        } //
    );
}

[[nodiscard]] bool HexagonClient::sendReady(const sf::base::U64 loginToken)
{
    SSVOH_CLOG_VERBOSE << "Sending ready to server...\n";

    return sendEncrypted( //
        CTSPReady{
            .loginToken = loginToken, //
        } //
    );
}

bool HexagonClient::connect()
{
    _state = State::Connecting;

    if (_socket.hasValue())
    {
        return fail("Socket already initialized");
    }

    const auto failEvent = [&](const sf::base::String& reason)
    {
        const sf::base::String errorStr = "Failure connecting, error " + reason;
        SSVOH_CLOG_ERROR << errorStr << '\n';

        addEvent(Event{EConnectionFailure{errorStr}});
        _state = State::ConnectionError;

        return false;
    };

    if (!initializeTcpSocket())
    {
        return failEvent("initializing TCP socket");
    }

    SSVOH_CLOG << "Sending first heartbeat...\n";
    if (!sendHeartbeat())
    {
        return failEvent("sending first heartbeat");
    }

    SSVOH_CLOG << "Sending public key...\n";
    if (!sendPublicKey())
    {
        return failEvent("sending public key");
    }

    // State transitions to `Connected` immediately so the host UI can
    // reflect the TCP-level connection. The user-visible
    // `EConnectionSuccess` event is held back until the server replies
    // with its public key and we can derive RT keys -- only then is the
    // connection actually usable for login/register. Without that gate,
    // a non-responsive server (firewalled, stale, mismatched build)
    // would surface "CONNECTION SUCCESS" while every encrypted send
    // silently fails because RT keys never landed.
    _state = State::Connected;
    return true;
}

HexagonClient::HexagonClient(Steam::steam_manager& steamManager, const sf::IpAddress& serverIp, const unsigned short serverPort) :
    _steamManager{steamManager},
    _ticketSteamID{},
    _serverIp{serverIp},
    _serverPort{serverPort},
    _socket{},
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
               << " - " << SSVOH_CLOG_VAR_IP(_serverIp) << '\n'
               << " - " << SSVOH_CLOG_VAR(_serverPort) << '\n'
               << " - " << SSVOH_CLOG_VAR(sKeyPublic) << '\n'
               << " - " << SSVOH_CLOG_VAR(sKeySecret) << '\n';

    if (!initializeTicketSteamID())
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

    if (_socket.hasValue())
    {
        _socket->setBlocking(true);

        if ((_state == State::LoggedIn || _state == State::LoggedIn_Ready) && _ticketSteamID.hasValue())
        {
            (void)sendLogout(*_ticketSteamID);
        }

        if (_state == State::Connected || _state == State::LoggedIn || _state == State::LoggedIn_Ready)
        {
            (void)sendDisconnect();
        }

        _socket->disconnect();
        _socket.reset();
    }

    SSVOH_CLOG << "Client disconnected\n";

    _state = State::Disconnected;
}

bool HexagonClient::sendHeartbeatIfNecessary()
{
    if (!_socket.hasValue())
    {
        return true;
    }

    constexpr std::chrono::duration heatbeatInterval = std::chrono::seconds(45);

    if (HRClock::now() - _lastHeartbeatTime > heatbeatInterval)
    {
        if (!sendHeartbeat())
        {
            SSVOH_CLOG_ERROR << "Error sending heartbeat, disconnecting client\n";

            disconnect();
            return fail();
        }
    }

    return true;
}

bool HexagonClient::receiveDataFromServer(sf::Packet& p)
{
    if (!_socket.hasValue())
    {
        return fail();
    }

    if (!recvPacket(p))
    {
        return fail();
    }

    _errorOss.setStr("");
    const PVServerToClient pv = decodeServerToClientPacket(_clientRTKeys.hasValue() ? &_clientRTKeys->keyReceive : nullptr,
                                                           _errorOss,
                                                           p);

    return pv.linearMatch( //

        [&](const PInvalid&) { return fail("Error processing packet from server, details: ", _errorOss.getString()); },

        [&](const PEncryptedMsg&) { return fail("Received non-decrypted encrypted msg packet from server"); },

        [&](const STCPKick&)
    {
        SSVOH_CLOG << "Received kick packet from server, disconnecting\n";

        addEvent(Event{EKicked{}});

        disconnect();
        return true;
    },

        [&](const STCPPublicKey& stcp)
    {
        SSVOH_CLOG << "Received public key packet from server\n";

        if (_serverPublicKey.hasValue())
        {
            SSVOH_CLOG << "Already had public key, replacing\n";
        }
        else
        {
            SSVOH_CLOG << "Did not have public key, setting\n";
        }

        _serverPublicKey.emplace(stcp.key);

        SSVOH_CLOG << "Server public key: '" << sodiumKeyToString(stcp.key) << "'\n";

        SSVOH_CLOG << "Calculating RT keys\n";
        _clientRTKeys = calculateClientSessionSodiumRTKeys(_clientPSKeys, stcp.key);

        if (!_clientRTKeys.hasValue())
        {
            SSVOH_CLOG_ERROR << "Failed calculating RT keys, disconnecting "
                                "from server\n";

            disconnect();
            return fail();
        }

        const auto keyReceive = sodiumKeyToString(_clientRTKeys->keyReceive);

        const auto keyTransmit = sodiumKeyToString(_clientRTKeys->keyTransmit);

        SSVOH_CLOG << "Calculated RT keys\n"
                   << " - " << SSVOH_CLOG_VAR(keyReceive) << '\n'
                   << " - " << SSVOH_CLOG_VAR(keyTransmit) << '\n';

        // Now that RT keys exist, the connection is actually usable for
        // encrypted traffic -- this is the right moment to surface
        // "connection succeeded" to the user. `connect()` only signals
        // TCP-level success; old-style behaviour fired this event right
        // after sending the client's public key, which gave the user a
        // green-light before encryption was negotiated and made
        // login/register silently no-op against unresponsive servers.
        addEvent(Event{EConnectionSuccess{}});

        return true;
    },

        [&](const STCPRegistrationSuccess&)
    {
        SSVOH_CLOG << "Successfully registered to server\n";

        addEvent(Event{ERegistrationSuccess{}});
        return true;
    },

        [&](const STCPRegistrationFailure& stcp)
    {
        SSVOH_CLOG << "Registration to server failed, error: '" << stcp.error << "'\n";

        addEvent(Event{ERegistrationFailure{sf::base::String(stcp.error)}});
        return true;
    },

        [&](const STCPLoginSuccess& stcp)
    {
        SSVOH_CLOG << "Successfully logged into server, token: '" << stcp.loginToken << "'\n";

        if (_loginToken.hasValue())
        {
            SSVOH_CLOG << "Already had login token, replacing\n";
        }
        else
        {
            SSVOH_CLOG << "Did not have login token, setting\n";
        }

        _loginToken.emplace(stcp.loginToken);
        _loginName.emplace(sf::base::String(stcp.loginName));

        _state = State::LoggedIn;

        SSVOH_ASSERT(_loginToken.hasValue());
        return sendRequestServerStatus(_loginToken.value());
    },

        [&](const STCPLoginFailure& stcp)
    {
        SSVOH_CLOG << "Login to server failed, error: '" << stcp.error << "'\n";

        addEvent(Event{ELoginFailure{sf::base::String(stcp.error)}});
        return true;
    },

        [&](const STCPLogoutSuccess&)
    {
        SSVOH_CLOG << "Logout from server success\n";

        addEvent(Event{ELogoutSuccess{}});
        return true;
    },

        [&](const STCPLogoutFailure&)
    {
        SSVOH_CLOG << "Logout from server failure\n";

        addEvent(Event{ELogoutFailure{}});
        return true;
    },

        [&](const STCPDeleteAccountSuccess&)
    {
        SSVOH_CLOG << "Delete account from server success\n";

        addEvent(Event{EDeleteAccountSuccess{}});
        return true;
    },

        [&](const STCPDeleteAccountFailure& stcp)
    {
        SSVOH_CLOG << "Delete account from server failure, error: '" << stcp.error << "'\n";

        addEvent(Event{EDeleteAccountFailure{sf::base::String(stcp.error)}});
        return true;
    },

        [&](const STCPTopScores& stcp)
    {
        SSVOH_CLOG << "Received top scores from server, levelValidator: '" << stcp.levelValidator << "', size: '"
                   << stcp.scores.size() << "'\n";

        addEvent(Event{EReceivedTopScores{.levelValidator = sf::base::String(stcp.levelValidator), .scores = stcp.scores}});

        return true;
    },

        [&](const STCPOwnScore& stcp)
    {
        SSVOH_CLOG << "Received own score from server, levelValidator: '" << stcp.levelValidator << "'\n";

        addEvent(Event{EReceivedOwnScore{.levelValidator = sf::base::String(stcp.levelValidator), .score = stcp.score}});

        return true;
    },

        [&](const STCPTopScoresAndOwnScore& stcp)
    {
        SSVOH_CLOG << "Received top scores and own score from server, "
                      "levelValidator: '"
                   << stcp.levelValidator << "'\n";

        addEvent(Event{EReceivedTopScores{.levelValidator = sf::base::String(stcp.levelValidator), .scores = stcp.scores}});

        if (stcp.ownScore.hasValue())
        {
            addEvent(Event{
                EReceivedOwnScore{.levelValidator = sf::base::String(stcp.levelValidator), .score = *stcp.ownScore}});
        }

        return true;
    },

        [&](const STCPServerStatus& stcp)
    {
        SSVOH_CLOG << "Received server status from server\n";

        const auto& [serverProtocolVersion, serverGameVersion, supportedLevelValidatorsVector] = stcp;

        if (serverGameVersion != GAME_VERSION)
        {
            addEvent(Event{EGameVersionMismatch{}});
        }

        if (serverProtocolVersion != PROTOCOL_VERSION)
        {
            addEvent(Event{EProtocolVersionMismatch{}});
            disconnect();
            return true;
        }

        _levelValidatorsSupportedByServer.insert(supportedLevelValidatorsVector.begin(),
                                                 supportedLevelValidatorsVector.end());

        _state = State::LoggedIn_Ready;
        addEvent(Event{ELoginSuccess{}});

        SSVOH_ASSERT(_loginToken.hasValue());
        return sendReady(_loginToken.value());
    },

        [&](const STCPReplayData& stcp)
    {
        SSVOH_CLOG << "Received replay from server, levelValidator: '" << stcp.levelValidator << "', scoreTimestamp: '"
                   << stcp.scoreTimestamp << "'\n";

        // Decompress here so the UI side gets a ready-to-play `replay_file`.
        // Failure is rare (only if the bytes were corrupted on the wire)
        // and is surfaced as `EReplayUnavailable` so the UI can show the
        // same empty-state path as a missing-file response.
        const sf::base::Optional<replay_file> rfOpt = decompress_replay_file(stcp.replay);
        if (!rfOpt.hasValue())
        {
            addEvent(Event{EReplayUnavailable{
                .levelValidator = sf::base::String(stcp.levelValidator),
                .scoreTimestamp = stcp.scoreTimestamp,
                .reason         = sf::base::String("decompression failed"),
            }});

            return true;
        }

        addEvent(Event{EReceivedReplay{
            .levelValidator = sf::base::String(stcp.levelValidator),
            .scoreTimestamp = stcp.scoreTimestamp,
            .replay         = *rfOpt,
        }});

        return true;
    },

        [&](const STCPReplayUnavailable& stcp)
    {
        SSVOH_CLOG << "Replay unavailable from server, levelValidator: '" << stcp.levelValidator
                   << "', scoreTimestamp: '" << stcp.scoreTimestamp << "', reason: '" << stcp.reason << "'\n";

        addEvent(Event{EReplayUnavailable{
            .levelValidator = sf::base::String(stcp.levelValidator),
            .scoreTimestamp = stcp.scoreTimestamp,
            .reason         = sf::base::String(stcp.reason),
        }});

        return true;
    }

        //
    );
}

void HexagonClient::update()
{
    if (!_socket.hasValue())
    {
        return;
    }

    try
    {
        sendHeartbeatIfNecessary();
        receiveDataFromServer(_packetBuffer);
    } catch (const std::runtime_error& e)
    {
        SSVOH_CLOG_ERROR << "Exception: '" << e.what() << "'\n";
    } catch (...)
    {
        SSVOH_CLOG_ERROR << "Unknown exception";
    }
}

static sf::base::String saltAndHashPwd(const sf::base::String& password)
{
#if __has_include("SSVOpenHexagon/Online/SecretPasswordSalt.hpp")
    const sf::base::String salt =
    #include "SSVOpenHexagon/Online/SecretPasswordSalt.hpp"
        ;
#else
    const sf::base::String salt = "salt";
#endif

    const sf::base::String saltedPassword = salt + password;
    return sodiumHash(saltedPassword);
}

bool HexagonClient::tryRegister(const sf::base::String& name, const sf::base::String& password)
{
    if (!connectedAndInState(State::Connected))
    {
        return fail();
    }

    if (name.empty() || name.size() > 32 || password.empty())
    {
        addEvent(Event{ERegistrationFailure{"Name or password fields too long or empty"}});
        return false;
    }

    if (!_ticketSteamID.hasValue())
    {
        addEvent(Event{ERegistrationFailure{"No Steam ticket - restart with Steam running"}});
        return false;
    }

    // The encrypted send path needs RT keys, which only land after the
    // server replies to our public key. Without this check the call
    // silently no-ops -- the user sees a connected server, clicks
    // REGISTER, and gets nothing back. Surface it as an explicit failure
    // so they know to wait or that the server is unresponsive.
    if (!_clientRTKeys.hasValue())
    {
        addEvent(Event{ERegistrationFailure{"Key exchange with server not yet complete - wait a moment and retry"}});
        return false;
    }

    return sendRegister(_ticketSteamID.value(), name, saltAndHashPwd(password));
}

bool HexagonClient::tryLogin(const sf::base::String& name, const sf::base::String& password)
{
    if (!connectedAndInState(State::Connected))
    {
        return fail();
    }

    if (name.empty() || name.size() > 32 || password.empty())
    {
        addEvent(Event{ELoginFailure{"Name or password fields too long or empty"}});
        return false;
    }

    if (!_ticketSteamID.hasValue())
    {
        addEvent(Event{ELoginFailure{"No Steam ticket - restart with Steam running"}});
        return false;
    }

    if (!_clientRTKeys.hasValue())
    {
        addEvent(Event{ELoginFailure{"Key exchange with server not yet complete - wait a moment and retry"}});
        return false;
    }

    return sendLogin(_ticketSteamID.value(), name, saltAndHashPwd(password));
}

bool HexagonClient::tryLogoutFromServer()
{
    if (!connectedAndInAnyState(State::LoggedIn, State::LoggedIn_Ready))
    {
        return fail();
    }

    _state = State::Connected;
    _loginToken.reset();
    _loginName.reset();

    if (!_ticketSteamID.hasValue())
    {
        return false;
    }
    return sendLogout(_ticketSteamID.value());
}

bool HexagonClient::tryDeleteAccount(const sf::base::String& password)
{
    if (!connectedAndInState(State::Connected))
    {
        return fail();
    }

    if (!_ticketSteamID.hasValue())
    {
        addEvent(Event{EDeleteAccountFailure{"No Steam ticket - restart with Steam running"}});
        return false;
    }

    if (!_clientRTKeys.hasValue())
    {
        addEvent(Event{EDeleteAccountFailure{"Key exchange with server not yet complete - wait a moment and retry"}});
        return false;
    }

    return sendDeleteAccount(_ticketSteamID.value(), saltAndHashPwd(password));
}

bool HexagonClient::tryRequestTopScores(const sf::base::String& levelValidator)
{
    if (!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    SSVOH_ASSERT(_loginToken.hasValue());
    return sendRequestTopScores(_loginToken.value(), levelValidator);
}

bool HexagonClient::trySendCompressedReplay(const sf::base::String&       levelValidator,
                                            const compressed_replay_file& compressedReplayFile)
{
    if (!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    if (!isLevelSupportedByServer(levelValidator))
    {
        SSVOH_CLOG_VERBOSE << "Not sending compressed replay for level '" << levelValidator << "', unsupported by server\n";

        return true;
    }

    SSVOH_ASSERT(_loginToken.hasValue());
    return sendCompressedReplay(_loginToken.value(), levelValidator, compressedReplayFile);
}

bool HexagonClient::tryRequestOwnScore(const sf::base::String& levelValidator)
{
    if (!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    SSVOH_ASSERT(_loginToken.hasValue());
    return sendRequestOwnScore(_loginToken.value(), levelValidator);
}

bool HexagonClient::tryRequestTopScoresAndOwnScore(const sf::base::String& levelValidator)
{
    if (!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    SSVOH_ASSERT(_loginToken.hasValue());
    return sendRequestTopScoresAndOwnScore(_loginToken.value(), levelValidator);
}

bool HexagonClient::tryRequestReplay(const sf::base::String& levelValidator, const sf::base::U64 scoreTimestamp)
{
    if (!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    SSVOH_ASSERT(_loginToken.hasValue());
    return sendRequestReplay(_loginToken.value(), levelValidator, scoreTimestamp);
}

bool HexagonClient::trySendStartedGame(const sf::base::String& levelValidator)
{
    if (!connectedAndInState(State::LoggedIn_Ready))
    {
        return fail();
    }

    SSVOH_ASSERT(_loginToken.hasValue());
    return sendStartedGame(_loginToken.value(), levelValidator);
}

[[nodiscard]] HexagonClient::State HexagonClient::getState() const noexcept
{
    return _state;
}

[[nodiscard]] bool HexagonClient::hasRTKeys() const noexcept
{
    return _clientRTKeys.hasValue();
}

[[nodiscard]] const sf::base::Optional<sf::base::String>& HexagonClient::getLoginName() const noexcept
{
    return _loginName;
}

void HexagonClient::addEvent(const Event& e)
{
    _events.pushBack(e);
}

[[nodiscard]] bool HexagonClient::connectedAndInState(const State s) const noexcept
{
    return _socket.hasValue() && _state == s;
}

[[nodiscard]] bool HexagonClient::connectedAndInAnyState(const State s0, const State s1) const noexcept
{
    return _socket.hasValue() && (_state == s0 || _state == s1);
}

[[nodiscard]] sf::base::Optional<HexagonClient::Event> HexagonClient::pollEvent()
{
    if (_events.empty())
    {
        return sf::base::nullOpt;
    }

    SFML_BASE_SCOPE_GUARD({ _events.eraseAt(0); });
    return sf::base::makeOptional(_events.front());
}

void HexagonClient::discardPendingReplayEvents() noexcept
{
    sf::base::vectorEraseIf(_events, [](const Event& e) { return e.is<EReceivedReplay>() || e.is<EReplayUnavailable>(); });
}

[[nodiscard]] bool HexagonClient::isLevelSupportedByServer(const sf::base::String& levelValidator) const noexcept
{
    return _levelValidatorsSupportedByServer.contains(levelValidator);
}

} // namespace hg
