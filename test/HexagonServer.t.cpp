// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

// Integration test for `hg::HexagonServer`. Exercises the full server event
// loop end-to-end:
//   * construct server with stub `HGAssets`/`HexagonGame` pointers
//     (replay paths are not exercised, so they can be null)
//   * run the server on a background thread, bound to an ephemeral port
//   * client #1 completes the unencrypted public-key handshake and verifies
//     the server returns its public key
//   * client #2 connects and stalls mid-packet (partial size prefix) -- this
//     would have frozen the old blocking loop; here we must still be able to
//     send a second request on client #1 and get a prompt response.
//   * `server.stop()` unblocks `run()` and the thread joins cleanly.

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
#include "SFML/Base/SizeT.hpp"

#include <chrono>
#include <sodium.h>
#include <thread>
#include <unordered_set>

namespace
{

using Status = sf::Socket::Status;

// Convenience -- the TEST_ASSERT_EQ macro requires an ostreamable type, so we
// compare enum values as ints.
constexpr int toInt(Status s)
{
    return static_cast<int>(s);
}

// Poll-connects to `port` until success or timeout -- the server thread may
// not have finished binding the listener yet when this is called.
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

// Reads exactly one Packet from `socket` (non-blocking receive + retry loop).
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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return Status::NotReady;
}

} // namespace

int main()
{
    // Sodium is used transitively by the protocol helpers (for packet framing)
    // and by the server (for its public/private keypair).
    TEST_ASSERT_EQ(sodium_init() >= 0, true);

    // ------------------------------------------------------------------------
    // Spin up the server on an ephemeral port. Control port: 0 too (whatever).
    // Stub-out assets/game -- this test never sends replay packets.
    const std::unordered_set<sf::base::String> emptyWhitelist;

    hg::HexagonServer server{nullptr /* assets */,
                             nullptr /* hexagonGame */,
                             sf::IpAddress::LocalHost,
                             sf::Socket::AnyPort /* serverPort */,
                             static_cast<unsigned short>(0) /* serverControlPort -- any free port */,
                             emptyWhitelist};

    const unsigned short port = server.getListenerPort();
    TEST_ASSERT_NE(port, 0);

    std::thread serverThread{[&server] { server.run(); }};

    // ------------------------------------------------------------------------
    // Client #1 -- complete a CTSPPublicKey round trip.
    sf::base::Optional<sf::TcpSocket> client1Opt = sf::TcpSocket::create(/* isBlocking */ true);
    TEST_ASSERT(client1Opt.hasValue());
    sf::TcpSocket& client1 = *client1Opt;

    connectWithRetry(client1, port, std::chrono::seconds(2));

    // Generate a throwaway client key pair -- the server just echoes its own
    // public key back in response to CTSPPublicKey (before any RT-key exchange
    // completes), so we don't need to actually decrypt anything.
    const hg::SodiumPSKeys clientKeys = hg::generateSodiumPSKeys();

    sf::Packet outPacket;
    hg::makeClientToServerPacket(outPacket, hg::CTSPPublicKey{.key = clientKeys.keyPublic});
    TEST_ASSERT_EQ(toInt(client1.send(outPacket)), toInt(Status::Done));

    // Expect STCPPublicKey back.
    sf::Packet inPacket;
    TEST_ASSERT_EQ(toInt(receivePacketWithRetry(client1, inPacket, std::chrono::seconds(2))), toInt(Status::Done));

    sf::OutStringStream        errOss;
    const hg::PVServerToClient decoded1 = hg::decodeServerToClientPacket(/* keyReceive */ nullptr, errOss, inPacket);

    TEST_ASSERT(decoded1.is<hg::STCPPublicKey>());

    // ------------------------------------------------------------------------
    // Client #2 -- connects and sends only 3 of the 4 size-prefix bytes, then
    // holds. With the old blocking server this would have frozen the event
    // loop and Client #1's next request would never get a response.
    sf::base::Optional<sf::TcpSocket> client2Opt = sf::TcpSocket::create(/* isBlocking */ true);
    TEST_ASSERT(client2Opt.hasValue());
    sf::TcpSocket& client2 = *client2Opt;

    connectWithRetry(client2, port, std::chrono::seconds(2));

    const unsigned char partialSizePrefix[3] = {0x00, 0x00, 0x00};
    sf::base::SizeT     partialSent          = 0;
    TEST_ASSERT_EQ(toInt(client2.send(partialSizePrefix, 3, partialSent)), toInt(Status::Done));
    TEST_ASSERT_EQ(partialSent, static_cast<sf::base::SizeT>(3));

    // Brief yield so the server actually sees the partial bytes.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // ------------------------------------------------------------------------
    // Client #1 -- send a second request. If the server is alive, we get a
    // response within the timeout. If the old hang bug is present, we don't.
    hg::makeClientToServerPacket(outPacket, hg::CTSPPublicKey{.key = clientKeys.keyPublic});
    TEST_ASSERT_EQ(toInt(client1.send(outPacket)), toInt(Status::Done));

    TEST_ASSERT_EQ(toInt(receivePacketWithRetry(client1, inPacket, std::chrono::seconds(2))), toInt(Status::Done));

    const hg::PVServerToClient decoded2 = hg::decodeServerToClientPacket(/* keyReceive */ nullptr, errOss, inPacket);

    TEST_ASSERT(decoded2.is<hg::STCPPublicKey>());

    // ------------------------------------------------------------------------
    // Shut everything down.
    client1.disconnect();
    client2.disconnect();

    server.stop();
    serverThread.join();
}
