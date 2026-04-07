// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/StringHash.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <unordered_map>
#include <unordered_set>

namespace hg
{

class ProfileData
{
private:
    GameVersion                                 version;
    sf::base::String                            name;
    std::unordered_map<sf::base::String, float> scores;
    std::unordered_set<sf::base::String>        favoriteLevelsDataIDs;

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
};

} // namespace hg
