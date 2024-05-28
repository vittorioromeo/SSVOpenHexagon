#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/Socket.hpp>
#include <SFML/Network/UdpSocket.hpp>

#include <string>
#include <iostream>

namespace {

[[nodiscard]] bool cin_getline_string(std::string& result) noexcept
{
    return static_cast<bool>(std::getline(std::cin, result));
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 1)
    {
        std::cerr << "Fatal error: no executable specified" << std::endl;
        return -1;
    }

    if (argc > 2)
    {
        std::cerr << "Invalid number of arguments" << std::endl;
        return -1;
    }

    std::string stringBuf;
    sf::Packet packet;
    sf::UdpSocket controlSocket;

    const auto sendToServer = [&]
    {
        packet.clear();
        packet << stringBuf;

        if (controlSocket.send(packet, sf::IpAddress::LocalHost, 50506) !=
            sf::Socket::Status::Done)
        {
            std::cerr << "Error sending control packet\n";
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
                std::cerr << "Error reading line from stdin\n";
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
