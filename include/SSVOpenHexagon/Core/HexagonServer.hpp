// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/UdpSocket.hpp>
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
    const unsigned short _serverControlPort;

    sf::UdpSocket _controlSocket;

    sf::TcpListener _listener;
    sf::SocketSelector _socketSelector;
    bool _running;

    sf::Packet _packetBuffer;
    std::ostringstream _errorOss;

    struct ConnectedClient
    {
        enum class State : std::uint8_t
        {
            Disconnected = 0,
            Connected = 1,
            LoggedIn = 2,
        };

        sf::TcpSocket _socket;
        TimePoint _lastActivity;
        int _consecutiveFailures;
        bool _mustDisconnect;
        std::optional<SodiumPublicKeyArray> _clientPublicKey;
        std::optional<SodiumRTKeys> _rtKeys;

        struct LoginData
        {
            std::uint32_t _userId;
            std::uint64_t _steamId;
            std::string _name;
            std::string _passwordHash;
            std::uint64_t _loginToken;
        };

        std::optional<LoginData> _loginData;

        State _state;

        struct GameStatus
        {
            TimePoint _startTP;
            std::string _levelValidator;
        };

        std::optional<GameStatus> _gameStatus;

        explicit ConnectedClient(const TimePoint lastActivity)
            : _socket{}, _lastActivity{lastActivity}, _consecutiveFailures{0},
              _mustDisconnect{false}, _clientPublicKey{},
              _loginData{}, _state{State::Disconnected}
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

    TimePoint _lastTokenPurge;

    [[nodiscard]] bool initializeControlSocket();
    [[nodiscard]] bool initializeTcpListener();
    [[nodiscard]] bool initializeSocketSelector();

    [[nodiscard]] bool sendPacket(ConnectedClient& c, sf::Packet& p);

    template <typename T>
    [[nodiscard]] bool sendEncrypted(ConnectedClient& c, const T& data);

    [[nodiscard]] bool sendKick(ConnectedClient& c);
    [[nodiscard]] bool sendPublicKey(ConnectedClient& c);
    [[nodiscard]] bool sendRegistrationSuccess(ConnectedClient& c);
    [[nodiscard]] bool sendRegistrationFailure(
        ConnectedClient& c, const std::string& error);
    [[nodiscard]] bool sendLoginSuccess(ConnectedClient& c,
        const std::uint64_t loginToken, const std::string& loginName);
    [[nodiscard]] bool sendLoginFailure(
        ConnectedClient& c, const std::string& error);
    [[nodiscard]] bool sendLogoutSuccess(ConnectedClient& c);
    [[nodiscard]] bool sendLogoutFailure(ConnectedClient& c);
    [[nodiscard]] bool sendDeleteAccountSuccess(ConnectedClient& c);
    [[nodiscard]] bool sendDeleteAccountFailure(
        ConnectedClient& c, const std::string& error);
    [[nodiscard]] bool sendTopScores(ConnectedClient& c,
        const std::string& levelValidator,
        const std::vector<Database::ProcessedScore>& scores);
    [[nodiscard]] bool sendOwnScore(ConnectedClient& c,
        const std::string& levelValidator,
        const Database::ProcessedScore& score);
    [[nodiscard]] bool sendTopScoresAndOwnScore(ConnectedClient& c,
        const std::string& levelValidator,
        const std::vector<Database::ProcessedScore>& scores,
        const std::optional<Database::ProcessedScore>& ownScore);

    void kickAndRemoveClient(ConnectedClient& c);

    void run();
    void runIteration();
    bool runIteration_Control();
    bool runIteration_TryAcceptingNewClient();
    void runIteration_LoopOverSockets();
    void runIteration_PurgeClients();
    void runIteration_PurgeTokens();

    [[nodiscard]] bool processPacket(ConnectedClient& c, sf::Packet& p);

    template <typename... Ts>
    [[nodiscard]] bool fail(const Ts&...);

public:
    explicit HexagonServer(HGAssets& assets, HexagonGame& hexagonGame);
    ~HexagonServer();

    HexagonServer(const HexagonServer&) = delete;
    HexagonServer(HexagonServer&&) = delete;
};

} // namespace hg
