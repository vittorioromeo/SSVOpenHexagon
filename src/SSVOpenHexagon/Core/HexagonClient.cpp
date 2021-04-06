// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonClient.hpp"

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"
#include "SSVOpenHexagon/Core/Steam.hpp"
#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Utils/Match.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include <SFML/Network/Packet.hpp>

#include <thread>
#include <chrono>

static auto& clog(const char* funcName)
{
    return ::ssvu::lo(::hg::Utils::concat("hg::HexagonClient::", funcName));
}

#define SSVOH_CLOG ::clog(__func__)

#define SSVOH_CLOG_VERBOSE \
    if(_verbose) ::clog(__func__)

#define SSVOH_CLOG_ERROR ::clog(__func__) << "[ERROR] "

#define SSVOH_CLOG_VAR(x) '\'' << #x << "': '" << x << '\''

namespace hg
{

[[nodiscard]] bool HexagonClient::initializeTicketSteamID()
{
    SSVOH_CLOG << "Waiting for Steam ID validation...\n";

    int tries = 0;
    while(!_steamManager.got_encrypted_app_ticket_response())
    {
        _steamManager.run_callbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        ++tries;

        if(tries > 6)
        {
            SSVOH_CLOG_ERROR << "Never got Steam ID validation response\n";
            return false;
        }
    }

    if(!_steamManager.got_encrypted_app_ticket())
    {
        SSVOH_CLOG_ERROR << "Never got valid Steam encrypted app ticket\n";
        return false;
    }

    const std::optional<CSteamID> ticketSteamId =
        _steamManager.get_ticket_steam_id();

    if(!ticketSteamId.has_value())
    {
        SSVOH_CLOG_ERROR << "No Steam ID received from encrypted app ticket\n";
        return false;
    }

    SSVOH_CLOG << "Successfully got validated Steam ID\n";
    _ticketSteamID = ticketSteamId->ConvertToUint64();
    return true;
}

[[nodiscard]] bool HexagonClient::initializeTcpSocket()
{
    if(_socketConnected)
    {
        SSVOH_CLOG_ERROR << "Socket already initialized\n";
        return false;
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

    return true;
}

[[nodiscard]] bool HexagonClient::sendPacketRecursive(
    const int tries, sf::Packet& p)
{
    if(tries > 5)
    {
        SSVOH_CLOG_ERROR
            << "Failure receiving packet from server, too many tries\n";

        return false;
    }

    const auto status = _socket.send(p);

    if(status == sf::Socket::Status::NotReady)
    {
        return false;
    }

    if(status == sf::Socket::Status::Error)
    {
        SSVOH_CLOG_ERROR << "Failure sending packet to server\n";

        disconnect();
        return false;
    }

    if(status == sf::Socket::Status::Disconnected)
    {
        SSVOH_CLOG_ERROR << "Disconnected while sending packet to server\n";

        disconnect();
        return false;
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
        SSVOH_CLOG_ERROR
            << "Failure receiving packet from server, too many tries\n";

        return false;
    }

    const auto status = _socket.receive(p);

    if(status == sf::Socket::Status::NotReady)
    {
        return false;
    }

    if(status == sf::Socket::Status::Error)
    {
        SSVOH_CLOG_ERROR << "Failure receiving packet from server\n";

        disconnect();
        return false;
    }

    if(status == sf::Socket::Status::Disconnected)
    {
        SSVOH_CLOG_ERROR << "Disconnected while receiving packet from server\n";

        disconnect();
        return false;
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

[[nodiscard]] bool HexagonClient::sendHeartbeat()
{
    if(!_socketConnected)
    {
        return false;
    }

    makeClientToServerPacket(_packetBuffer, CTSPHeartbeat{});

    if(!sendPacket(_packetBuffer))
    {
        return false;
    }

    _lastHeartbeatTime = Clock::now();
    return true;
}

[[nodiscard]] bool HexagonClient::sendPublicKey()
{
    if(!_socketConnected)
    {
        return false;
    }

    makeClientToServerPacket(
        _packetBuffer, CTSPPublicKey{_clientPSKeys.keyPublic});

    return sendPacket(_packetBuffer);
}

[[nodiscard]] bool HexagonClient::sendReady()
{
    if(!_socketConnected)
    {
        return false;
    }

    makeClientToServerPacket(_packetBuffer, CTSPReady{});
    return sendPacket(_packetBuffer);
}

template <typename T>
[[nodiscard]] bool HexagonClient::sendEncrypted(const T& data)
{
    if(!_socketConnected)
    {
        return false;
    }

    if(!_clientRTKeys.has_value())
    {
        SSVOH_CLOG_ERROR << "Tried to send encrypted message without RT keys\n";
        return false;
    }

    if(!makeClientToServerEncryptedPacket(
           _clientRTKeys->keyTransmit, _packetBuffer, data))
    {
        SSVOH_CLOG_ERROR << "Error building encrypted message packet\n";
        return false;
    }

    return sendPacket(_packetBuffer);
}

[[nodiscard]] bool HexagonClient::sendPrint(const std::string& s)
{
    return sendEncrypted(CTSPPrint{s});
}

[[nodiscard]] bool HexagonClient::sendRegister(const std::uint64_t steamId,
    const std::string& name, const std::string& passwordHash)
{
    SSVOH_CLOG << "Sending registration request to server...\n";
    return sendEncrypted(CTSPRegister{steamId, name, passwordHash});
}

[[nodiscard]] bool HexagonClient::sendLogin(const std::uint64_t steamId,
    const std::string& name, const std::string& passwordHash)
{
    SSVOH_CLOG << "Sending login request to server...\n";
    return sendEncrypted(CTSPLogin{steamId, name, passwordHash});
}

bool HexagonClient::connect()
{
    _state = State::Connecting;

    if(_socketConnected)
    {
        SSVOH_CLOG_ERROR << "Socket already initialized\n";
        return false;
    }

    if(!initializeTcpSocket())
    {
        SSVOH_CLOG_ERROR
            << "Failure connecting, error initializing TCP socket\n";

        _state = State::ConnectionError;
        return false;
    }

    if(!sendHeartbeat())
    {
        SSVOH_CLOG_ERROR
            << "Failure connecting, error sending first heartbeat\n";

        _state = State::ConnectionError;
        return false;
    }

    if(!sendPublicKey())
    {
        SSVOH_CLOG_ERROR << "Failure connecting, error sending public key\n";

        _state = State::ConnectionError;
        return false;
    }

    _state = State::Connected;
    return true;
}

HexagonClient::HexagonClient(Steam::steam_manager& steamManager)
    : _steamManager{steamManager}, _serverIp{Config::getServerIp()},
      _serverPort{Config::getServerPort()}, _socket{}, _socketConnected{false},
      _packetBuffer{}, _errorOss{}, _lastHeartbeatTime{}, _verbose{true},
      _clientPSKeys{generateSodiumPSKeys()}, _state{State::Disconnected}
{
    const auto sKeyPublic = sodiumKeyToString(_clientPSKeys.keyPublic);
    const auto sKeySecret = sodiumKeyToString(_clientPSKeys.keySecret);

    SSVOH_CLOG << "Initializing client...\n"
               << " - " << SSVOH_CLOG_VAR(_serverIp) << '\n'
               << " - " << SSVOH_CLOG_VAR(_serverPort) << '\n'
               << " - " << SSVOH_CLOG_VAR(sKeyPublic) << '\n'
               << " - " << SSVOH_CLOG_VAR(sKeySecret) << '\n';

    if(_serverIp == sf::IpAddress::None)
    {
        SSVOH_CLOG_ERROR << "Failure initializing client, invalid ip address '"
                         << Config::getServerIp() << "'\n";

        _state = State::InitError;
        return;
    }

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

    makeClientToServerPacket(_packetBuffer, CTSPDisconnect{});
    _socket.send(_packetBuffer);

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

    if(Clock::now() - _lastHeartbeatTime > heatbeatInterval)
    {
        if(!sendHeartbeat())
        {
            SSVOH_CLOG_ERROR
                << "Error sending heartbeat, disconnecting client\n";

            disconnect();
            return false;
        }
    }

    return true;
}

bool HexagonClient::receiveDataFromServer(sf::Packet& p)
{
    if(!_socketConnected)
    {
        return false;
    }

    if(!recvPacket(p))
    {
        return false;
    }

    _errorOss.str("");
    const PVServerToClient pv = decodeServerToClientPacket(
        _clientRTKeys.has_value() ? &_clientRTKeys->keyReceive : nullptr,
        _errorOss, p);

    return Utils::match(
        pv,

        [&](const PInvalid&) {
            SSVOH_CLOG_ERROR << "Error processing packet from server, details: "
                             << _errorOss.str() << '\n';

            return false;
        },

        [&](const PEncryptedMsg&) {
            SSVOH_CLOG
                << "Received non-decrypted encrypted msg packet from server\n";

            return false;
        },

        [&](const STCPKick&) {
            SSVOH_CLOG << "Received kick packet from server, disconnecting\n";

            disconnect();
            return true;
        },

        [&](const STCPPublicKey& stcp) {
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
                return false;
            }

            const auto keyReceive =
                sodiumKeyToString(_clientRTKeys->keyReceive);
            const auto keyTransmit =
                sodiumKeyToString(_clientRTKeys->keyTransmit);

            SSVOH_CLOG << "Calculated RT keys\n"
                       << " - " << SSVOH_CLOG_VAR(keyReceive) << '\n'
                       << " - " << SSVOH_CLOG_VAR(keyTransmit) << '\n';

            SSVOH_CLOG << "Replying with login attempt\n";

            // TODO: get rid of these packet types, send login attempt here,
            // display registration reminder in case of failure

            return sendReady() && sendPrint("hello world!!!");
        },

        [&](const STCPRegistrationSuccess& stcp) {
            SSVOH_CLOG << "Successfully registered to server\n";
            // TODO
            return true;
        },

        [&](const STCPRegistrationFailure& stcp) {
            SSVOH_CLOG << "Registration to server failed, error: '"
                       << stcp.error << "'\n";
            // TODO
            return true;
        },

        [&](const STCPLoginSuccess& stcp) {
            SSVOH_CLOG << "Successfully logged into server\n";

            _state = State::LoggedIn;
            // TODO
            return true;
        },

        [&](const STCPLoginFailure& stcp) {
            SSVOH_CLOG << "Login to server failed, error: '" << stcp.error
                       << "'\n";
            // TODO
            return true;
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

    sendHeartbeatIfNecessary();
    receiveDataFromServer(_packetBuffer);
}

static std::string hashPwd(const std::string& password)
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

bool HexagonClient::tryRegisterToServer(
    const std::string& name, const std::string& password)
{
    if(!_socketConnected)
    {
        return false;
    }

    if(_state != State::Connected)
    {
        return false;
    }

    return sendRegister(_ticketSteamID, name, hashPwd(password));
}

bool HexagonClient::tryLoginToServer(
    const std::string& name, const std::string& password)
{
    if(!_socketConnected)
    {
        return false;
    }

    if(_state != State::Connected)
    {
        return false;
    }

    return sendLogin(_ticketSteamID, name, hashPwd(password));
}

[[nodiscard]] HexagonClient::State HexagonClient::getState() const noexcept
{
    return _state;
}

} // namespace hg
