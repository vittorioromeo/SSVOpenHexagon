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

#include <SFML/Network/Packet.hpp>

#include <thread>
#include <chrono>

static auto& clog(const char* funcName)
{
    return ::ssvu::lo(::hg::Utils::concat("hg::HexagonClient::", funcName));
}

#define SSVOH_CLOG ::clog(__func__)
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
    _socket.setBlocking(true);

    SSVOH_CLOG << "Connecting socket to server...\n";

    if(_socket.connect(_serverIp, _serverPort,
           /* timeout */ sf::seconds(0.5)) != sf::Socket::Status::Done)
    {
        SSVOH_CLOG_ERROR << "Failure connecting socket to server\n";

        _socketConnected = false;
        return false;
    }

    _socketConnected = true;
    return true;
}

[[nodiscard]] bool HexagonClient::sendHeartbeat()
{
    if(!_socketConnected)
    {
        return false;
    }

    makePacket(_packetBuffer, PHeartbeat{});

    if(_socket.send(_packetBuffer) != sf::Socket::Status::Done)
    {
        SSVOH_CLOG_ERROR << "Failure sending heartbeat packet to server\n";
        return false;
    }

    _lastHeartbeatTime = Clock::now();
    return true;
}

HexagonClient::HexagonClient(Steam::steam_manager& steamManager)
    : _steamManager{steamManager}, _serverIp{Config::getServerIp()},
      _serverPort{Config::getServerPort()}, _socket{}, _socketConnected{false},
      _packetBuffer{}, _lastHeartbeatTime{}
{
    SSVOH_CLOG << "Initializing client...\n";

    if(_serverIp == sf::IpAddress::None)
    {
        SSVOH_CLOG_ERROR << "Failure initializing client, invalid ip address '"
                         << Config::getServerIp() << "'\n";

        return;
    }

    SSVOH_CLOG << "Client data:\n"
               << SSVOH_CLOG_VAR(_serverIp) << '\n'
               << SSVOH_CLOG_VAR(_serverPort) << '\n';

    if(!initializeTicketSteamID())
    {
        SSVOH_CLOG_ERROR << "Failure initializing client, no ticket Steam ID\n";
        return;
    }

    if(!initializeTcpSocket())
    {
        SSVOH_CLOG_ERROR
            << "Failure initializing client, error initializing TCP socket\n";

        return;
    }

    if(!sendHeartbeat())
    {
        SSVOH_CLOG_ERROR
            << "Failure initializing client, error sending first heartbeat\n";

        return;
    }
}

HexagonClient::~HexagonClient()
{
    SSVOH_CLOG << "Uninitializing client...\n";
    disconnect();
}

void HexagonClient::disconnect()
{
    SSVOH_CLOG << "Disconnecting client...\n";

    _socket.disconnect();
    _socketConnected = false;
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

void HexagonClient::update()
{
    sendHeartbeatIfNecessary();
}

} // namespace hg
