// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/System/IO.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/StringStreamOp.hpp"

#include <SSVUtils/Core/FileSystem/FileSystem.hpp>
#include <iostream>
#include <ostream>

namespace ssvuj
{

namespace Impl
{

[[nodiscard]] inline std::string& getBuffer()
{
    thread_local std::string buffer;
    return buffer;
}

[[nodiscard]] inline bool tryParse(Obj& mObj, Reader& mReader, const std::string& mSrc)
{
    if (mReader.parse(mSrc, mObj, false))
    {
        return true;
    }

    std::cout << "ssvuj::logReadError:" << mReader.getFormattedErrorMessages() << "\nFrom: [" << mSrc << "]" << std::endl;

    return false;
}

} // namespace Impl

[[nodiscard]] inline bool readFromString(Obj& mObj, const sf::base::String& mStr)
{
    Reader reader;
    return Impl::tryParse(mObj, reader, std::string(mStr.cStr()));
}

[[nodiscard]] inline bool readFromFile(Obj& mObj, const ssvufs::Path& mPath)
{
    Reader reader;
    return Impl::tryParse(mObj, reader, mPath.getContentsAsStr(Impl::getBuffer()));
}

[[nodiscard]] inline bool readFromFile(Obj& mObj, const ssvufs::Path& mPath, sf::base::String& mError)
{
    Reader reader;
    if (!Impl::tryParse(mObj, reader, mPath.getContentsAsStr(Impl::getBuffer())))
    {
        if (reader.getFormattedErrorMessages().empty())
        {
            mError = "";
        }
        else
        {
            mError = reader.getFormattedErrorMessages() + " in file " + mPath.getFileName();
        }

        return false;
    }

    return true;
}

[[nodiscard]] inline Obj getFromStr(const sf::base::String& mStr)
{
    Obj result;
    (void)readFromString(result, mStr);
    return result;
}

[[nodiscard]] inline Obj getFromFile(const ssvufs::Path& mPath)
{
    Obj result;
    (void)readFromFile(result, mPath);
    return result;
}

[[nodiscard]] inline std::pair<Obj, sf::base::String> getFromFileWithErrors(const ssvufs::Path& mPath)
{
    Obj              result;
    sf::base::String error;
    (void)readFromFile(result, mPath, error);
    return {result, error};
}

inline void writeToStream(const Obj& mObj, std::ostream& mStream)
{
    Writer writer;
    writer.write(mStream, mObj);
    mStream.flush();
}

inline void writeToString(const Obj& mObj, sf::base::String& mStr)
{
    sf::OutStringStream o;
    std::ostream        tmp{o.rdbuf()};
    writeToStream(mObj, tmp);
    mStr = o.to<sf::base::String>();
}

inline void writeToFile(const Obj& mObj, const ssvufs::Path& mPath)
{
    std::ofstream o{mPath};
    writeToStream(mObj, o);
    o.close();
}

[[nodiscard]] inline sf::base::String getWriteToString(const Obj& mObj)
{
    sf::base::String result;
    writeToString(mObj, result);
    return result;
}

} // namespace ssvuj
