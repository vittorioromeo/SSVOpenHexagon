// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"

#include "SSVOpenHexagon/Core/Replay.hpp"

#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Global/ProtocolVersion.hpp"

#include <vrm/pp/tpl.hpp>

#include <sodium.h>

#include <sstream>
#include <optional>
#include <variant>
#include <array>
#include <string>
#include <vector>

#include <cstdint>
#include <cstddef>
#include <cstdint>

namespace sf {

class Packet;

}

namespace hg {

namespace Impl {

struct CiphertextVectorPtr
{
    std::vector<std::uint8_t>* ptr;
};

} // namespace Impl

struct PInvalid
{
    std::string error;
};

struct PEncryptedMsg
{
    SodiumNonceArray nonce;
    std::uint64_t messageLength;
    std::uint64_t ciphertextLength;
    Impl::CiphertextVectorPtr ciphertext;
};

// ----------------------------------------------------------------------------

// clang-format off
struct CTSPHeartbeat                   { };
struct CTSPDisconnect                  { };
struct CTSPPublicKey                   { SodiumPublicKeyArray key; };
struct CTSPRegister                    { std::uint64_t steamId; std::string name; std::string passwordHash; };
struct CTSPLogin                       { std::uint64_t steamId; std::string name; std::string passwordHash; };
struct CTSPLogout                      { std::uint64_t steamId; };
struct CTSPDeleteAccount               { std::uint64_t steamId; std::string passwordHash; };
struct CTSPRequestTopScores            { std::uint64_t loginToken; std::string levelValidator; };
struct CTSPReplay                      { std::uint64_t loginToken; replay_file replayFile; };
struct CTSPRequestOwnScore             { std::uint64_t loginToken; std::string levelValidator; };
struct CTSPRequestTopScoresAndOwnScore { std::uint64_t loginToken; std::string levelValidator; };
struct CTSPStartedGame                 { std::uint64_t loginToken; std::string levelValidator; };
struct CTSPCompressedReplay            { std::uint64_t loginToken; compressed_replay_file compressedReplayFile; };
struct CTSPRequestServerStatus         { std::uint64_t loginToken; };
struct CTSPReady                       { std::uint64_t loginToken; };
// clang-format on

#define SSVOH_CTS_PACKETS                                         \
    VRM_PP_TPL_MAKE(CTSPHeartbeat, CTSPDisconnect, CTSPPublicKey, \
        CTSPRegister, CTSPLogin, CTSPLogout, CTSPDeleteAccount,   \
        CTSPRequestTopScores, CTSPReplay, CTSPRequestOwnScore,    \
        CTSPRequestTopScoresAndOwnScore, CTSPStartedGame,         \
        CTSPCompressedReplay, CTSPRequestServerStatus, CTSPReady)

using PVClientToServer = std::variant<PInvalid, PEncryptedMsg,
    VRM_PP_TPL_EXPLODE(SSVOH_CTS_PACKETS)>;

// ----------------------------------------------------------------------------

template <typename T>
void makeClientToServerPacket(sf::Packet& p, const T& data);

template <typename T>
[[nodiscard]] bool makeClientToServerEncryptedPacket(
    const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data);

[[nodiscard]] PVClientToServer decodeClientToServerPacket(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p);

// ----------------------------------------------------------------------------

// clang-format off
struct STCPKick                   { };
struct STCPPublicKey              { SodiumPublicKeyArray key; };
struct STCPRegistrationSuccess    { };
struct STCPRegistrationFailure    { std::string error; };
struct STCPLoginSuccess           { std::uint64_t loginToken; std::string loginName; };
struct STCPLoginFailure           { std::string error; };
struct STCPLogoutSuccess          { };
struct STCPLogoutFailure          { };
struct STCPDeleteAccountSuccess   { };
struct STCPDeleteAccountFailure   { std::string error; };
struct STCPTopScores              { std::string levelValidator; std::vector<Database::ProcessedScore> scores; };
struct STCPOwnScore               { std::string levelValidator; Database::ProcessedScore score; };
struct STCPTopScoresAndOwnScore   { std::string levelValidator; std::vector<Database::ProcessedScore> scores; std::optional<Database::ProcessedScore> ownScore; };
struct STCPServerStatus           { ProtocolVersion protocolVersion; GameVersion gameVersion; std::vector<std::string> supportedLevelValidators; };
// clang-format on

#define SSVOH_STC_PACKETS                                               \
    VRM_PP_TPL_MAKE(STCPKick, STCPPublicKey, STCPRegistrationSuccess,   \
        STCPRegistrationFailure, STCPLoginSuccess, STCPLoginFailure,    \
        STCPLogoutSuccess, STCPLogoutFailure, STCPDeleteAccountSuccess, \
        STCPDeleteAccountFailure, STCPTopScores, STCPOwnScore,          \
        STCPTopScoresAndOwnScore, STCPServerStatus)

using PVServerToClient = std::variant<PInvalid, PEncryptedMsg,
    VRM_PP_TPL_EXPLODE(SSVOH_STC_PACKETS)>;

// ----------------------------------------------------------------------------

template <typename T>
void makeServerToClientPacket(sf::Packet& p, const T& data);

template <typename T>
[[nodiscard]] bool makeServerToClientEncryptedPacket(
    const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data);

[[nodiscard]] PVServerToClient decodeServerToClientPacket(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p);

} // namespace hg
