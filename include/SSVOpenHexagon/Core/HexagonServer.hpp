// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/ProtocolVersion.hpp"

#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "SSVOpenHexagon/Utils/Timestamp.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/SocketSelector.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "SFML/System/Atomic.hpp"
#include "SFML/System/IO.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <list>

namespace hg
{

class HGAssets;
class HexagonGame;
struct GameVersion;
struct replay_file;
struct compressed_replay_file;

class HexagonServer
{
private:
    HGAssets*    _assets;      //!< may be `nullptr` in test builds that do not exercise replay handling
    HexagonGame* _hexagonGame; //!< may be `nullptr` in test builds that do not exercise replay handling

    const ankerl::unordered_dense::set<sf::base::String> _supportedLevelValidators;
    const sf::base::Vector<sf::base::String>   _supportedLevelValidatorsVector;

    const sf::IpAddress  _serverIp;
    const unsigned short _serverPort;
    const unsigned short _serverControlPort;

    sf::UdpSocket _controlSocket;

    sf::base::Optional<sf::TcpListener> _listener;
    sf::SocketSelector                  _socketSelector;
    sf::Atomic<bool>                    _running;

    sf::Packet          _packetBuffer;
    sf::OutStringStream _errorOss;

    struct ConnectedClient
    {
        enum class State : sf::base::U8
        {
            Disconnected   = 0,
            Connected      = 1,
            LoggedIn       = 2,
            LoggedIn_Ready = 3,
        };

        sf::TcpSocket                            _socket;
        Utils::SCTimePoint                       _lastActivity;
        int                                      _consecutiveFailures;
        bool                                     _mustDisconnect;
        sf::base::Optional<SodiumPublicKeyArray> _clientPublicKey;
        sf::base::Optional<SodiumRTKeys>         _rtKeys;

        struct LoginData
        {
            sf::base::U32    _userId;
            sf::base::U64    _steamId;
            sf::base::String _name;
            sf::base::String _passwordHash;
            sf::base::U64    _loginToken;
        };

        sf::base::Optional<LoginData> _loginData;

        State _state;

        struct GameStatus
        {
            Utils::SCTimePoint _startTP;
            sf::base::String   _levelValidator;
        };

        sf::base::Optional<GameStatus> _gameStatus;

        // Wall-clock instant of the last `CTSPRequestReplay` we honored
        // for this client. Used to throttle replay requests -- the
        // request handler refuses (and silently drops) any request that
        // arrives within the cooldown window of the previous one. Default
        // value (zero/epoch) means "no recent request".
        Utils::SCTimePoint _lastReplayRequestAt{};

        ConnectedClient(const Utils::SCTimePoint lastActivity, sf::TcpSocket&& socket);
    };

    std::list<ConnectedClient> _connectedClients;
    using ConnectedClientIterator = std::list<ConnectedClient>::iterator;

    bool _verbose;

    const SodiumPSKeys _serverPSKeys;

    Utils::SCTimePoint _lastTokenPurge;
    Utils::SCTimePoint _lastLogsFlush;

    [[nodiscard]] bool initializeControlSocket();
    [[nodiscard]] bool initializeTcpListener();
    [[nodiscard]] bool initializeSocketSelector();

    [[nodiscard]] bool sendPacket(ConnectedClient& c, sf::Packet& p);

    template <typename T>
    [[nodiscard]] bool sendEncrypted(ConnectedClient& c, const T& data);

    [[nodiscard]] bool sendKick(ConnectedClient& c);
    [[nodiscard]] bool sendPublicKey(ConnectedClient& c);
    [[nodiscard]] bool sendRegistrationSuccess(ConnectedClient& c);
    [[nodiscard]] bool sendRegistrationFailure(ConnectedClient& c, const sf::base::String& error);
    [[nodiscard]] bool sendLoginSuccess(ConnectedClient& c, const sf::base::U64 loginToken, const sf::base::String& loginName);
    [[nodiscard]] bool sendLoginFailure(ConnectedClient& c, const sf::base::String& error);
    [[nodiscard]] bool sendLogoutSuccess(ConnectedClient& c);
    [[nodiscard]] bool sendLogoutFailure(ConnectedClient& c);
    [[nodiscard]] bool sendDeleteAccountSuccess(ConnectedClient& c);
    [[nodiscard]] bool sendDeleteAccountFailure(ConnectedClient& c, const sf::base::String& error);
    [[nodiscard]] bool sendTopScores(ConnectedClient&                                  c,
                                     const sf::base::String&                           levelValidator,
                                     const sf::base::Vector<Database::ProcessedScore>& scores);
    [[nodiscard]] bool sendOwnScore(ConnectedClient&                c,
                                    const sf::base::String&         levelValidator,
                                    const Database::ProcessedScore& score);
    [[nodiscard]] bool sendTopScoresAndOwnScore(ConnectedClient&                                    c,
                                                const sf::base::String&                             levelValidator,
                                                const sf::base::Vector<Database::ProcessedScore>&   scores,
                                                const sf::base::Optional<Database::ProcessedScore>& ownScore);
    [[nodiscard]] bool sendServerStatus(ConnectedClient&                          c,
                                        const ProtocolVersion&                    protocolVersion,
                                        const GameVersion&                        gameVersion,
                                        const sf::base::Vector<sf::base::String>& supportedLevelValidators);
    [[nodiscard]] bool sendReplayData(ConnectedClient&        c,
                                      const sf::base::String& levelValidator,
                                      const sf::base::U64     scoreTimestamp,
                                      compressed_replay_file  replay);
    [[nodiscard]] bool sendReplayUnavailable(ConnectedClient&        c,
                                             const sf::base::String& levelValidator,
                                             const sf::base::U64     scoreTimestamp,
                                             const sf::base::String& reason);

    [[nodiscard]] bool kickAndRemoveClient(ConnectedClient& c);

    void runIteration();
    bool runIteration_Control();
    bool runIteration_TryAcceptingNewClient();
    void runIteration_LoopOverSockets();
    void runIteration_PurgeClients();
    void runIteration_PurgeTokens();
    void runIteration_FlushLogs();

    [[nodiscard]] bool validateLogin(ConnectedClient& c, const char* context, const sf::base::U64 ctspLoginToken);

    [[nodiscard]] bool processReplay(ConnectedClient& c, const sf::base::U64 loginToken, const replay_file& rf);

    template <typename T>
    void printCTSPDataVerbose(ConnectedClient& c, const char* title, const T& ctsp);

    [[nodiscard]] bool processPacket(ConnectedClient& c, sf::Packet& p);

    template <typename... Ts>
    [[nodiscard]] bool fail(const Ts&...);

    [[nodiscard]] bool isLevelSupported(const sf::base::String& levelValidator) const;

public:
    explicit HexagonServer(HGAssets*                                   assets,
                           HexagonGame*                                hexagonGame,
                           const sf::IpAddress&                        serverIp,
                           const unsigned short                        serverPort,
                           const unsigned short                        serverControlPort,
                           const ankerl::unordered_dense::set<sf::base::String>& serverLevelWhitelist);

    ~HexagonServer();

    HexagonServer(const HexagonServer&) = delete;
    HexagonServer(HexagonServer&&)      = delete;

    // True iff the constructor's init steps (control socket bind, TCP
    // listener bind, selector setup) all succeeded. The constructor
    // doesn't throw on failure -- it logs and leaves the object in a
    // degenerate state -- so callers must check this before `run()` and
    // surface a non-zero exit code on failure. Without it, a port-bind
    // failure under `systemd` exits 0/SUCCESS and the unit loops the
    // restart-cooldown forever, which is what bit us in production.
    [[nodiscard]] bool initOk() const noexcept;

    // Enters the event loop and returns only when `stop()` is called or the
    // listener is closed (e.g. by SIGINT if the caller installs a handler).
    // No-op if `initOk()` returns false.
    void run();

    // Thread-safe. Requests `run()` to return; resets the listener to unblock
    // a pending `SocketSelector::wait()`.
    void stop();

    // Port actually bound by the listener (useful when the server was created
    // with `sf::Socket::AnyPort`). Returns 0 if the listener failed to init.
    [[nodiscard]] unsigned short getListenerPort() const;

    // Snapshot of the public keys currently stored for connected clients
    // (one entry per `ConnectedClient` whose `CTSPPublicKey` has been
    // processed). Not thread-safe: call only when `run()` is not executing,
    // e.g. after `stop()` + `thread.join()`.
    [[nodiscard]] sf::base::Vector<SodiumPublicKeyArray> getConnectedClientPublicKeys() const;
};

} // namespace hg
