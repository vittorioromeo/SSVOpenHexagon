// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

// Regression test for the server-hang bug. A blocking TcpSocket calling
// `receive(Packet&)` after a peer sends only part of the 4-byte size prefix
// will block forever waiting for the rest of the prefix. Switching the
// accepted socket to non-blocking makes `receive(Packet&)` return `NotReady`
// in that situation, leaving the partial state buffered inside `TcpSocket`
// so a later call resumes without data loss.
//
// HexagonServer relies on this behavior to avoid freezing its single-threaded
// event loop when any one client stalls mid-packet.

#include "TestUtils.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/SocketSelector.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"

int main()
{
    // Listener on an ephemeral port; blocking is fine here, we only call
    // accept() once after a connection we initiate ourselves.
    auto listenerOpt = sf::TcpListener::create(sf::Socket::AnyPort, /* isBlocking */ true);
    TEST_ASSERT(listenerOpt.hasValue());

    sf::TcpListener&     listener = *listenerOpt;
    const unsigned short port     = listener.getLocalPort();
    TEST_ASSERT_NE(port, 0);

    // Client connects to the listener.
    auto clientOpt = sf::TcpSocket::create(/* isBlocking */ true);
    TEST_ASSERT(clientOpt.hasValue());

    sf::TcpSocket& client = *clientOpt;
    TEST_ASSERT_EQ(static_cast<int>(client.connect(sf::IpAddress::LocalHost, port, sf::seconds(1))),
                   static_cast<int>(sf::Socket::Status::Done));

    // Server accepts the connection and switches to non-blocking mode -- this
    // is the crucial step that HexagonServer now performs.
    auto acceptResult = listener.accept();
    TEST_ASSERT_EQ(static_cast<int>(acceptResult.status), static_cast<int>(sf::Socket::Status::Done));
    TEST_ASSERT(acceptResult.socket.hasValue());

    sf::TcpSocket& server = *acceptResult.socket;
    server.setBlocking(false);

    // Wire format of `TcpSocket::send(Packet&)` is:
    //   [u32 payload size, host (little-endian) byte order][payload bytes]
    // We send these bytes ourselves so we control exactly where the split
    // happens. 8-byte payload → size prefix little-endian is {0x08, 0x00, 0x00, 0x00}.
    const unsigned char   payload[8]    = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
    const sf::base::SizeT payloadSize   = sizeof(payload);
    const unsigned char   sizePrefix[4] = {static_cast<unsigned char>(payloadSize), 0x00, 0x00, 0x00};

    // Step 1: client sends the first 3 of 4 size-prefix bytes.
    {
        sf::base::SizeT sent = 0;
        TEST_ASSERT_EQ(static_cast<int>(client.send(sizePrefix, 3, sent)), static_cast<int>(sf::Socket::Status::Done));
        TEST_ASSERT_EQ(sent, static_cast<sf::base::SizeT>(3));
    }

    // Wait for those bytes to be visible at the server side.
    sf::SocketSelector selector;
    TEST_ASSERT(selector.add(server));
    TEST_ASSERT(selector.wait(sf::seconds(2)));
    TEST_ASSERT(selector.isReady(server));

    // Step 2: on a BLOCKING socket, this receive would hang forever waiting
    // for the 4th size-prefix byte. On a non-blocking socket it must return
    // NotReady with the 3 bytes preserved in TcpSocket::m_pendingPacket.
    sf::Packet packet;
    TEST_ASSERT_EQ(static_cast<int>(server.receive(packet)), static_cast<int>(sf::Socket::Status::NotReady));

    // Step 3: client sends the remaining size-prefix byte plus the payload.
    {
        sf::base::SizeT sent = 0;
        TEST_ASSERT_EQ(static_cast<int>(client.send(&sizePrefix[3], 1, sent)), static_cast<int>(sf::Socket::Status::Done));
        TEST_ASSERT_EQ(sent, static_cast<sf::base::SizeT>(1));

        sent = 0;
        TEST_ASSERT_EQ(static_cast<int>(client.send(payload, payloadSize, sent)),
                       static_cast<int>(sf::Socket::Status::Done));
        TEST_ASSERT_EQ(sent, payloadSize);
    }

    // Step 4: loop until the packet is fully reassembled. TCP does not
    // guarantee atomicity across step 3's two `send()` calls; the selector
    // fires on "any data readable", not "all expected data readable", so on
    // a busy host the 1-byte prefix completion and the 8-byte payload can
    // arrive in separate segments. `TcpSocket::m_pendingPacket` preserves
    // partial state across `NotReady` returns, so we just keep polling --
    // the same pattern HexagonServer's event loop uses. A 2-second
    // `selector.wait` failure here is a real timeout (no data arrived),
    // not a flake.
    sf::Socket::Status status = sf::Socket::Status::NotReady;
    while (status != sf::Socket::Status::Done)
    {
        TEST_ASSERT(selector.wait(sf::seconds(2)));
        TEST_ASSERT(selector.isReady(server));

        status = server.receive(packet);
        TEST_ASSERT(status == sf::Socket::Status::Done || status == sf::Socket::Status::NotReady);
    }
    TEST_ASSERT_EQ(packet.getDataSize(), payloadSize);

    const unsigned char* received = static_cast<const unsigned char*>(packet.getData());
    for (sf::base::SizeT i = 0; i < payloadSize; ++i)
    {
        TEST_ASSERT_EQ(received[i], payload[i]);
    }

    client.disconnect();
    server.disconnect();
}
