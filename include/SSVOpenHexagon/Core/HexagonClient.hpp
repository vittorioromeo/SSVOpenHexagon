// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/Sodium.hpp"

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/Packet.hpp>

#include <list>
#include <cstdint>
#include <chrono>
#include <sstream>
#include <optional>

namespace hg::Steam
{

class steam_manager;

}

namespace hg
{

class HexagonClient
{
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    Steam::steam_manager& _steamManager;

    std::uint64_t _ticketSteamID;

    const sf::IpAddress _serverIp;
    const unsigned short _serverPort;

    sf::TcpSocket _socket;
    bool _socketConnected;

    sf::Packet _packetBuffer;
    std::ostringstream _errorOss;

    TimePoint _lastHeartbeatTime;

    bool _verbose;

    const SodiumPSKeys _clientPSKeys;
    std::optional<SodiumPublicKeyArray> _serverPublicKey;
    std::optional<SodiumRTKeys> _clientRTKeys;

    [[nodiscard]] bool initializeTicketSteamID();
    [[nodiscard]] bool initializeTcpSocket();

    template <typename T>
    [[nodiscard]] bool sendEncrypted(const T& data);

    [[nodiscard]] bool sendHeartbeat();
    [[nodiscard]] bool sendPublicKey();
    [[nodiscard]] bool sendReady();
    [[nodiscard]] bool sendPrint(const std::string& s);
    [[nodiscard]] bool sendRegister(
        const std::uint64_t steamId, const std::string& name);
    [[nodiscard]] bool sendLogin(
        const std::uint64_t steamId, const std::string& name);

    [[nodiscard]] bool sendPacketRecursive(const int tries, sf::Packet& p);
    [[nodiscard]] bool recvPacketRecursive(const int tries, sf::Packet& p);

    [[nodiscard]] bool sendPacket(sf::Packet& p);
    [[nodiscard]] bool recvPacket(sf::Packet& p);

    bool receiveDataFromServer(sf::Packet& p);

    void disconnect();

    bool sendHeartbeatIfNecessary();

    bool connect();

public:
    explicit HexagonClient(Steam::steam_manager& steamManager);
    ~HexagonClient();

    HexagonClient(const HexagonClient&) = delete;
    HexagonClient(HexagonClient&&) = delete;

    void update();
};

} // namespace hg
