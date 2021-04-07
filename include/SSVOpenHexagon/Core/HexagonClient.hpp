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
#include <vector>
#include <variant>
#include <deque>

namespace hg::Steam
{

class steam_manager;

}

namespace hg
{

class HexagonClient
{
public:
    enum class State : std::uint8_t
    {
        Disconnected = 0,
        InitError = 1,
        Connecting = 2,
        ConnectionError = 3,
        Connected = 4,
        LoggedIn = 5,
    };

    // clang-format off
    struct EConnectionSuccess    { };
    struct EConnectionFailure    { std::string error; };
    struct EKicked               { };
    struct ERegistrationSuccess  { };
    struct ERegistrationFailure  { std::string error;};
    struct ELoginSuccess         { };
    struct ELoginFailure         { std::string error; };
    struct ELogoutSuccess        { };
    struct ELogoutFailure        { };
    struct EDeleteAccountSuccess { };
    struct EDeleteAccountFailure { std::string error; };
    // clang-format on

    using Event = std::variant< //
        EConnectionSuccess,     //
        EConnectionFailure,     //
        EKicked,                //
        ERegistrationSuccess,   //
        ERegistrationFailure,   //
        ELoginSuccess,          //
        ELoginFailure,          //
        ELogoutSuccess,         //
        ELogoutFailure,         //
        EDeleteAccountSuccess,  //
        EDeleteAccountFailure   //
        >;

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    Steam::steam_manager& _steamManager;

    std::optional<std::uint64_t> _ticketSteamID;

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

    State _state;

    std::optional<std::uint64_t> _loginToken;
    std::optional<std::string> _loginName;

    std::deque<Event> _events;

    [[nodiscard]] bool initializeTicketSteamID();
    [[nodiscard]] bool initializeTcpSocket();

    template <typename T>
    [[nodiscard]] bool sendEncrypted(const T& data);

    [[nodiscard]] bool sendHeartbeat();
    [[nodiscard]] bool sendPublicKey();
    [[nodiscard]] bool sendReady();
    [[nodiscard]] bool sendPrint(const std::string& s);
    [[nodiscard]] bool sendRegister(const std::uint64_t steamId,
        const std::string& name, const std::string& passwordHash);
    [[nodiscard]] bool sendLogin(const std::uint64_t steamId,
        const std::string& name, const std::string& passwordHash);
    [[nodiscard]] bool sendLogout(const std::uint64_t steamId);
    [[nodiscard]] bool sendDeleteAccount(
        const std::uint64_t steamId, const std::string& passwordHash);

    [[nodiscard]] bool sendPacketRecursive(const int tries, sf::Packet& p);
    [[nodiscard]] bool recvPacketRecursive(const int tries, sf::Packet& p);

    [[nodiscard]] bool sendPacket(sf::Packet& p);
    [[nodiscard]] bool recvPacket(sf::Packet& p);

    bool receiveDataFromServer(sf::Packet& p);

    bool sendHeartbeatIfNecessary();

    void addEvent(const Event& e);

public:
    explicit HexagonClient(Steam::steam_manager& steamManager);
    ~HexagonClient();

    HexagonClient(const HexagonClient&) = delete;
    HexagonClient(HexagonClient&&) = delete;

    bool connect();
    void disconnect();

    void update();

    bool tryRegister(const std::string& name, const std::string& password);

    bool tryLogin(const std::string& name, const std::string& password);
    bool tryLogoutFromServer();
    bool tryDeleteAccount(const std::string& password);

    [[nodiscard]] State getState() const noexcept;

    [[nodiscard]] const std::optional<std::string>&
    getLoginName() const noexcept;

    [[nodiscard]] std::optional<Event> pollEvent();
};

} // namespace hg
