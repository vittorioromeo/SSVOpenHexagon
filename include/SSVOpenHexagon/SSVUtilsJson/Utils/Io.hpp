// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVUtils/Core/Log/Log.hpp>
#include <SSVUtils/Core/FileSystem/FileSystem.hpp>

#include <sstream>

namespace ssvuj
{
namespace Impl
{
inline bool tryParse(Obj& mObj, Reader& mReader, const std::string& mSrc)
{
    if(mReader.parse(mSrc, mObj, false)) return true;
    ssvu::lo("ssvuj::logReadError") << mReader.getFormattedErrorMessages()
                                    << "\nFrom: [" << mSrc << "]" << std::endl;
    return false;
}
} // namespace Impl

inline void readFromString(Obj& mObj, const std::string& mStr)
{
    Reader reader;
    Impl::tryParse(mObj, reader, mStr);
}
inline void readFromFile(Obj& mObj, const ssvufs::Path& mPath)
{
    Reader reader;
    Impl::tryParse(mObj, reader, mPath.getContentsAsStr());
}
inline void readFromFile(
    Obj& mObj, const ssvufs::Path& mPath, std::string& mError)
{
    Reader reader;
    Impl::tryParse(mObj, reader, mPath.getContentsAsStr());
    if(reader.getFormattedErrorMessages().empty())
    {
        mError = "";
    }
    else
    {
        mError = reader.getFormattedErrorMessages() + " in file " +
                 mPath.getFileName();
    }
}

inline Obj getFromStr(const std::string& mStr)
{
    Obj result;
    readFromString(result, mStr);
    return result;
}
inline Obj getFromFile(const ssvufs::Path& mPath)
{
    Obj result;
    readFromFile(result, mPath);
    return result;
}
inline std::pair<Obj, std::string> getFromFileWithErrors(
    const ssvufs::Path& mPath)
{
    Obj result;
    std::string error;
    readFromFile(result, mPath, error);
    return std::make_pair(result, error);
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
inline auto getWriteToString(const Obj& mObj)
{
    std::string result;
    writeToString(mObj, result);
    return result;
}

} // namespace ssvuj
