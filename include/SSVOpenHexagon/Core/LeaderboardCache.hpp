// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once


#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/Utils/Clock.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/UniquePtr.hpp"
#include "SFML/Base/Vector.hpp"

namespace hg
{

class LeaderboardCache
{
private:
    struct CachedScores
    {
        sf::base::Vector<Database::ProcessedScore>   _scores;
        sf::base::Optional<Database::ProcessedScore> _ownScore;
        HRTimePoint                                  _cacheTime;
        bool                                         _received{false}; // Becomes true only once the server replies
        ankerl::unordered_dense::set<sf::base::U64>
            _unavailableTimestamps; // Set of score timestamps that have no stored replay on the server
    };

    // `UniquePtr` indirection gives address stability: callers (e.g.
    // `Services::leaderboardUnavailable`) hold pointers into individual
    // `CachedScores` that must survive map rehashes.
    ankerl::unordered_dense::map<sf::base::String, sf::base::UniquePtr<CachedScores>> _levelValidatorToScores;

    // Empty fallback returned by `getUnavailableTimestamps` when no entry
    // exists for the validator. Lets callers avoid null-checking.
    static const ankerl::unordered_dense::set<sf::base::U64> kEmptyUnavailable;

    [[nodiscard]] CachedScores& getOrCreate(const sf::base::String& levelValidator);

public:
    void receivedScores(const sf::base::String& levelValidator, const sf::base::Vector<Database::ProcessedScore>& scores);

    void receivedOwnScore(const sf::base::String& levelValidator, const Database::ProcessedScore& score);

    void requestedScores(const sf::base::String& levelValidator);

    // Marks `(levelValidator, scoreTimestamp)` as having no stored
    // replay. Subsequent lookups via `isReplayUnavailable` return true
    // without re-asking the server.
    void markReplayUnavailable(const sf::base::String& levelValidator, sf::base::U64 scoreTimestamp);

    [[nodiscard]] bool shouldRequestScores(const sf::base::String& levelValidator) const;

    [[nodiscard]] const sf::base::Vector<Database::ProcessedScore>& getScores(const sf::base::String& levelValidator) const;

    [[nodiscard]] const Database::ProcessedScore* getOwnScore(const sf::base::String& levelValidator) const;

    [[nodiscard]] bool getSupported(const sf::base::String& levelValidator) const;
    [[nodiscard]] bool hasInformation(const sf::base::String& levelValidator) const;

    // True iff a successful `EReceivedTopScores` event has landed for
    // this validator. Use to distinguish "request still in flight" from
    // "server replied with zero scores".
    [[nodiscard]] bool hasReceivedScores(const sf::base::String& levelValidator) const;

    // Stable reference to the set of "no replay" timestamps for a validator.
    [[nodiscard]] const ankerl::unordered_dense::set<sf::base::U64>& getUnavailableTimestamps(
        const sf::base::String& levelValidator) const;
};

} // namespace hg
