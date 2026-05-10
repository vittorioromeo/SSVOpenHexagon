// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/SSVUtilsJson/Global/Common.hpp"

#include "SFML/Base/String.hpp"

#include <utility>


namespace sf
{
class Path;
} // namespace sf


namespace ssvuj
{

[[nodiscard]] bool readFromString(Obj& mObj, const sf::base::String& mStr);
[[nodiscard]] bool readFromFile(Obj& mObj, const sf::Path& mPath);
[[nodiscard]] bool readFromFile(Obj& mObj, const sf::Path& mPath, sf::base::String& mError);
[[nodiscard]] bool readFromFile(Obj& mObj, const char* mPath);
[[nodiscard]] bool readFromFile(Obj& mObj, const char* mPath, sf::base::String& mError);

[[nodiscard]] Obj                              getFromStr(const sf::base::String& mStr);
[[nodiscard]] Obj                              getFromFile(const sf::Path& mPath);
[[nodiscard]] Obj                              getFromFile(const char* mPath);
[[nodiscard]] std::pair<Obj, sf::base::String> getFromFileWithErrors(const sf::Path& mPath);
[[nodiscard]] std::pair<Obj, sf::base::String> getFromFileWithErrors(const char* mPath);

void writeToString(const Obj& mObj, sf::base::String& mStr);
void writeToFile(const Obj& mObj, const sf::Path& mPath);
void writeToFile(const Obj& mObj, const char* mPath);

[[nodiscard]] sf::base::String getWriteToString(const Obj& mObj);

} // namespace ssvuj
