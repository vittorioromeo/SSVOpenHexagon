// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Shared.hpp"

#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"

#include <SFML/Network/Packet.hpp>

#include <sodium.h>

#include <cstdint>
#include <sstream>
#include <iostream>
#include <optional>
#include <type_traits>

namespace hg
{

namespace
{

template <typename...>
struct TypeList
{
};

template <typename T, typename... Ts>
[[nodiscard]] constexpr bool contains(TypeList<Ts...>)
{
    return (std::is_same_v<T, Ts> || ...);
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr std::size_t indexOfType(TypeList<Ts...>)
{
    static_assert(contains<T>(TypeList<Ts...>{}));

    constexpr std::array test{std::is_same_v<T, Ts>...};
    for(std::size_t i = 0; i < test.size(); ++i)
    {
        if(test[i])
        {
            return i;
        }
    }

    throw;
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr bool variantContains(TypeList<std::variant<Ts...>>)
{
    return contains<T>(TypeList<Ts...>{});
}

template <typename T, typename... Ts>
[[nodiscard]] constexpr std::size_t indexOfVariantType(
    TypeList<std::variant<Ts...>>)
{
    return indexOfType<T>(TypeList<Ts...>{});
}

using PacketType = sf::Uint8;

template <typename T>
[[nodiscard]] constexpr PacketType getPacketType()
{
    if constexpr(variantContains<T>(TypeList<PVClientToServer>{}))
    {
        return indexOfVariantType<T>(TypeList<PVClientToServer>{});
    }
    else if constexpr(variantContains<T>(TypeList<PVServerToClient>{}))
    {
        return indexOfVariantType<T>(TypeList<PVServerToClient>{});
    }
    else
    {
        throw;
    }
}

static constexpr sf::Uint8 preamble1stByte{'o'};
static constexpr sf::Uint8 preamble2ndByte{'h'};

void encodePreamble(sf::Packet& p)
{
    p << static_cast<sf::Uint8>(preamble1stByte)
      << static_cast<sf::Uint8>(preamble2ndByte);
}

void encodeVersion(sf::Packet& p)
{
    p << static_cast<sf::Uint8>(GAME_VERSION.major)
      << static_cast<sf::Uint8>(GAME_VERSION.minor)
      << static_cast<sf::Uint8>(GAME_VERSION.micro);
}

void clearPacketAndEncodePreambleAndVersion(sf::Packet& p)
{
    p.clear();

    encodePreamble(p);
    encodeVersion(p);
}

template <typename T>
void encodePacketType(sf::Packet& p, const T&)
{
    p << static_cast<sf::Uint8>(getPacketType<T>());
}

template <typename T, typename = void>
struct Extractor
{
    [[nodiscard]] static bool extractInto(
        T& target, std::ostringstream& errorOss, sf::Packet& p)
    {
        if(!(p >> target))
        {
            errorOss << "Error extracting single object\n";
            return false;
        }

        return true;
    }
};

template <typename T, std::size_t N>
struct Extractor<std::array<T, N>>
{
    using Type = std::array<T, N>;

    [[nodiscard]] static bool extractInto(
        Type& result, std::ostringstream& errorOss, sf::Packet& p)
    {
        for(std::size_t i = 0; i < N; ++i)
        {
            if(p >> result[i])
            {
                continue;
            }

            errorOss << "Error extracting array element at index '" << i
                     << "'\n";

            return false;
        }

        return true;
    }
};

template <typename T>
[[nodiscard]] bool extractInto(
    T& target, std::ostringstream& errorOss, sf::Packet& p)
{
    return Extractor<T>::extractInto(target, errorOss, p);
}

template <typename T>
[[nodiscard]] std::optional<T> extract(
    std::ostringstream& errorOss, sf::Packet& p)
{
    T temp;

    if(!Extractor<T>::extractInto(temp, errorOss, p))
    {
        return std::nullopt;
    }

    return {std::move(temp)};
}

template <typename T>
[[nodiscard]] auto makeMatcher(std::ostringstream& errorOss, sf::Packet& p)
{
    return [&](const char* name, const T& expected) -> bool {
        T temp;

        if(!extractInto<T>(temp, errorOss, p))
        {
            errorOss << "Error extracting " << name << '\n';
            return false;
        }

        if(temp != expected)
        {
            errorOss << "Error, " << name << " has value '" << temp
                     << ", which doesn't match expected value '" << expected
                     << "'\n";

            return false;
        }

        return true;
    };
}

template <typename T>
[[nodiscard]] auto makeExtractor(std::ostringstream& errorOss, sf::Packet& p)
{
    return [&](const char* name) -> std::optional<T> {
        T temp;

        if(!extractInto<T>(temp, errorOss, p))
        {
            errorOss << "Error extracting " << name << '\n';
            return std::nullopt;
        }

        return {std::move(temp)};
    };
}

[[nodiscard]] bool verifyReceivedPacketPreambleAndVersion(
    std::ostringstream& errorOss, sf::Packet& p)
{
    const auto matchByte = makeMatcher<sf::Uint8>(errorOss, p);

    return matchByte("preamble 1st byte", preamble1stByte) &&
           matchByte("preamble 2nd byte", preamble2ndByte) &&
           matchByte("major version", GAME_VERSION.major) &&
           matchByte("minor version", GAME_VERSION.minor) &&
           matchByte("micro version", GAME_VERSION.micro);
}

[[nodiscard]] std::optional<PacketType> extractPacketType(
    std::ostringstream& errorOss, sf::Packet& p)
{
    const std::optional<sf::Uint8> extracted =
        makeExtractor<sf::Uint8>(errorOss, p)("packet type");

    if(!extracted.has_value())
    {
        return std::nullopt;
    }

    return {static_cast<PacketType>(*extracted)};
}

template <std::size_t NExpected, typename T, std::size_t N>
void encodeArray(sf::Packet& p, const std::array<T, N>& arr)
{
    static_assert(NExpected == N);

    for(std::size_t i = 0; i < arr.size(); ++i)
    {
        p << arr[i];
    }
}

template <typename T>
void encodeDynamicRange(sf::Packet& p, const T* data, const std::size_t len)
{
    SSVOH_ASSERT(data != nullptr);

    for(std::size_t i = 0; i < len; ++i)
    {
        p << data[i];
    }
}

template <typename T>
void encodeFirstNVectorElements(
    sf::Packet& p, const std::vector<T>& data, const std::size_t len)
{
    SSVOH_ASSERT(data.size() >= len);

    for(std::size_t i = 0; i < len; ++i)
    {
        p << data[i];
    }
}

std::vector<sf::Uint8>& getStaticMessageBuffer()
{
    thread_local std::vector<sf::Uint8> result;
    return result;
}

std::vector<sf::Uint8>& getStaticCiphertextBuffer()
{
    thread_local std::vector<sf::Uint8> result;
    return result;
}

sf::Packet& getStaticPacketBuffer()
{
    thread_local sf::Packet result;
    return result;
}

void encodeOHPacket(sf::Packet& p, const CTSPHeartbeat& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const CTSPDisconnect& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const CTSPPublicKey& data)
{
    encodePacketType(p, data);
    encodeArray<sodiumPublicKeyBytes>(p, data.key);
}

void encodeOHPacket(sf::Packet& p, const CTSPReady& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const CTSPPrint& data)
{
    encodePacketType(p, data);
    p << data.msg;
}

void encodeOHPacket(sf::Packet& p, const CTSPRegister& data)
{
    encodePacketType(p, data);
    p << data.steamId;
    p << data.name;
    p << data.passwordHash;
}

void encodeOHPacket(sf::Packet& p, const CTSPLogin& data)
{
    encodePacketType(p, data);
    p << data.steamId;
    p << data.name;
    p << data.passwordHash;
}

void encodeOHPacket(sf::Packet& p, const PEncryptedMsg& data)
{
    encodePacketType(p, data);

    encodeArray<sodiumNonceBytes>(p, data.nonce);
    p << data.messageLength;
    p << data.ciphertextLength;
    encodeFirstNVectorElements(p, *data.ciphertext, data.ciphertextLength);
}

void encodeOHPacket(sf::Packet& p, const STCPKick& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const STCPPublicKey& data)
{
    encodePacketType(p, data);

    for(std::size_t i = 0; i < sodiumPublicKeyBytes; ++i)
    {
        p << data.key[i];
    }
}

void encodeOHPacket(sf::Packet& p, const STCPRegistrationSuccess& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const STCPRegistrationFailure& data)
{
    encodePacketType(p, data);
    p << data.error;
}

void encodeOHPacket(sf::Packet& p, const STCPLoginSuccess& data)
{
    encodePacketType(p, data);
}

void encodeOHPacket(sf::Packet& p, const STCPLoginFailure& data)
{
    encodePacketType(p, data);
    p << data.error;
}

[[nodiscard]] bool decryptPacket(std::ostringstream& errorOss, sf::Packet& p,
    const SodiumReceiveKeyArray& keyReceive, sf::Packet& decryptedPacket)
{
    SodiumNonceArray nonce;
    if(!extractInto(nonce, errorOss, p))
    {
        errorOss << "Error decoding client nonce\n";
        return false;
    }

    sf::Uint64 messageLength;
    if(!extractInto(messageLength, errorOss, p))
    {
        errorOss << "Error decoding client message length\n";
        return false;
    }

    sf::Uint64 ciphertextLength;
    if(!extractInto(ciphertextLength, errorOss, p))
    {
        errorOss << "Error decoding client ciphertext length\n";
        return false;
    }

    std::vector<sf::Uint8>& ciphertext = getStaticCiphertextBuffer();
    ciphertext.resize(ciphertextLength);

    for(std::size_t i = 0; i < ciphertextLength; ++i)
    {
        if(p >> ciphertext[i])
        {
            continue;
        }

        errorOss << "Error decoding client ciphertext at index '" << i << "'\n";

        return false;
    }

    std::vector<sf::Uint8>& message = getStaticMessageBuffer();
    message.resize(messageLength);

    if(crypto_secretbox_open_easy(message.data(), ciphertext.data(),
           ciphertextLength, nonce.data(), keyReceive.data()) != 0)
    {
        errorOss << "Failure decrypting encrypted client message\n";
        return false;
    }

    decryptedPacket.clear();
    decryptedPacket.append(message.data(), messageLength);

    return true;
}

} // namespace

// ----------------------------------------------------------------------------

template <typename T>
void makeClientToServerPacket(sf::Packet& p, const T& data)
{
    clearPacketAndEncodePreambleAndVersion(p);
    encodeOHPacket(p, data);
}

template void makeClientToServerPacket(sf::Packet&, const PEncryptedMsg&);

#define INSTANTIATE_MAKE_CTS(mIdx, mData, mArg) \
    template void makeClientToServerPacket(sf::Packet&, const mArg&);

VRM_PP_FOREACH_REVERSE(
    INSTANTIATE_MAKE_CTS, VRM_PP_EMPTY(), VRM_PP_TPL_EXPLODE(SSVOH_CTS_PACKETS))

#undef INSTANTIATE_MAKE_CTS

// ----------------------------------------------------------------------------

template <typename T>
[[nodiscard]] bool makeClientToServerEncryptedPacket(
    const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data)
{
    sf::Packet& packetToEncrypt = getStaticPacketBuffer();
    packetToEncrypt.clear();

    encodeOHPacket(packetToEncrypt, data);

    PEncryptedMsg encryptedMsg{
        .nonce = generateNonce(),
        .messageLength = packetToEncrypt.getDataSize(),
        .ciphertextLength = getCiphertextLength(packetToEncrypt.getDataSize())
        //
    };

    encryptedMsg.ciphertext = &getStaticCiphertextBuffer();
    encryptedMsg.ciphertext->resize(encryptedMsg.ciphertextLength);

    if(crypto_secretbox_easy(encryptedMsg.ciphertext->data(),
           static_cast<const sf::Uint8*>(packetToEncrypt.getData()),
           encryptedMsg.messageLength, encryptedMsg.nonce.data(),
           keyTransmit.data()) != 0)
    {
        return false;
    }

    makeClientToServerPacket(p, encryptedMsg);
    return true;
}

#define INSTANTIATE_MAKE_CTS_ENCRYPTED(mIdx, mData, mArg) \
    template bool makeClientToServerEncryptedPacket(      \
        const SodiumTransmitKeyArray&, sf::Packet&, const mArg&);

VRM_PP_FOREACH_REVERSE(INSTANTIATE_MAKE_CTS_ENCRYPTED, VRM_PP_EMPTY(),
    VRM_PP_TPL_EXPLODE(SSVOH_CTS_PACKETS))

#undef INSTANTIATE_MAKE_CTS_ENCRYPTED

// ----------------------------------------------------------------------------

#define HANDLE_EMPTY_PACKET(type)        \
    do                                   \
    {                                    \
        if(*pt == getPacketType<type>()) \
        {                                \
            return {type{}};             \
        }                                \
    } while(false)

#define EXTRACT_OR_FAIL(target, info)                   \
    do                                                  \
    {                                                   \
        if(!extractInto(target, errorOss, p))           \
        {                                               \
            errorOss << "Error decoding " info "\n";    \
            return {PInvalid{.error = errorOss.str()}}; \
        }                                               \
    } while(false)

[[nodiscard]] static PVClientToServer
    decodeClientToServerPacketInner(const SodiumReceiveKeyArray* keyReceive,
        std::ostringstream& errorOss, sf::Packet& p)
{
    const std::optional<PacketType> pt = extractPacketType(errorOss, p);

    if(!pt.has_value())
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    if(*pt == getPacketType<PEncryptedMsg>())
    {
        if(keyReceive == nullptr)
        {
            errorOss << "Cannot decode client encrypted message without "
                        "receive key\n";

            return {PInvalid{.error = errorOss.str()}};
        }

        sf::Packet& decryptedPacket = getStaticPacketBuffer();
        if(!decryptPacket(errorOss, p, *keyReceive, decryptedPacket))
        {
            return {PInvalid{.error = errorOss.str()}};
        }

        return decodeClientToServerPacketInner(
            keyReceive, errorOss, decryptedPacket);
    }

    HANDLE_EMPTY_PACKET(CTSPHeartbeat);
    HANDLE_EMPTY_PACKET(CTSPDisconnect);

    if(*pt == getPacketType<CTSPPublicKey>())
    {
        CTSPPublicKey result;
        EXTRACT_OR_FAIL(result.key, "client public key");
        return {result};
    }

    HANDLE_EMPTY_PACKET(CTSPReady);

    if(*pt == getPacketType<CTSPPrint>())
    {
        CTSPPrint result;
        EXTRACT_OR_FAIL(result.msg, "client print message");
        return {result};
    }

    if(*pt == getPacketType<CTSPRegister>())
    {
        CTSPRegister result;
        EXTRACT_OR_FAIL(result.steamId, "register steam id");
        EXTRACT_OR_FAIL(result.name, "register name");
        EXTRACT_OR_FAIL(result.passwordHash, "register password hash");
        return {result};
    }

    if(*pt == getPacketType<CTSPLogin>())
    {
        CTSPLogin result;
        EXTRACT_OR_FAIL(result.steamId, "login steam id");
        EXTRACT_OR_FAIL(result.name, "login name");
        EXTRACT_OR_FAIL(result.passwordHash, "login password hash");
        return {result};
    }

    errorOss << "Unknown packet type '" << static_cast<int>(*pt) << "'\n";
    return {PInvalid{.error = errorOss.str()}};
}

[[nodiscard]] PVClientToServer decodeClientToServerPacket(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p)
{
    if(!verifyReceivedPacketPreambleAndVersion(errorOss, p))
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    return decodeClientToServerPacketInner(keyReceive, errorOss, p);
}

// ----------------------------------------------------------------------------

template <typename T>
void makeServerToClientPacket(sf::Packet& p, const T& data)
{
    clearPacketAndEncodePreambleAndVersion(p);
    encodeOHPacket(p, data);
}

template void makeServerToClientPacket(sf::Packet&, const PEncryptedMsg&);

#define INSTANTIATE_MAKE_STC(mIdx, mData, mArg) \
    template void makeServerToClientPacket(sf::Packet&, const mArg&);

VRM_PP_FOREACH_REVERSE(
    INSTANTIATE_MAKE_STC, VRM_PP_EMPTY(), VRM_PP_TPL_EXPLODE(SSVOH_STC_PACKETS))

#undef INSTANTIATE_MAKE_STC

// ----------------------------------------------------------------------------

template <typename T>
[[nodiscard]] bool makeServerToClientEncryptedPacket(
    const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data)
{
    sf::Packet& packetToEncrypt = getStaticPacketBuffer();
    packetToEncrypt.clear();

    encodeOHPacket(packetToEncrypt, data);

    PEncryptedMsg encryptedMsg{
        .nonce = generateNonce(),
        .messageLength = packetToEncrypt.getDataSize(),
        .ciphertextLength = getCiphertextLength(packetToEncrypt.getDataSize())
        //
    };

    encryptedMsg.ciphertext = &getStaticCiphertextBuffer();
    encryptedMsg.ciphertext->resize(encryptedMsg.ciphertextLength);

    if(crypto_secretbox_easy(encryptedMsg.ciphertext->data(),
           static_cast<const sf::Uint8*>(packetToEncrypt.getData()),
           encryptedMsg.messageLength, encryptedMsg.nonce.data(),
           keyTransmit.data()) != 0)
    {
        return false;
    }

    makeServerToClientPacket(p, encryptedMsg);
    return true;
}

#define INSTANTIATE_MAKE_STC_ENCRYPTED(mIdx, mData, mArg) \
    template bool makeServerToClientEncryptedPacket(      \
        const SodiumTransmitKeyArray&, sf::Packet&, const mArg&);

VRM_PP_FOREACH_REVERSE(INSTANTIATE_MAKE_STC_ENCRYPTED, VRM_PP_EMPTY(),
    VRM_PP_TPL_EXPLODE(SSVOH_STC_PACKETS))

#undef INSTANTIATE_MAKE_STC_ENCRYPTED

// ----------------------------------------------------------------------------

[[nodiscard]] static PVServerToClient
    decodeServerToClientPacketInner(const SodiumReceiveKeyArray* keyReceive,
        std::ostringstream& errorOss, sf::Packet& p)
{
    const std::optional<PacketType> pt = extractPacketType(errorOss, p);

    if(!pt.has_value())
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    if(*pt == getPacketType<PEncryptedMsg>())
    {
        if(keyReceive == nullptr)
        {
            errorOss << "Cannot decode client encrypted message without "
                        "receive key\n";

            return {PInvalid{.error = errorOss.str()}};
        }

        sf::Packet& decryptedPacket = getStaticPacketBuffer();
        if(!decryptPacket(errorOss, p, *keyReceive, decryptedPacket))
        {
            return {PInvalid{.error = errorOss.str()}};
        }

        return decodeServerToClientPacketInner(
            keyReceive, errorOss, decryptedPacket);
    }

    HANDLE_EMPTY_PACKET(STCPKick);

    if(*pt == getPacketType<STCPPublicKey>())
    {
        STCPPublicKey result;
        EXTRACT_OR_FAIL(result.key, "server public key");
        return {result};
    }

    HANDLE_EMPTY_PACKET(STCPRegistrationSuccess);

    if(*pt == getPacketType<STCPRegistrationFailure>())
    {
        STCPRegistrationFailure result;
        EXTRACT_OR_FAIL(result.error, "registration failure error");
        return {result};
    }

    HANDLE_EMPTY_PACKET(STCPLoginSuccess);

    if(*pt == getPacketType<STCPLoginFailure>())
    {
        STCPLoginFailure result;
        EXTRACT_OR_FAIL(result.error, "login failure error");
        return {result};
    }

    errorOss << "Unknown packet type '" << static_cast<int>(*pt) << "'\n";
    return {PInvalid{.error = errorOss.str()}};
}

[[nodiscard]] PVServerToClient decodeServerToClientPacket(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p)
{
    if(!verifyReceivedPacketPreambleAndVersion(errorOss, p))
    {
        return {PInvalid{.error = errorOss.str()}};
    }

    return decodeServerToClientPacketInner(keyReceive, errorOss, p);
}

} // namespace hg
