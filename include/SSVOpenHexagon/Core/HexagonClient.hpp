// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <list>

namespace hg
{

class HexagonClient
{
private:
    const sf::IpAddress _serverIp;
    const unsigned short _serverPort;

    sf::TcpSocket _socket;

    void initializeTcpSocket();

public:
    explicit HexagonClient();
    ~HexagonClient();

    HexagonClient(const HexagonClient&) = delete;
    HexagonClient(HexagonClient&&) = delete;
};

} // namespace hg
