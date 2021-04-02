// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/HexagonServer.hpp"

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Utils/Concat.hpp"

#include <SFML/Network.hpp>

#include <thread>
#include <chrono>

static auto& slog(const char* funcName)
{
    return ::ssvu::lo(::hg::Utils::concat("hg::HexagonServer::", funcName));
}

#define SSVOH_SLOG ::slog(__func__)
#define SSVOH_SLOG_ERROR ::slog(__func__) << "[ERROR] "
#define SSVOH_SLOG_VAR(x) '\'' << #x << "': '" << x << '\''

namespace hg
{

// TODO:
void HexagonServer::initializeTcpListener()
{
    SSVOH_SLOG << "Initializing TCP listener...\n";

    _listener.setBlocking(true);
    if(_listener.listen(_serverPort) == sf::TcpListener::Status::Error)
    {
        SSVOH_SLOG_ERROR << "Failure initializing TCP listener\n";
        return;
    }

    _socketSelector.add(_listener);

    while(_running)
    {
        SSVOH_SLOG << "Waiting for clients...\n";

        if(!_socketSelector.wait(/* TODO: timeout? */))
        {
            continue;
        }

        // Test the listener
        if(_socketSelector.isReady(_listener))
        {
            SSVOH_SLOG << "Listener is ready\n";

            ConnectedClient& potentialClient =
                _connectedClients.emplace_back(Clock::now());

            sf::TcpSocket& potentialSocket = potentialClient._socket;
            potentialSocket.setBlocking(true);

            const void* potentialClientAddress =
                static_cast<void*>(&potentialClient);

            // The listener is ready: there is a pending connection
            if(_listener.accept(potentialSocket) == sf::Socket::Done)
            {
                SSVOH_SLOG << "Listener accepted new client '"
                           << potentialClientAddress << "'\n";

                // Add the new client to the selector so that we will
                // be notified when he sends something
                _socketSelector.add(potentialSocket);
            }
            else
            {
                SSVOH_SLOG << "Listener failed to accept new client '"
                           << potentialClientAddress << "'\n";

                // Error, we won't get a new connection, delete the socket
                _connectedClients.pop_back();
            }

            continue;
        }

        SSVOH_SLOG << "Listener is not ready, checking sockets\n";

        for(auto it = _connectedClients.begin(); it != _connectedClients.end();
            ++it)
        {
            ConnectedClient& connectedClient = *it;
            const void* clientAddress = static_cast<void*>(&connectedClient);
            sf::TcpSocket& clientSocket = connectedClient._socket;

            if(!_socketSelector.isReady(clientSocket))
            {
                continue;
            }

            SSVOH_SLOG << "Client '" << clientAddress << "' has sent data\n ";

            // The client has sent some data, we can receive it
            sf::Packet packet;
            if(clientSocket.receive(packet) == sf::Socket::Done)
            {
                SSVOH_SLOG << "Successfully received data from client '"
                           << clientAddress << "'\n";

                connectedClient._lastActivity = Clock::now();
                connectedClient._consecutiveFailures = 0;

                if(std::string s; packet >> s)
                {
                    // ok, x was extracted successfully
                    SSVOH_SLOG << "Extracted string from client '"
                               << clientAddress << "': '" << s << "'\n";
                }
                else
                {
                    SSVOH_SLOG << "Failed Extracting string from client '"
                               << clientAddress << "'\n";
                }

                continue;
            }

            // Failed to receive data
            SSVOH_SLOG << "Failed to receive data from client '"
                       << clientAddress << "' (consecutive failures: "
                       << connectedClient._consecutiveFailures << ")\n";

            ++connectedClient._consecutiveFailures;

            constexpr int maxConsecutiveFailures = 5;
            if(connectedClient._consecutiveFailures == maxConsecutiveFailures)
            {
                SSVOH_SLOG << "Too many consecutive failures for client '"
                           << clientAddress << "', removing from list\n";

                _socketSelector.remove(connectedClient._socket);
                it = _connectedClients.erase(it);
            }
        }
    }
}

HexagonServer::HexagonServer(HGAssets& assets, HexagonGame& hexagonGame)
    : _assets{assets}, _hexagonGame{hexagonGame},
      _serverIp{Config::getServerIp()}, _serverPort{Config::getServerPort()},
      _listener{}, _socketSelector{}, _running{true}
{
    SSVOH_SLOG << "Initializing server...\n";

    if(_serverIp == sf::IpAddress::None)
    {
        SSVOH_SLOG_ERROR << "Failure initializing server, invalid ip address '"
                         << Config::getServerIp() << "'\n";

        return;
    }

    SSVOH_SLOG << "Server data:\n"
               << SSVOH_SLOG_VAR(_serverIp) << '\n'
               << SSVOH_SLOG_VAR(_serverPort) << '\n';

    initializeTcpListener();
}

HexagonServer::~HexagonServer()
{
    SSVOH_SLOG << "Uninitializing server...\n";
}

} // namespace hg
