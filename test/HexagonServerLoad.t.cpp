// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

// Load test for `hg::HexagonServer`. Spawns several concurrent clients, each
// hammering the server with a tight request/response loop, and verifies that:
//   * every request gets a matching response (no lost packets, no corruption),
//   * the server stays responsive throughout (no hangs, no crashes),
//   * `stop()` still shuts the loop down cleanly after the storm.
//
// We use the unencrypted `CTSPPublicKey` path because the server always
// replies with an `STCPPublicKey`, which gives us an observable round-trip
// without having to go through the full Sodium RT-key exchange.

#include "SSVOpenHexagon/Core/HexagonServer.hpp"
#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "TestUtils.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/StdChrono.hpp"

#include <atomic>
#include <sodium.h>
#include <thread>
#include <unordered_set>
#include <vector>

namespace
{

using Status = sf::Socket::Status;

constexpr int toInt(Status s)
{
    return static_cast<int>(s);
}

void connectWithRetry(sf::TcpSocket& socket, const unsigned short port, const std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (socket.connect(sf::IpAddress::LocalHost, port, sf::seconds(0.2f)) == Status::Done)
        {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    TEST_ASSERT(false && "connectWithRetry timed out");
}

Status receivePacketWithRetry(sf::TcpSocket& socket, sf::Packet& packet, const std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        const auto s = socket.receive(packet);
        if (s == Status::Done || s == Status::Disconnected || s == Status::Error)
        {
            return s;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return Status::NotReady;
}

// One client worker: connect, perform `messagesPerClient` round trips against
// the server, disconnect. Every successful round trip increments `successes`.
void clientWorker(const unsigned short port, const int messagesPerClient, std::atomic<int>& successes)
{
    auto socketOpt = sf::TcpSocket::create(/* isBlocking */ true);
    if (!socketOpt.hasValue())
    {
        return;
    }

    sf::TcpSocket& socket = *socketOpt;
    connectWithRetry(socket, port, std::chrono::seconds(5));

    // Non-blocking receive so our wait is bounded even if the server stalls.
    socket.setBlocking(false);

    const hg::SodiumPSKeys keys = hg::generateSodiumPSKeys();

    sf::Packet          outPacket;
    sf::Packet          inPacket;
    sf::OutStringStream errOss;

    for (int i = 0; i < messagesPerClient; ++i)
    {
        outPacket.clear();
        hg::makeClientToServerPacket(outPacket, hg::CTSPPublicKey{.key = keys.keyPublic});

        // Sends may return `Partial` under heavy load; loop until Done or fail.
        Status sendStatus = Status::NotReady;
        for (int tries = 0; tries < 5; ++tries)
        {
            sendStatus = socket.send(outPacket);
            if (sendStatus == Status::Done || sendStatus == Status::Error || sendStatus == Status::Disconnected)
            {
                break;
            }
            // Partial or NotReady -- brief yield and retry.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (sendStatus != Status::Done)
        {
            break;
        }

        inPacket.clear();
        if (receivePacketWithRetry(socket, inPacket, std::chrono::seconds(5)) != Status::Done)
        {
            break;
        }

        const hg::PVServerToClient decoded = hg::decodeServerToClientPacket(/* keyReceive */ nullptr, errOss, inPacket);

        if (decoded.is<hg::STCPPublicKey>())
        {
            successes.fetch_add(1, std::memory_order_relaxed);
        }
    }

    socket.disconnect();
}

} // namespace

int main()
{
    TEST_ASSERT_EQ(sodium_init() >= 0, true);

    // ------------------------------------------------------------------------
    // Start the server.
    const std::unordered_set<sf::base::String> emptyWhitelist;

    hg::HexagonServer
        server{nullptr, nullptr, sf::IpAddress::LocalHost, sf::Socket::AnyPort, static_cast<unsigned short>(0), emptyWhitelist};

    const unsigned short port = server.getListenerPort();
    TEST_ASSERT_NE(port, 0);

    std::thread serverThread{[&server] { server.run(); }};

    // ------------------------------------------------------------------------
    // Workload sizing: chosen to push the event loop without blowing past
    // `FD_SETSIZE` and to finish within seconds on a dev machine.
    //   8 clients × 100 messages = 800 full round trips.
    constexpr int nClients          = 8;
    constexpr int messagesPerClient = 100;
    constexpr int expectedSuccesses = nClients * messagesPerClient;

    std::atomic<int> successes{0};

    const auto startTime = std::chrono::steady_clock::now();

    std::vector<std::thread> clientThreads;
    clientThreads.reserve(nClients);
    for (int i = 0; i < nClients; ++i)
    {
        clientThreads.emplace_back(clientWorker, port, messagesPerClient, std::ref(successes));
    }
    for (std::thread& t : clientThreads)
    {
        t.join();
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);

    // ------------------------------------------------------------------------
    // Every request must have produced a response.
    TEST_ASSERT_EQ(successes.load(), expectedSuccesses);

    // ------------------------------------------------------------------------
    // Server must still be alive and responsive after the storm -- exercise it
    // one more time with a single request.
    auto probeOpt = sf::TcpSocket::create(true);
    TEST_ASSERT(probeOpt.hasValue());
    sf::TcpSocket& probe = *probeOpt;

    connectWithRetry(probe, port, std::chrono::seconds(2));
    probe.setBlocking(false);

    const hg::SodiumPSKeys probeKeys = hg::generateSodiumPSKeys();

    sf::Packet outPacket;
    hg::makeClientToServerPacket(outPacket, hg::CTSPPublicKey{.key = probeKeys.keyPublic});
    TEST_ASSERT_EQ(toInt(probe.send(outPacket)), toInt(Status::Done));

    sf::Packet          inPacket;
    sf::OutStringStream errOss;
    TEST_ASSERT_EQ(toInt(receivePacketWithRetry(probe, inPacket, std::chrono::seconds(2))), toInt(Status::Done));

    const hg::PVServerToClient decoded = hg::decodeServerToClientPacket(nullptr, errOss, inPacket);
    TEST_ASSERT(decoded.is<hg::STCPPublicKey>());

    probe.disconnect();

    // ------------------------------------------------------------------------
    // Shut down.
    server.stop();
    serverThread.join();

    // Report for human eyes when run standalone.
    sf::cOut() << "Load test: " << expectedSuccesses << " round trips in " << elapsed.count() << " ms ("
               << (expectedSuccesses * 1000 / (elapsed.count() == 0 ? 1 : elapsed.count())) << " req/s)\n";
}
