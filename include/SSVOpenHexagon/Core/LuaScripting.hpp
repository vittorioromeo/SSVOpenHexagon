// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <vector>
#include <string>

namespace Lua
{

class LuaContext;

}

namespace hg
{

class random_number_generator;
class CCustomWallManager;

} // namespace hg

namespace hg::Utils
{

class LuaMetadata;

}

namespace hg::LuaScripting
{

[[nodiscard]] Utils::LuaMetadata& getMetadata();

void init(Lua::LuaContext& lua, random_number_generator& rng, const bool inMenu,
    CCustomWallManager& cwManager);

void printDocs();

const std::vector<std::string>& getAllFunctionNames();

std::string getDocsForFunction(const std::string& fnName);

} // namespace hg::LuaScripting
