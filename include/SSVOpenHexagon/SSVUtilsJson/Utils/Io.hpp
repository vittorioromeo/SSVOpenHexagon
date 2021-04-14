// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <string>
#include <sstream>

namespace ssvuj {

namespace Impl {

[[nodiscard]] inline std::string& getBuffer()
{
    thread_local std::string buffer;
    return buffer;
}

[[nodiscard]] inline bool tryParse(
    Obj& mObj, Reader& mReader, const std::string& mSrc)
{
    if(mReader.parse(mSrc, mObj, false))
    {
        return true;
    }

    ssvu::lo("ssvuj::logReadError") << mReader.getFormattedErrorMessages()
                                    << "\nFrom: [" << mSrc << "]" << std::endl;

    return false;
}

} // namespace Impl

[[nodiscard]] inline bool readFromString(Obj& mObj, const std::string& mStr)
{
    Reader reader;
    return Impl::tryParse(mObj, reader, mStr);
}

[[nodiscard]] inline bool readFromFile(Obj& mObj, const ssvufs::Path& mPath)
{
    Reader reader;
    return Impl::tryParse(
        mObj, reader, mPath.getContentsAsStr(Impl::getBuffer()));
}

[[nodiscard]] inline bool readFromFile(
    Obj& mObj, const ssvufs::Path& mPath, std::string& mError)
{
    Reader reader;
    if(!Impl::tryParse(mObj, reader, mPath.getContentsAsStr(Impl::getBuffer())))
    {
        if(reader.getFormattedErrorMessages().empty())
        {
            mError = "";
        }
        else
        {
            mError = reader.getFormattedErrorMessages() + " in file " +
                     mPath.getFileName();
        }

        return false;
    }

    return true;
}

[[nodiscard]] inline Obj getFromStr(const std::string& mStr)
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

[[nodiscard]] inline std::pair<Obj, std::string> getFromFileWithErrors(
    const ssvufs::Path& mPath)
{
    Obj result;
    std::string error;
    (void)readFromFile(result, mPath, error);
    return {result, error};
}

inline void writeToStream(const Obj& mObj, std::ostream& mStream)
{
    Writer writer;
    writer.write(mStream, mObj);
    mStream.flush();
}

inline void writeToString(const Obj& mObj, std::string& mStr)
{
    std::ostringstream o;
    writeToStream(mObj, o);
    mStr = o.str();
}

inline void writeToFile(const Obj& mObj, const ssvufs::Path& mPath)
{
    std::ofstream o{mPath};
    writeToStream(mObj, o);
    o.close();
}

[[nodiscard]] inline std::string getWriteToString(const Obj& mObj)
{
    std::string result;
    writeToString(mObj, result);
    return result;
}

} // namespace ssvuj
