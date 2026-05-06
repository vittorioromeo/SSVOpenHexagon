// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/StringHash.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <unordered_map>
#include <unordered_set>

namespace hg
{

// Per-level user metadata, keyed by level *id* (one entry per level, all
// difficulties share). The new UI's level-select screen uses this for sort
// keys ("recently played", "play count") and filters ("only show
// uncompleted"). Persisted alongside `scores` / `favorites` in the profile
// JSON.
struct PerLevelState
{
    sf::base::U64 lastPlayedTs{0};
    sf::base::U32 playCount{0};
    bool          isCompleted{false};
};

class ProfileData
{
private:
    GameVersion                                 version;
    sf::base::String                            name;
    std::unordered_map<sf::base::String, float> scores;
    std::unordered_set<sf::base::String>        favoriteLevelsDataIDs;
    std::unordered_map<sf::base::String, PerLevelState> perLevelState;

public:
    ProfileData(const GameVersion                                  mVersion,
                const sf::base::String&                            mName,
                const std::unordered_map<sf::base::String, float>& mScores,
                const sf::base::Vector<sf::base::String>&          mFavorites);

    [[nodiscard]] GameVersion                                        getVersion() const noexcept;
    [[nodiscard]] const sf::base::String&                            getName() const noexcept;
    [[nodiscard]] const std::unordered_map<sf::base::String, float>& getScores() const noexcept;

    [[nodiscard]] std::unordered_set<sf::base::String>& getFavoriteLevelIds() noexcept;

    [[nodiscard]] const std::unordered_set<sf::base::String>& getFavoriteLevelIds() const noexcept;

    void                setScore(const sf::base::String& mId, const float mScore);
    [[nodiscard]] float getScore(const sf::base::String& mId) const;

    void addFavoriteLevel(const sf::base::String& mLevelID);
    void removeFavoriteLevel(const sf::base::String& mLevelID);

    [[nodiscard]] bool isLevelFavorite(const sf::base::String& mLevelID) const noexcept;

    // Per-level state -- never throws; missing entries are returned as
    // zero-initialised. Mutating accessor `getOrCreatePerLevelState` inserts
    // an empty entry on first access.
    [[nodiscard]] PerLevelState        getPerLevelState   (const sf::base::String& mLevelId) const noexcept;
    [[nodiscard]] PerLevelState&       getOrCreatePerLevelState(const sf::base::String& mLevelId);
    [[nodiscard]] const std::unordered_map<sf::base::String, PerLevelState>& getPerLevelStates() const noexcept;
};

} // namespace hg
