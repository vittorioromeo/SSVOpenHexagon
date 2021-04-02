// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/SocketSelector.hpp>

#include <list>
#include <chrono>

namespace hg
{

class HGAssets;
class HexagonGame;

class HexagonServer
{
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    HGAssets& _assets;
    HexagonGame& _hexagonGame;

    const sf::IpAddress _serverIp;
    const unsigned short _serverPort;

    sf::TcpListener _listener;
    sf::SocketSelector _socketSelector;
    bool _running;

    struct ConnectedClient
    {
        sf::TcpSocket _socket;
        TimePoint _lastActivity;
        int _consecutiveFailures;

        explicit ConnectedClient(const TimePoint lastActivity)
            : _lastActivity{lastActivity}, _consecutiveFailures{0}
        {
        }
    };

    std::list<ConnectedClient> _connectedClients;

    void initializeTcpListener();

public:
    explicit HexagonServer(HGAssets& assets, HexagonGame& hexagonGame);
    ~HexagonServer();

    HexagonServer(const HexagonServer&) = delete;
    HexagonServer(HexagonServer&&) = delete;
};

} // namespace hg
