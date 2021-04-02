// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonClient.hpp"

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

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

void HexagonClient::initializeTcpSocket()
{
    _socket.setBlocking(true);

    SSVOH_CLOG << "Connecting socket to server...\n";

    if(_socket.connect(_serverIp, _serverPort, /* timeout */ sf::seconds(2)) !=
        sf::Socket::Status::Done)
    {
        SSVOH_CLOG << "Failure connecting socket to server\n";
        return;
    }

    SSVOH_CLOG << "Socket successfully connected to server\n";

    SSVOH_CLOG << "Sending test packet to server...\n";

    sf::Packet testPacket;
    testPacket << "hello world!";

    if(_socket.send(testPacket) != sf::Socket::Status::Done)
    {
        SSVOH_CLOG << "Failure sending test packet to server\n";
        return;
    }
}

HexagonClient::HexagonClient()
    : _serverIp{Config::getServerIp()},
      _serverPort{Config::getServerPort()}, _socket{}
{
    SSVOH_CLOG << "Initializing client...\n";

    if(_serverIp == sf::IpAddress::None)
    {
        SSVOH_CLOG_ERROR << "Failure initializing server, invalid ip address '"
                         << Config::getServerIp() << "'\n";

        return;
    }

    SSVOH_CLOG << "Client data:\n"
               << SSVOH_CLOG_VAR(_serverIp) << '\n'
               << SSVOH_CLOG_VAR(_serverPort) << '\n';

    initializeTcpSocket();
}

HexagonClient::~HexagonClient()
{
    SSVOH_CLOG << "Uninitializing client...\n";
}

} // namespace hg
