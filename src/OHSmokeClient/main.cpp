// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

// Standalone smoke-test client for `HexagonServer`.
//
// Walks the unencrypted public-key handshake against a running server:
//   1. Open one TCP connection to <host>:<port> (defaults: 127.0.0.1:50505).
//   2. Repeat `--count N` times (default N=1) on that same connection:
//        - Send a `CTSPPublicKey` carrying a freshly-generated client key.
//        - Receive a packet, decode it, assert the variant holds
//          `STCPPublicKey`.
//   3. Print a final summary line and exit.
//
// Exit codes:
//   0 -- every round-trip succeeded
//   1 -- protocol-level failure (connect/send/receive/decode) on any
//       round-trip
//   2 -- pre-flight failure (sodium init, TcpSocket::create, bad args)
//
// Used by `scripts/smoke-test-server.sh` both as a single-shot
// correctness check and (with `--count`, multiple processes spawned
// in parallel) as a load probe that the server stays responsive
// under concurrent traffic from real OS processes.

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
                 "Usage: %s [--count N] [host] [port]\n"
                 "       host defaults to 127.0.0.1, port to %u, count to 1\n"
                 "       --count N performs N round-trips on a single TCP connection\n",
                 argv0,
                 static_cast<unsigned>(kDefaultPort));
}

[[nodiscard]] int parsePort(const char* s, unsigned short& out)
{
    char*           end = nullptr;
    const long long val = std::strtoll(s, &end, 10);
    if (end == s || *end != '\0' || val < 1 || val > 65'535)
    {
        return 1;
    }
    out = static_cast<unsigned short>(val);
    return 0;
}

[[nodiscard]] int parseCount(const char* s, int& out)
{
    char*           end = nullptr;
    const long long val = std::strtoll(s, &end, 10);
    if (end == s || *end != '\0' || val < 1 || val > 1'000'000)
    {
        return 1;
    }
    out = static_cast<int>(val);
    return 0;
}

} // namespace

int main(int argc, char* argv[])
{
    const char*    host  = "127.0.0.1";
    unsigned short port  = kDefaultPort;
    int            count = 1;

    // Hand-rolled arg parsing: optional `--count N` anywhere, followed by
    // up to two positional args [host] [port]. Keeps things dependency-free.
    int positional = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0)
        {
            usage(argv[0]);
            return 0;
        }
        if (std::strcmp(argv[i], "--count") == 0)
        {
            if (i + 1 >= argc || parseCount(argv[i + 1], count) != 0)
            {
                std::fprintf(stderr,
                             "[smoke-client] --count requires a positive integer (got '%s')\n",
                             (i + 1 < argc) ? argv[i + 1] : "");
                usage(argv[0]);
                return 2;
            }
            ++i;
            continue;
        }
        if (positional == 0)
        {
            host = argv[i];
            ++positional;
            continue;
        }
        if (positional == 1)
        {
            if (parsePort(argv[i], port) != 0)
            {
                std::fprintf(stderr, "[smoke-client] invalid port: '%s'\n", argv[i]);
                usage(argv[0]);
                return 2;
            }
            ++positional;
            continue;
        }
        std::fprintf(stderr, "[smoke-client] unexpected argument: '%s'\n", argv[i]);
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

    // Reuse a single keypair across all round-trips on this connection --
    // we don't need the private half for the unencrypted handshake.
    const hg::SodiumPSKeys clientKeys = hg::generateSodiumPSKeys();

    sf::Packet          outPacket;
    sf::Packet          inPacket;
    sf::OutStringStream errOss;

    for (int i = 0; i < count; ++i)
    {
        outPacket.clear();
        hg::makeClientToServerPacket(outPacket, hg::CTSPPublicKey{.key = clientKeys.keyPublic});

        if (sock.send(outPacket) != sf::Socket::Status::Done)
        {
            std::fprintf(stderr, "[smoke-client] sending CTSPPublicKey failed (round-trip %d/%d)\n", i + 1, count);
            return 1;
        }

        inPacket.clear();
        if (sock.receive(inPacket) != sf::Socket::Status::Done)
        {
            std::fprintf(stderr, "[smoke-client] receiving server reply failed (round-trip %d/%d)\n", i + 1, count);
            return 1;
        }

        const hg::PVServerToClient decoded = hg::decodeServerToClientPacket(/* keyReceive */ nullptr, errOss, inPacket);

        if (decoded.getIf<hg::STCPPublicKey>() == nullptr)
        {
            std::fprintf(stderr,
                         "[smoke-client] expected STCPPublicKey on round-trip %d/%d, got something else (decode error: "
                         "'%s')\n",
                         i + 1,
                         count,
                         errOss.getString().c_str());
            return 1;
        }
    }

    sock.disconnect();
    std::fprintf(stdout, "[smoke-client] OK [%d round-trip%s]\n", count, count == 1 ? "" : "s");
    return 0;
}
