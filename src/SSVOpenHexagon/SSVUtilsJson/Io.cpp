// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/JsonCpp/JsonStream.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/Utils/Io.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/String.hpp"


namespace ssvuj
{

namespace
{

[[nodiscard]] bool tryParse(Obj& mObj, Reader& mReader, const sf::base::String& mSrc)
{
    if (mReader.parse(mSrc, mObj, false))
        return true;

    sf::base::printLn("ssvuj::logReadError:{}\nFrom: [{}]", mReader.getFormattedErrorMessages(), mSrc);

    return false;
}

} // namespace

bool readFromString(Obj& mObj, const sf::base::String& mStr)
{
    Reader reader;
    return tryParse(mObj, reader, mStr);
}

bool readFromFile(Obj& mObj, const sf::Path& mPath)
{
    sf::base::String contents;
    if (!sf::readFromFile(mPath, contents))
        return false;

    Reader reader;
    return tryParse(mObj, reader, contents);
}

bool readFromFile(Obj& mObj, const sf::Path& mPath, sf::base::String& mError)
{
    sf::base::String contents;
    if (!sf::readFromFile(mPath, contents))
    {
        mError = sf::base::String("Could not read file: ") + mPath.getFilename().to<sf::base::String>();
        return false;
    }

    Reader reader;
    if (!tryParse(mObj, reader, contents))
    {
        const sf::base::String msg = reader.getFormattedErrorMessages();
        if (msg.empty())
            mError = "";
        else
            mError = msg + " in file " + mPath.getFilename().to<sf::base::String>();

        return false;
    }

    return true;
}

Obj getFromStr(const sf::base::String& mStr)
{
    Obj result;
    (void)readFromString(result, mStr);
    return result;
}

Obj getFromFile(const sf::Path& mPath)
{
    Obj result;
    (void)readFromFile(result, mPath);
    return result;
}

std::pair<Obj, sf::base::String> getFromFileWithErrors(const sf::Path& mPath)
{
    Obj              result;
    sf::base::String error;
    (void)readFromFile(result, mPath, error);
    return {result, error};
}

void writeToString(const Obj& mObj, sf::base::String& mStr)
{
    Json::StyledStreamWriter writer;
    writer.write(mStr, mObj);
}

void writeToFile(const Obj& mObj, const sf::Path& mPath)
{
    sf::base::String serialized;
    writeToString(mObj, serialized);
    (void)sf::writeToFile(mPath, sf::base::StringView{serialized.data(), serialized.size()});
}

bool readFromFile(Obj& mObj, const char* mPath)
{
    return readFromFile(mObj, sf::Path{mPath});
}

bool readFromFile(Obj& mObj, const char* mPath, sf::base::String& mError)
{
    return readFromFile(mObj, sf::Path{mPath}, mError);
}

Obj getFromFile(const char* mPath)
{
    return getFromFile(sf::Path{mPath});
}

std::pair<Obj, sf::base::String> getFromFileWithErrors(const char* mPath)
{
    return getFromFileWithErrors(sf::Path{mPath});
}

void writeToFile(const Obj& mObj, const char* mPath)
{
    writeToFile(mObj, sf::Path{mPath});
}

sf::base::String getWriteToString(const Obj& mObj)
{
    sf::base::String result;
    writeToString(mObj, result);
    return result;
}

} // namespace ssvuj
