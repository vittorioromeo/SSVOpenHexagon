// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

// Concurrency test for `hg::HexagonServer`. Opens many client connections
// simultaneously -- far more than the existing load test's 8 -- so that
// `_connectedClients` actually fills up while the server is iterating it.
//
// Each client sends a `CTSPPublicKey` carrying a UNIQUE public key. After
// the storm we stop the server and compare the keys the server stored
// against the ones the clients generated -- that proves the server received
// each client's distinct payload correctly, not just echoed responses.

#include "SSVOpenHexagon/Core/HexagonServer.hpp"
#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "TestUtils.hpp"
#include "sodium/core.h"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/Atomic.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Thread.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StdChrono.hpp"
#include "SFML/Base/Vector.hpp"

#include <latch>
#include <sodium.h>

namespace
{

using Status = sf::Socket::Status;

void connectWithRetry(sf::TcpSocket& socket, const unsigned short port, const std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (socket.connect(sf::IpAddress::LocalHost, port, sf::seconds(0.2f)) == Status::Done)
        {
            return;
        }
        sf::ThisThread::sleepFor(sf::milliseconds(10));
    }
    TEST_ASSERT(false && "connectWithRetry timed out");
}

} // namespace

int main()
{
    TEST_ASSERT_EQ(sodium_init() >= 0, true);

    const ankerl::unordered_dense::set<sf::base::String> emptyWhitelist;

    hg::HexagonServer
        server{nullptr, nullptr, sf::IpAddress::LocalHost, sf::Socket::AnyPort, static_cast<unsigned short>(0), emptyWhitelist};

    const unsigned short port = server.getListenerPort();
    TEST_ASSERT_NE(port, 0);

    sf::Thread serverThread{[&server] { server.run(); }};

    // ------------------------------------------------------------------------
    // Workload: N concurrent clients, each sending a unique public key.
    // 100 stays well below both FD_SETSIZE (1024) and the typical open-file
    // ulimit while forcing `_connectedClients` to grow to 100 simultaneously.
    constexpr int N = 100;

    // Pre-generate unique keypairs so the test owns the expected values.
    sf::base::Vector<hg::SodiumPSKeys> clientKeys;
    clientKeys.reserve(N);
    for (int i = 0; i < N; ++i)
    {
        clientKeys.emplaceBack(hg::generateSodiumPSKeys());
    }

    // Clients are kept alive in this vector so the TCP connections persist
    // until we have queried the server's stored state.
    sf::base::Vector<sf::base::Optional<sf::TcpSocket>> clientSockets;
    clientSockets.resize(N);

    // Every thread counts down once it has opened its TCP connection; nobody
    // proceeds to send until all N are connected. This guarantees that the
    // moment the server processes the first `CTSPPublicKey`, at least N TCP
    // connections have already been established (some may still be queued in
    // the listener's accept backlog, but the kernel has them).
    std::latch allConnected{N};

    sf::Atomic<int> successCount{0};

    sf::base::Vector<sf::Thread> clientThreads;
    clientThreads.reserve(N);

    for (int i = 0; i < N; ++i)
    {
        clientThreads.emplaceBack([&, i]()
        {
            auto sockOpt = sf::TcpSocket::create(/* isBlocking */ true);
            if (!sockOpt.hasValue())
            {
                allConnected.count_down();
                return;
            }

            connectWithRetry(*sockOpt, port, std::chrono::seconds(10));

            allConnected.count_down();
            allConnected.wait();

            // Blocking send + receive: no client-side polling overhead. The
            // overall test `timeout` is the safety net if the server hangs.
            sf::Packet outPacket;
            hg::makeClientToServerPacket(outPacket, hg::CTSPPublicKey{.key = clientKeys[i].keyPublic});

            if (sockOpt->send(outPacket) != Status::Done)
            {
                clientSockets[i] = SFML_BASE_MOVE(sockOpt);
                return;
            }

            // Blocks until the full `STCPPublicKey` arrives -- implies the
            // server has accepted us, added us to `_connectedClients`, and
            // stored our key in `_clientPublicKey`.
            sf::Packet          inPacket;
            sf::OutStringStream errOss;
            if (sockOpt->receive(inPacket) == Status::Done)
            {
                const hg::PVServerToClient decoded = hg::decodeServerToClientPacket(nullptr, errOss, inPacket);
                if (decoded.is<hg::STCPPublicKey>())
                {
                    successCount.fetchAddRelaxed(1);
                }
            }

            // Don't disconnect -- keep the socket alive past thread exit so
            // the server still has us in `_connectedClients` when we query it.
            clientSockets[i] = SFML_BASE_MOVE(sockOpt);
        });
    }

    for (sf::Thread& t : clientThreads)
    {
        t.join();
    }

    // ------------------------------------------------------------------------
    // Every client must have got its round trip back.
    TEST_ASSERT_EQ(successCount.loadRelaxed(), N);

    // ------------------------------------------------------------------------
    // Stop the server so it's safe to read `_connectedClients`, then verify
    // that the server stored every client's unique public key.
    server.stop();
    serverThread.join();

    const sf::base::Vector<hg::SodiumPublicKeyArray> storedKeys = server.getConnectedClientPublicKeys();
    TEST_ASSERT_EQ(storedKeys.size(), static_cast<sf::base::SizeT>(N));

    // Build an unordered set of the stored keys (stringified for hashing).
    ankerl::unordered_dense::set<sf::base::String> storedKeySet;
    storedKeySet.reserve(N);
    for (const hg::SodiumPublicKeyArray& k : storedKeys)
    {
        storedKeySet.emplace(hg::sodiumKeyToString(k));
    }

    // Every client's generated key must be present in the server's view.
    for (const hg::SodiumPSKeys& ck : clientKeys)
    {
        const sf::base::String asStr = hg::sodiumKeyToString(ck.keyPublic);
        TEST_ASSERT(storedKeySet.count(asStr) == 1u);
    }

    // ------------------------------------------------------------------------
    // Close all client sockets (they were kept alive only to preserve server
    // state). Server is already stopped; these just release OS handles.
    for (sf::base::Optional<sf::TcpSocket>& s : clientSockets)
    {
        if (s.hasValue())
        {
            s->disconnect();
        }
    }
}
