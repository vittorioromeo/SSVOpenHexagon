// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/Sodium.hpp"

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/Packet.hpp>

#include <list>
#include <chrono>
#include <sstream>
#include <optional>

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

    sf::Packet _packetBuffer;
    std::ostringstream _errorOss;

    struct ConnectedClient
    {
        sf::TcpSocket _socket;
        TimePoint _lastActivity;
        int _consecutiveFailures;
        bool _mustDisconnect;
        std::optional<SodiumPublicKeyArray> _clientPublicKey;
        std::optional<SodiumRTKeys> _rsKeys;
        bool _ready;

        explicit ConnectedClient(const TimePoint lastActivity)
            : _socket{}, _lastActivity{lastActivity}, _consecutiveFailures{0},
              _mustDisconnect{false}, _clientPublicKey{}, _ready{false}
        {
        }

        ~ConnectedClient()
        {
            _socket.disconnect();
        }
    };

    std::list<ConnectedClient> _connectedClients;
    using ConnectedClientIterator = std::list<ConnectedClient>::iterator;

    bool _verbose;

    const SodiumPSKeys _serverPSKeys;

    [[nodiscard]] bool initializeTcpListener();
    [[nodiscard]] bool initializeSocketSelector();

    [[nodiscard]] bool sendKick(ConnectedClient& c);
    [[nodiscard]] bool sendPublicKey(ConnectedClient& c);

    void runSocketSelector();
    void runSocketSelector_Iteration();
    bool runSocketSelector_Iteration_TryAcceptingNewClient();
    void runSocketSelector_Iteration_LoopOverSockets();
    void runSocketSelector_Iteration_PurgeClients();

    [[nodiscard]] bool processPacket(ConnectedClient& c, sf::Packet& p);

public:
    explicit HexagonServer(HGAssets& assets, HexagonGame& hexagonGame);
    ~HexagonServer();

    HexagonServer(const HexagonServer&) = delete;
    HexagonServer(HexagonServer&&) = delete;
};

} // namespace hg
