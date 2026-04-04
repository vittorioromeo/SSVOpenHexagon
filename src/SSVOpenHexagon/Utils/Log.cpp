// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/SSVUtilsJson/JsonCpp/json.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/JsonCpp/jsoncpp.inl"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "SSVUtils/Core/FileSystem/Path.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/StringViewStreamOp.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <steam/steamclientpublic.h>
#include <string>


namespace hg
{

struct LogStream::Impl
{
    std::ofstream logFileStream{"log.txt"};
};

LogStream& LogStream::operator<<(const LogEndl&)
{
    std::cout << std::endl;
    impl->logFileStream << std::endl;
    return *this;
}

LogStream& LogStream::operator<<(const sf::base::StringView& value)
{
    std::cout << value;
    impl->logFileStream << value;
    return *this;
}

template <typename T>
LogStream& LogStream::operator<<(const T& value)
{
    std::cout << value;
    impl->logFileStream << value;
    return *this;
}

#define INSTANTIATE_LOGSTREAM_OPERATOR(...) \
    template LogStream& LogStream::operator<< <__VA_ARGS__>(__VA_ARGS__ const& value)

INSTANTIATE_LOGSTREAM_OPERATOR(bool);
INSTANTIATE_LOGSTREAM_OPERATOR(char);
INSTANTIATE_LOGSTREAM_OPERATOR(char*);
INSTANTIATE_LOGSTREAM_OPERATOR(const char*);
INSTANTIATE_LOGSTREAM_OPERATOR(float);
INSTANTIATE_LOGSTREAM_OPERATOR(int);
INSTANTIATE_LOGSTREAM_OPERATOR(long);
INSTANTIATE_LOGSTREAM_OPERATOR(short*);
INSTANTIATE_LOGSTREAM_OPERATOR(std::string_view);
INSTANTIATE_LOGSTREAM_OPERATOR(std::string);
INSTANTIATE_LOGSTREAM_OPERATOR(unsigned int);
INSTANTIATE_LOGSTREAM_OPERATOR(unsigned long long);
INSTANTIATE_LOGSTREAM_OPERATOR(unsigned long);
INSTANTIATE_LOGSTREAM_OPERATOR(long long);
INSTANTIATE_LOGSTREAM_OPERATOR(unsigned char);
INSTANTIATE_LOGSTREAM_OPERATOR(unsigned short);
INSTANTIATE_LOGSTREAM_OPERATOR(double);
INSTANTIATE_LOGSTREAM_OPERATOR(void*);
INSTANTIATE_LOGSTREAM_OPERATOR(const void*);
INSTANTIATE_LOGSTREAM_OPERATOR(sf::base::String);
INSTANTIATE_LOGSTREAM_OPERATOR(Json::Value);
INSTANTIATE_LOGSTREAM_OPERATOR(EResult);
INSTANTIATE_LOGSTREAM_OPERATOR(ssvu::FileSystem::Path);
INSTANTIATE_LOGSTREAM_OPERATOR(std::filesystem::path);

void LogStream::flush()
{
    std::cout.flush();
    impl->logFileStream.flush();
}

LogStream& getLogStream()
{
    static LogStream logStream;
    return logStream;
}

LogStream& lo()
{
    return getLogStream();
}

LogStream& lo(sf::base::StringView title)
{
    std::cout << '[' << title << ']';

    auto& logStream = getLogStream();
    logStream.impl->logFileStream << '[' << title << ']';

    return logStream;
}

} // namespace hg
