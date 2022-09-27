// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"

#include "SSVOpenHexagon/Utils/Clock.hpp"

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/Packet.hpp>

#include <chrono>
#include <deque>
#include <list>
#include <optional>
#include <sstream>
#include <unordered_set>
#include <variant>
#include <vector>

#include <cstdint>

namespace hg::Steam {
class steam_manager;
}

namespace hg {

struct replay_file;
struct compressed_replay_file;

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
        LoggedIn_Ready = 6,
    };

    // clang-format off
    struct EConnectionSuccess       { };
    struct EConnectionFailure       { std::string error; };
    struct EKicked                  { };
    struct ERegistrationSuccess     { };
    struct ERegistrationFailure     { std::string error;};
    struct ELoginSuccess            { };
    struct ELoginFailure            { std::string error; };
    struct ELogoutSuccess           { };
    struct ELogoutFailure           { };
    struct EDeleteAccountSuccess    { };
    struct EDeleteAccountFailure    { std::string error; };
    struct EReceivedTopScores       { std::string levelValidator; std::vector<Database::ProcessedScore> scores; };
    struct EReceivedOwnScore        { std::string levelValidator; Database::ProcessedScore score; };
    struct EGameVersionMismatch     { };
    struct EProtocolVersionMismatch { };
    // clang-format on

    using Event = std::variant<  //
        EConnectionSuccess,      //
        EConnectionFailure,      //
        EKicked,                 //
        ERegistrationSuccess,    //
        ERegistrationFailure,    //
        ELoginSuccess,           //
        ELoginFailure,           //
        ELogoutSuccess,          //
        ELogoutFailure,          //
        EDeleteAccountSuccess,   //
        EDeleteAccountFailure,   //
        EReceivedTopScores,      //
        EReceivedOwnScore,       //
        EGameVersionMismatch,    //
        EProtocolVersionMismatch //
        >;

private:
    Steam::steam_manager& _steamManager;

    std::optional<std::uint64_t> _ticketSteamID;

    const sf::IpAddress _serverIp;
    const unsigned short _serverPort;

    sf::TcpSocket _socket;
    bool _socketConnected;

    sf::Packet _packetBuffer;
    std::ostringstream _errorOss;

    HRTimePoint _lastHeartbeatTime;

    bool _verbose;

    const SodiumPSKeys _clientPSKeys;
    std::optional<SodiumPublicKeyArray> _serverPublicKey;
    std::optional<SodiumRTKeys> _clientRTKeys;

    State _state;

    std::optional<std::uint64_t> _loginToken;
    std::optional<std::string> _loginName;

    std::deque<Event> _events;

    std::unordered_set<std::string> _levelValidatorsSupportedByServer;

    [[nodiscard]] bool initializeTicketSteamID();
    [[nodiscard]] bool initializeTcpSocket();

    template <typename T>
    [[nodiscard]] bool sendUnencrypted(const T& data);

    template <typename T>
    [[nodiscard]] bool sendEncrypted(const T& data);

    [[nodiscard]] bool sendHeartbeat();
    [[nodiscard]] bool sendDisconnect();
    [[nodiscard]] bool sendPublicKey();
    [[nodiscard]] bool sendRegister(const std::uint64_t steamId,
        const std::string& name, const std::string& passwordHash);
    [[nodiscard]] bool sendLogin(const std::uint64_t steamId,
        const std::string& name, const std::string& passwordHash);
    [[nodiscard]] bool sendLogout(const std::uint64_t steamId);
    [[nodiscard]] bool sendDeleteAccount(
        const std::uint64_t steamId, const std::string& passwordHash);
    [[nodiscard]] bool sendRequestTopScores(
        const std::uint64_t loginToken, const std::string& levelValidator);
    [[nodiscard]] bool sendRequestOwnScore(
        const std::uint64_t loginToken, const std::string& levelValidator);
    [[nodiscard]] bool sendRequestTopScoresAndOwnScore(
        const std::uint64_t loginToken, const std::string& levelValidator);
    [[nodiscard]] bool sendStartedGame(
        const std::uint64_t loginToken, const std::string& levelValidator);
    [[nodiscard]] bool sendCompressedReplay(const std::uint64_t loginToken,
        const std::string& levelValidator,
        const compressed_replay_file& compressedReplayFile);
    [[nodiscard]] bool sendRequestServerStatus(const std::uint64_t loginToken);
    [[nodiscard]] bool sendReady(const std::uint64_t loginToken);

    [[nodiscard]] bool sendPacketRecursive(const int tries, sf::Packet& p);
    [[nodiscard]] bool recvPacketRecursive(const int tries, sf::Packet& p);

    [[nodiscard]] bool sendPacket(sf::Packet& p);
    [[nodiscard]] bool recvPacket(sf::Packet& p);

    bool receiveDataFromServer(sf::Packet& p);

    bool sendHeartbeatIfNecessary();

    void addEvent(const Event& e);

    template <typename... Ts>
    [[nodiscard]] bool fail(const Ts&...);

    [[nodiscard]] bool connectedAndInState(const State s) const noexcept;
    [[nodiscard]] bool connectedAndInAnyState(
        const State s0, const State s1) const noexcept;

public:
    explicit HexagonClient(Steam::steam_manager& steamManager,
        const sf::IpAddress& serverIp, const unsigned short serverPort);

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
    bool tryRequestTopScores(const std::string& levelValidator);
    bool tryRequestOwnScore(const std::string& levelValidator);
    bool tryRequestTopScoresAndOwnScore(const std::string& levelValidator);
    bool trySendStartedGame(const std::string& levelValidator);
    bool trySendCompressedReplay(const std::string& levelValidator,
        const compressed_replay_file& compressedReplayFile);

    [[nodiscard]] State getState() const noexcept;
    [[nodiscard]] bool hasRTKeys() const noexcept;

    [[nodiscard]] const std::optional<std::string>&
    getLoginName() const noexcept;

    [[nodiscard]] std::optional<Event> pollEvent();

    [[nodiscard]] bool isLevelSupportedByServer(
        const std::string& levelValidator) const noexcept;
};

} // namespace hg
