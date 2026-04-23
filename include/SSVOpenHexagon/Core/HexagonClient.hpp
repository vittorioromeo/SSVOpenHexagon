// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/StringHash.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"

#include <deque>
#include <unordered_set>

namespace hg::Steam
{
class steam_manager;
}

namespace hg
{

struct replay_file;
struct compressed_replay_file;

class HexagonClient
{
public:
    enum class State : sf::base::U8
    {
        Disconnected    = 0,
        InitError       = 1,
        Connecting      = 2,
        ConnectionError = 3,
        Connected       = 4,
        LoggedIn        = 5,
        LoggedIn_Ready  = 6,
    };

    // clang-format off
    struct EConnectionSuccess       { };
    struct EConnectionFailure       { sf::base::String error; };
    struct EKicked                  { };
    struct ERegistrationSuccess     { };
    struct ERegistrationFailure     { sf::base::String error;};
    struct ELoginSuccess            { };
    struct ELoginFailure            { sf::base::String error; };
    struct ELogoutSuccess           { };
    struct ELogoutFailure           { };
    struct EDeleteAccountSuccess    { };
    struct EDeleteAccountFailure    { sf::base::String error; };
    struct EReceivedTopScores       { sf::base::String levelValidator; sf::base::Vector<Database::ProcessedScore> scores; };
    struct EReceivedOwnScore        { sf::base::String levelValidator; Database::ProcessedScore score; };
    struct EGameVersionMismatch     { };
    struct EProtocolVersionMismatch { };
    // clang-format on

    using Event = sf::base::Variant< //
        EConnectionSuccess,          //
        EConnectionFailure,          //
        EKicked,                     //
        ERegistrationSuccess,        //
        ERegistrationFailure,        //
        ELoginSuccess,               //
        ELoginFailure,               //
        ELogoutSuccess,              //
        ELogoutFailure,              //
        EDeleteAccountSuccess,       //
        EDeleteAccountFailure,       //
        EReceivedTopScores,          //
        EReceivedOwnScore,           //
        EGameVersionMismatch,        //
        EProtocolVersionMismatch     //
        >;

private:
    Steam::steam_manager& _steamManager;

    sf::base::Optional<sf::base::U64> _ticketSteamID;

    const sf::IpAddress  _serverIp;
    const unsigned short _serverPort;

    sf::base::Optional<sf::TcpSocket> _socket;

    sf::Packet          _packetBuffer;
    sf::OutStringStream _errorOss;

    HRTimePoint _lastHeartbeatTime;

    bool _verbose;

    const SodiumPSKeys                       _clientPSKeys;
    sf::base::Optional<SodiumPublicKeyArray> _serverPublicKey;
    sf::base::Optional<SodiumRTKeys>         _clientRTKeys;

    State _state;

    sf::base::Optional<sf::base::U64>    _loginToken;
    sf::base::Optional<sf::base::String> _loginName;

    std::deque<Event> _events;

    std::unordered_set<sf::base::String> _levelValidatorsSupportedByServer;

    [[nodiscard]] bool initializeTicketSteamID();
    [[nodiscard]] bool initializeTcpSocket();

    template <typename T>
    [[nodiscard]] bool sendUnencrypted(const T& data);

    template <typename T>
    [[nodiscard]] bool sendEncrypted(const T& data);

    [[nodiscard]] bool sendHeartbeat();
    [[nodiscard]] bool sendDisconnect();
    [[nodiscard]] bool sendPublicKey();
    [[nodiscard]] bool sendRegister(const sf::base::U64     steamId,
                                    const sf::base::String& name,
                                    const sf::base::String& passwordHash);
    [[nodiscard]] bool sendLogin(const sf::base::U64 steamId, const sf::base::String& name, const sf::base::String& passwordHash);
    [[nodiscard]] bool sendLogout(const sf::base::U64 steamId);
    [[nodiscard]] bool sendDeleteAccount(const sf::base::U64 steamId, const sf::base::String& passwordHash);
    [[nodiscard]] bool sendRequestTopScores(const sf::base::U64 loginToken, const sf::base::String& levelValidator);
    [[nodiscard]] bool sendRequestOwnScore(const sf::base::U64 loginToken, const sf::base::String& levelValidator);
    [[nodiscard]] bool sendRequestTopScoresAndOwnScore(const sf::base::U64 loginToken, const sf::base::String& levelValidator);
    [[nodiscard]] bool sendStartedGame(const sf::base::U64 loginToken, const sf::base::String& levelValidator);
    [[nodiscard]] bool sendCompressedReplay(const sf::base::U64           loginToken,
                                            const sf::base::String&       levelValidator,
                                            const compressed_replay_file& compressedReplayFile);
    [[nodiscard]] bool sendRequestServerStatus(const sf::base::U64 loginToken);
    [[nodiscard]] bool sendReady(const sf::base::U64 loginToken);

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
    [[nodiscard]] bool connectedAndInAnyState(const State s0, const State s1) const noexcept;

public:
    explicit HexagonClient(Steam::steam_manager& steamManager, const sf::IpAddress& serverIp, const unsigned short serverPort);

    ~HexagonClient();

    HexagonClient(const HexagonClient&) = delete;
    HexagonClient(HexagonClient&&)      = delete;

    bool connect();
    void disconnect();

    void update();

    bool tryRegister(const sf::base::String& name, const sf::base::String& password);
    bool tryLogin(const sf::base::String& name, const sf::base::String& password);
    bool tryLogoutFromServer();
    bool tryDeleteAccount(const sf::base::String& password);
    bool tryRequestTopScores(const sf::base::String& levelValidator);
    bool tryRequestOwnScore(const sf::base::String& levelValidator);
    bool tryRequestTopScoresAndOwnScore(const sf::base::String& levelValidator);
    bool trySendStartedGame(const sf::base::String& levelValidator);
    bool trySendCompressedReplay(const sf::base::String& levelValidator, const compressed_replay_file& compressedReplayFile);

    [[nodiscard]] State getState() const noexcept;
    [[nodiscard]] bool  hasRTKeys() const noexcept;

    [[nodiscard]] const sf::base::Optional<sf::base::String>& getLoginName() const noexcept;

    [[nodiscard]] sf::base::Optional<Event> pollEvent();

    [[nodiscard]] bool isLevelSupportedByServer(const sf::base::String& levelValidator) const noexcept;
};

} // namespace hg
