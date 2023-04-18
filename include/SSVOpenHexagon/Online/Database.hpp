// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"

#include <string>
#include <cstdint>
#include <optional>
#include <vector>

// TODO (P2): remove reliance on steam ID for future platforms

namespace hg::Database {

void addUser(const User& user);

void removeUser(const std::uint32_t id);

void dumpUsers();

[[nodiscard]] bool anyUserWithSteamId(const std::uint64_t steamId);

[[nodiscard]] bool anyUserWithName(const std::string& name);

[[nodiscard]] std::optional<User> getUserWithSteamIdAndName(
    const std::uint64_t steamId, const std::string& name);

void removeAllLoginTokensForUser(const std::uint32_t userId);

void addLoginToken(const LoginToken& loginToken);

[[nodiscard]] std::vector<User> getAllUsersWithSteamId(
    const std::uint64_t steamId);

[[nodiscard]] std::optional<User> getUserWithSteamId(
    const std::uint64_t steamId);

[[nodiscard]] std::vector<LoginToken> getAllStaleLoginTokens();
void removeAllStaleLoginTokens();

[[nodiscard]] std::vector<ProcessedScore> getTopScores(
    const int topLimit, const std::string& levelValidator);

[[nodiscard]] bool isLoginTokenValid(std::uint64_t token);

void addScore(const std::string& levelValidator, const std::uint64_t timestamp,
    const std::uint64_t userSteamId, const double value);

[[nodiscard]] std::optional<ProcessedScore> getScore(
    const std::string& levelValidator, const std::uint64_t userSteamId);

[[nodiscard]] std::optional<std::string> execute(const std::string& query);

} // namespace hg::Database
