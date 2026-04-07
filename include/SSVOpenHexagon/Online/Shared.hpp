// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Global/ProtocolVersion.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Online/DatabaseRecords.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"

#include <sodium.h>
#include <string>

namespace sf
{

class Packet;

}

namespace hg
{

namespace Impl
{

struct CiphertextVectorPtr
{
    sf::base::Vector<sf::base::U8>* ptr;
};

} // namespace Impl

struct PInvalid
{
    std::string error;
};

struct PEncryptedMsg
{
    SodiumNonceArray          nonce;
    sf::base::U64             messageLength;
    sf::base::U64             ciphertextLength;
    Impl::CiphertextVectorPtr ciphertext;
};

// ----------------------------------------------------------------------------

// clang-format off
struct CTSPHeartbeat                   { };
struct CTSPDisconnect                  { };
struct CTSPPublicKey                   { SodiumPublicKeyArray key; };
struct CTSPRegister                    { sf::base::U64 steamId; std::string name; std::string passwordHash; };
struct CTSPLogin                       { sf::base::U64 steamId; std::string name; std::string passwordHash; };
struct CTSPLogout                      { sf::base::U64 steamId; };
struct CTSPDeleteAccount               { sf::base::U64 steamId; std::string passwordHash; };
struct CTSPRequestTopScores            { sf::base::U64 loginToken; std::string levelValidator; };
struct CTSPReplay                      { sf::base::U64 loginToken; replay_file replayFile; };
struct CTSPRequestOwnScore             { sf::base::U64 loginToken; std::string levelValidator; };
struct CTSPRequestTopScoresAndOwnScore { sf::base::U64 loginToken; std::string levelValidator; };
struct CTSPStartedGame                 { sf::base::U64 loginToken; std::string levelValidator; };
struct CTSPCompressedReplay            { sf::base::U64 loginToken; compressed_replay_file compressedReplayFile; };
struct CTSPRequestServerStatus         { sf::base::U64 loginToken; };
struct CTSPReady                       { sf::base::U64 loginToken; };
// clang-format on

#define SSVOH_IDENTITY(x) x
#define SSVOH_COMMA()     ,

#define SSVOH_CTS_PACKETS_X(x, c)                                                                                      \
    x(CTSPHeartbeat) c() x(CTSPDisconnect) c() x(CTSPPublicKey) c() x(CTSPRegister) c() x(CTSPLogin) c() x(CTSPLogout) \
        c() x(CTSPDeleteAccount) c() x(CTSPRequestTopScores) c() x(CTSPReplay) c() x(CTSPRequestOwnScore) c()          \
            x(CTSPRequestTopScoresAndOwnScore) c() x(CTSPStartedGame) c() x(CTSPCompressedReplay) c()                  \
                x(CTSPRequestServerStatus) c() x(CTSPReady)

#define SSVOH_CTS_PACKETS SSVOH_CTS_PACKETS_X(SSVOH_IDENTITY, SSVOH_COMMA)

using PVClientToServer = sf::base::Variant<PInvalid, PEncryptedMsg, SSVOH_CTS_PACKETS>;

// ----------------------------------------------------------------------------

template <typename T>
void makeClientToServerPacket(sf::Packet& p, const T& data);

template <typename T>
[[nodiscard]] bool makeClientToServerEncryptedPacket(const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data);

[[nodiscard]] PVClientToServer decodeClientToServerPacket(const SodiumReceiveKeyArray* keyReceive,
                                                          sf::OutStringStream&         errorOss,
                                                          sf::Packet&                  p);

// ----------------------------------------------------------------------------

// clang-format off
struct STCPKick                   { };
struct STCPPublicKey              { SodiumPublicKeyArray key; };
struct STCPRegistrationSuccess    { };
struct STCPRegistrationFailure    { std::string error; };
struct STCPLoginSuccess           { sf::base::U64 loginToken; std::string loginName; };
struct STCPLoginFailure           { std::string error; };
struct STCPLogoutSuccess          { };
struct STCPLogoutFailure          { };
struct STCPDeleteAccountSuccess   { };
struct STCPDeleteAccountFailure   { std::string error; };
struct STCPTopScores              { std::string levelValidator; sf::base::Vector<Database::ProcessedScore> scores; };
struct STCPOwnScore               { std::string levelValidator; Database::ProcessedScore score; };
struct STCPTopScoresAndOwnScore   { std::string levelValidator; sf::base::Vector<Database::ProcessedScore> scores; sf::base::Optional<Database::ProcessedScore> ownScore; };
struct STCPServerStatus           { ProtocolVersion protocolVersion; GameVersion gameVersion; sf::base::Vector<std::string> supportedLevelValidators; };
// clang-format on

#define SSVOH_STC_PACKETS_X(x, c)                                                                                    \
    x(STCPKick) c() x(STCPPublicKey) c() x(STCPRegistrationSuccess) c() x(STCPRegistrationFailure) c()               \
        x(STCPLoginSuccess) c() x(STCPLoginFailure) c() x(STCPLogoutSuccess) c() x(STCPLogoutFailure) c()            \
            x(STCPDeleteAccountSuccess) c() x(STCPDeleteAccountFailure) c() x(STCPTopScores) c() x(STCPOwnScore) c() \
                x(STCPTopScoresAndOwnScore) c() x(STCPServerStatus)

#define SSVOH_STC_PACKETS SSVOH_STC_PACKETS_X(SSVOH_IDENTITY, SSVOH_COMMA)

using PVServerToClient = sf::base::Variant<PInvalid, PEncryptedMsg, SSVOH_STC_PACKETS>;

// ----------------------------------------------------------------------------

template <typename T>
void makeServerToClientPacket(sf::Packet& p, const T& data);

template <typename T>
[[nodiscard]] bool makeServerToClientEncryptedPacket(const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data);

[[nodiscard]] PVServerToClient decodeServerToClientPacket(const SodiumReceiveKeyArray* keyReceive,
                                                          sf::OutStringStream&         errorOss,
                                                          sf::Packet&                  p);

} // namespace hg
