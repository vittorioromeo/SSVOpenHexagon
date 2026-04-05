// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/Vector.hpp"

#include <functional>
#include <string>

namespace Lua
{
class LuaContext;
}

namespace hg
{
class random_number_generator;
class CCustomWallManager;
struct LevelStatus;
struct HexagonGameStatus;
class StyleData;
class HGAssets;
struct PackData;
} // namespace hg

namespace hg::Utils
{
class LuaMetadata;
}

namespace hg::LuaScripting
{

[[nodiscard]] Utils::LuaMetadata& getMetadata();

void init(Lua::LuaContext&                               lua,
          random_number_generator&                       rng,
          const bool                                     inMenu,
          CCustomWallManager&                            cwManager,
          LevelStatus&                                   levelStatus,
          HexagonGameStatus&                             hexagonGameStatus,
          StyleData&                                     styleData,
          HGAssets&                                      assets,
          const std::function<void(const std::string&)>& fRunLuaFile,
          sf::base::Vector<std::string>&                 execScriptPackPathContext,
          const std::function<const std::string&()>&     fPackPathGetter,
          const std::function<const PackData&()>&        fGetPackData,
          const bool                                     headless);

void printDocs();

const sf::base::Vector<std::string>& getAllFunctionNames();

std::string getDocsForFunction(const std::string& fnName);

} // namespace hg::LuaScripting
