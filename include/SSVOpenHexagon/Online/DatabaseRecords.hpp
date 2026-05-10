// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/String.hpp"

namespace hg::Database
{

// `ProcessedScore` is the only `DatabaseRecords` type referenced by public headers
// (HexagonClient/HexagonServer). The sqlite_orm-bound `User`, `Score`, `LoginToken`
// structs live in `DatabaseInternals.hpp`, which is included only by `Database.cpp`,
// so the orm types and their `<string>` / `<vector>` dependencies stay out of the
// public include graph.
struct ProcessedScore
{
    sf::base::U32    position;
    sf::base::String userName;
    sf::base::U64    scoreTimestamp;
    double           scoreValue;
};

} // namespace hg::Database
