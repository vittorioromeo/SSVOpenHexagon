// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

// Standalone smoke-test client for `HexagonServer`.
//
// Walks the unencrypted public-key handshake against a running server:
//   1. Open a TCP connection to <host>:<port> (defaults: 127.0.0.1:50505).
//   2. Send a `CTSPPublicKey` carrying a freshly-generated client public key.
//   3. Receive a packet, decode it, assert the variant holds `STCPPublicKey`.
//   4. Print the decoded server public key on success.
//
// Exit codes:
//   0 — round-trip succeeded
//   1 — protocol-level failure (connect/send/receive/decode)
//   2 — pre-flight failure (sodium init, TcpSocket::create, bad args)
//
// Used by `scripts/smoke-test-server.sh` to verify that the server's
// network path actually round-trips a real packet, not just that it
// accepts and closes raw TCP connections.

#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Optional.hpp"

#include <sodium.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{

constexpr unsigned short kDefaultPort = 50'505;

void usage(const char* argv0)
{
    std::fprintf(stderr,
                 "Usage: %s [host] [port]\n"
                 "       host defaults to 127.0.0.1, port to %u\n",
                 argv0,
                 static_cast<unsigned>(kDefaultPort));
}

[[nodiscard]] int parsePort(const char* s, unsigned short& out)
{
    char*           end  = nullptr;
    const long long val  = std::strtoll(s, &end, 10);
    if (end == s || *end != '\0' || val < 1 || val > 65'535)
    {
        return 1;
    }
    out = static_cast<unsigned short>(val);
    return 0;
}

} // namespace

int main(int argc, char* argv[])
{
    const char*    host = "127.0.0.1";
    unsigned short port = kDefaultPort;

    if (argc >= 2)
    {
        if (std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0)
        {
            usage(argv[0]);
            return 0;
        }
        host = argv[1];
    }
    if (argc >= 3)
    {
        if (parsePort(argv[2], port) != 0)
        {
            std::fprintf(stderr, "[smoke-client] invalid port: '%s'\n", argv[2]);
            usage(argv[0]);
            return 2;
        }
    }
    if (argc > 3)
    {
        usage(argv[0]);
        return 2;
    }

    if (sodium_init() < 0)
    {
        std::fprintf(stderr, "[smoke-client] sodium_init failed\n");
        return 2;
    }

    const sf::base::Optional<sf::IpAddress> serverIp = sf::IpAddressUtils::resolve(host);
    if (!serverIp.hasValue())
    {
        std::fprintf(stderr, "[smoke-client] could not resolve host '%s'\n", host);
        return 2;
    }

    sf::base::Optional<sf::TcpSocket> sockOpt = sf::TcpSocket::create(/* isBlocking */ true);
    if (!sockOpt.hasValue())
    {
        std::fprintf(stderr, "[smoke-client] TcpSocket::create failed\n");
        return 2;
    }
    sf::TcpSocket& sock = *sockOpt;

    if (sock.connect(*serverIp, port, sf::seconds(2.f)) != sf::Socket::Status::Done)
    {
        std::fprintf(stderr, "[smoke-client] connect to %s:%u failed\n", host, static_cast<unsigned>(port));
        return 1;
    }
    std::fprintf(stdout, "[smoke-client] connected to %s:%u\n", host, static_cast<unsigned>(port));

    // Generate a throwaway client keypair just so the server has something
    // to store as our `_clientPublicKey`. We don't need the private half
    // for this handshake — the server replies with its OWN public key
    // unencrypted, no further crypto from our side.
    const hg::SodiumPSKeys clientKeys = hg::generateSodiumPSKeys();

    sf::Packet outPacket;
    hg::makeClientToServerPacket(outPacket, hg::CTSPPublicKey{.key = clientKeys.keyPublic});

    if (sock.send(outPacket) != sf::Socket::Status::Done)
    {
        std::fprintf(stderr, "[smoke-client] sending CTSPPublicKey failed\n");
        return 1;
    }
    std::fprintf(stdout, "[smoke-client] sent CTSPPublicKey\n");

    sf::Packet inPacket;
    if (sock.receive(inPacket) != sf::Socket::Status::Done)
    {
        std::fprintf(stderr, "[smoke-client] receiving server reply failed\n");
        return 1;
    }
    std::fprintf(stdout,
                 "[smoke-client] received reply (%zu bytes)\n",
                 static_cast<std::size_t>(inPacket.getDataSize()));

    sf::OutStringStream        errOss;
    const hg::PVServerToClient decoded = hg::decodeServerToClientPacket(/* keyReceive */ nullptr, errOss, inPacket);

    const hg::STCPPublicKey* spk = decoded.getIf<hg::STCPPublicKey>();
    if (spk == nullptr)
    {
        std::fprintf(stderr,
                     "[smoke-client] expected STCPPublicKey, got something else (decode error: '%s')\n",
                     errOss.getString().c_str());
        return 1;
    }

    std::fprintf(stdout, "[smoke-client] decoded STCPPublicKey, server pub key: '%s'\n", hg::sodiumKeyToString(spk->key).cStr());

    sock.disconnect();
    std::fprintf(stdout, "[smoke-client] OK\n");
    return 0;
}
