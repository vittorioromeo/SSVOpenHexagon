// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/SSVUtilsJson/JsonCpp/json.hpp"
#include "SSVOpenHexagon/Utils/Log.hpp"
#include "steam/steamclientpublic.h" // IWYU pragma: keep -- EResult, instantiated for `<<` below

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"         // IWYU pragma: keep
#include "SFML/System/PathStreamOp.hpp" // IWYU pragma: keep

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp" // IWYU pragma: keep -- print() formats numeric args
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp" // IWYU pragma: keep
#include "SFML/Base/StringStreamOp.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/StringViewStreamOp.hpp"

#include <fstream>
#include <ios>      // std::streamsize
#include <ostream>  // std::endl


namespace hg
{

namespace
{
// Route console output through Fmt for types that have an ADL `fmtArg`
// overload. Types without one (e.g. `Json::Value`, `EResult`, raw pointers)
// still hit the file stream below but skip the console mirror -- preferable
// to forcing every consumer to provide an `fmtArg` overload up front.
template <typename T>
void consoleOut(const T& value)
{
    if constexpr (requires(sf::base::FmtSink& sink, const sf::base::FmtSpec& spec) { fmtArg(sink, value, spec); })
    {
        sf::base::print("{}", value);
    }
}
} // namespace

struct LogStream::Impl
{
    std::ofstream logFileStream{"log.txt"};
};

LogStream& LogStream::operator<<(const LogEndl&)
{
    sf::base::print("\n");
    sf::base::priv::fmtFlushStdout();
    impl->logFileStream << std::endl;
    return *this;
}

LogStream& LogStream::operator<<(const sf::base::StringView& value)
{
    sf::base::print("{}", value);
    impl->logFileStream << value;
    return *this;
}

template <typename T>
LogStream& LogStream::operator<<(const T& value)
{
    consoleOut(value);
    impl->logFileStream << value;
    return *this;
}

// `Json::Value` previously formatted via `operator<<(std::ostream&, const Value&)`
// in vendored jsoncpp. The writer now streams into any `FmtSinkRef`, so we
// build a tee that fans the bytes out to stdout and the log file in a single
// pass -- no full-document buffering in between.
namespace
{
struct StdoutAppendSink
{
    void append(const char* data, sf::base::SizeT n)
    {
        sf::base::priv::fmtWriteStdout(data, n);
    }
};

struct OFStreamAppendSink
{
    std::ofstream& s;
    void           append(const char* data, sf::base::SizeT n)
    {
        s.write(data, static_cast<std::streamsize>(n));
    }
};

struct TeeAppendSink
{
    sf::base::FmtSinkRef a, b;
    void                 append(const char* data, sf::base::SizeT n)
    {
        a.append(data, n);
        b.append(data, n);
    }
};
} // namespace

template <>
LogStream& LogStream::operator<<<Json::Value>(const Json::Value& value)
{
    StdoutAppendSink   stdoutSink;
    OFStreamAppendSink fileSink{impl->logFileStream};
    TeeAppendSink      tee{stdoutSink, fileSink};

    Json::StyledWriter writer;
    writer.write(value, tee);
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
INSTANTIATE_LOGSTREAM_OPERATOR(sf::base::String);
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
INSTANTIATE_LOGSTREAM_OPERATOR(EResult);
INSTANTIATE_LOGSTREAM_OPERATOR(sf::Path);

void LogStream::flush()
{
    sf::base::priv::fmtFlushStdout();
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
    sf::base::print("[{}]", title);

    auto& logStream = getLogStream();
    logStream.impl->logFileStream << '[' << title << ']';

    return logStream;
}

} // namespace hg
