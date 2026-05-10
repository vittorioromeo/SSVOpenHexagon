#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/String.hpp"

namespace
{

[[nodiscard]] bool cin_getline_string(sf::base::String& result) noexcept
{
    return sf::getLine(sf::cIn(), result);
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 1)
    {
        sf::cErr() << "Fatal error: no executable specified" << sf::endL;
        return -1;
    }

    if (argc > 2)
    {
        sf::cErr() << "Invalid number of arguments" << sf::endL;
        return -1;
    }

    sf::base::String stringBuf;
    sf::Packet       packet;

    auto controlSocketOpt = sf::UdpSocket::create(true /* isBlocking */);
    if (!controlSocketOpt.hasValue())
    {
        sf::cErr() << "Failed to create UDP control socket\n";
        return -1;
    }

    sf::UdpSocket& controlSocket = *controlSocketOpt;

    const auto sendToServer = [&]
    {
        packet.clear();
        packet << stringBuf;

        if (controlSocket.send(packet, sf::IpAddress::LocalHost, 50'506) != sf::Socket::Status::Done)
        {
            sf::cErr() << "Error sending control packet\n";
            return false;
        }

        return true;
    };

    if (argc == 1) // Interactive mode
    {
        while (true)
        {
            if (!cin_getline_string(stringBuf))
            {
                sf::cErr() << "Error reading line from stdin\n";
                continue;
            }

            sendToServer();
        }
    }

    if (argc == 2) // One-off send
    {
        stringBuf = argv[1];
        return sendToServer() ? 0 : 1;
    }
}
