// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Shared.hpp"

#include "SSVOpenHexagon/Online/Sodium.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/ProtocolVersion.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"

#include <SFML/Config.hpp>
#include <SFML/Network/Packet.hpp>

#include <sodium.h>

#include <boost/pfr.hpp>

#include <cstdint>
#include <sstream>
#include <iostream>
#include <optional>
#include <type_traits>

namespace hg {

namespace {

template <typename...>
struct TypeList
{};

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

void encodeProtocolVersion(sf::Packet& p)
{
    p << static_cast<sf::Uint8>(PROTOCOL_VERSION);
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
    encodeProtocolVersion(p);
    encodeVersion(p);
}

template <typename T>
void encodePacketType(sf::Packet& p, const T&)
{
    p << static_cast<sf::Uint8>(getPacketType<T>());
}

template <typename T>
[[nodiscard]] bool extractInto(
    T& target, std::ostringstream& errorOss, sf::Packet& p);

template <typename T, typename = void>
struct Extractor
{
    template <typename U = T>
    [[nodiscard]] static auto doExtractIntoImpl(
        U& target, std::ostringstream& errorOss, sf::Packet& p, int)
        -> decltype((p >> target), bool())
    {
        if(!(p >> target))
        {
            errorOss << "Error extracting single object\n";
            return false;
        }

        return true;
    }

    template <typename U = T>
    [[nodiscard]] static bool doExtractIntoImpl(
        U& target, std::ostringstream& errorOss, sf::Packet& p, long)
    {
        bool result = true;

        if constexpr((boost::pfr::tuple_size_v<T>) > 0)
        {
            boost::pfr::for_each_field(target,
                [&](auto& nestedField)
                {
                    if(!extractInto(nestedField, errorOss, p))
                    {
                        result = false;
                    }
                });
        }

        return result;
    }

    [[nodiscard]] static bool doExtractInto(
        T& target, std::ostringstream& errorOss, sf::Packet& p)
    {
        return doExtractIntoImpl(target, errorOss, p, 0);
    }
};

template <typename T, std::size_t N>
struct Extractor<std::array<T, N>>
{
    using Type = std::array<T, N>;

    [[nodiscard]] static bool doExtractInto(
        Type& result, std::ostringstream& errorOss, sf::Packet& p)
    {
        for(std::size_t i = 0; i < N; ++i)
        {
            if(extractInto(result[i], errorOss, p))
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
struct Extractor<std::vector<T>>
{
    using Type = std::vector<T>;

    [[nodiscard]] static bool doExtractInto(
        Type& result, std::ostringstream& errorOss, sf::Packet& p)
    {
        sf::Uint64 size;
        if(!(p >> size))
        {
            errorOss << "Error extracting vector size\n";
            return false;
        }

        result.resize(size);

        for(std::size_t i = 0; i < size; ++i)
        {
            if(extractInto(result[i], errorOss, p))
            {
                continue;
            }

            errorOss << "Error extracting vector element at index '" << i
                     << "'\n";

            return false;
        }

        return true;
    }
};

template <typename T>
struct Extractor<std::optional<T>>
{
    using Type = std::optional<T>;

    [[nodiscard]] static bool doExtractInto(
        Type& result, std::ostringstream& errorOss, sf::Packet& p)
    {
        bool set;
        if(!(p >> set))
        {
            errorOss << "Error extracting optional set flag\n";
            return false;
        }

        if(!set)
        {
            result.reset();
            return true;
        }

        result.emplace();

        if(!extractInto(*result, errorOss, p))
        {
            errorOss << "Error extracting optional element\n";
            return false;
        }

        return true;
    }
};

template <>
struct Extractor<hg::replay_file>
{
    using Type = hg::replay_file;

    [[nodiscard]] static bool doExtractInto(
        Type& result, std::ostringstream& errorOss, sf::Packet& p)
    {
        if(!result.deserialize_from_packet(p))
        {
            errorOss << "Error deserializing replay\n";
            return false;
        }

        return true;
    }
};

template <>
struct Extractor<hg::compressed_replay_file>
{
    using Type = hg::compressed_replay_file;

    [[nodiscard]] static bool doExtractInto(
        Type& result, std::ostringstream& errorOss, sf::Packet& p)
    {
        if(!result.deserialize_from_packet(p))
        {
            errorOss << "Error deserializing compressed replay\n";
            return false;
        }

        return true;
    }
};

template <>
struct Extractor<hg::GameVersion>
{
    using Type = hg::GameVersion;

    [[nodiscard]] static bool doExtractInto(
        Type& result, std::ostringstream& errorOss, sf::Packet& p)
    {
        sf::Int32 major;
        if(!(p >> major))
        {
            errorOss << "Error deserializing major version\n";
            return false;
        }

        sf::Int32 minor;
        if(!(p >> minor))
        {
            errorOss << "Error deserializing minor version\n";
            return false;
        }

        sf::Int32 micro;
        if(!(p >> micro))
        {
            errorOss << "Error deserializing micro version\n";
            return false;
        }

        result.major = major;
        result.minor = minor;
        result.micro = micro;

        return true;
    }
};

template <typename T>
[[nodiscard]] bool extractInto(
    T& target, std::ostringstream& errorOss, sf::Packet& p)
{
    return Extractor<T>::doExtractInto(target, errorOss, p);
}

template <typename T>
[[nodiscard]] std::optional<T> extract(
    std::ostringstream& errorOss, sf::Packet& p)
{
    T temp;

    if(!extractInto(temp, errorOss, p))
    {
        return std::nullopt;
    }

    return {std::move(temp)};
}

template <typename T>
[[nodiscard]] auto makeAlwaysTrueMatcher(
    std::ostringstream& errorOss, sf::Packet& p)
{
    return [&](const char* name) -> bool
    {
        T temp;

        if(!extractInto<T>(temp, errorOss, p))
        {
            errorOss << "Error extracting " << name << '\n';
            return false;
        }

        return true;
    };
}

class AdvancedMatcher
{
private:
    std::ostringstream& _errorOss;
    sf::Packet& _p;

public:
    [[nodiscard]] explicit AdvancedMatcher(
        std::ostringstream& errorOss, sf::Packet& p)
        : _errorOss{errorOss}, _p{p}
    {}

    template <typename T>
    [[nodiscard]] bool extractIntoOrPrintError(const char* name, T& target)
    {
        if(!extractInto<T>(target, _errorOss, _p))
        {
            _errorOss << "Error extracting " << name << '\n';
            return false;
        }

        return true;
    }

    template <typename T>
    [[nodiscard]] bool matchOrPrintError(
        const char* name, T& target, const T& expected)
    {
        if(!extractIntoOrPrintError(name, target))
        {
            return false;
        }

        if(target != expected)
        {
            _errorOss << "Error, " << name << " has value '" << target
                      << ", which doesn't match expected value '" << expected
                      << "'\n";

            return false;
        }

        return true;
    }

    template <typename T>
    [[nodiscard]] std::optional<T> extractOrPrintError(const char* name)
    {
        T temp;

        if(!extractIntoOrPrintError(name, temp))
        {
            return std::nullopt;
        }

        return {std::move(temp)};
    }
};

template <typename T>
[[nodiscard]] auto makeMatcher(std::ostringstream& errorOss, sf::Packet& p)
{
    return [&](const char* name, const T& expected) -> bool
    {
        T temp;

        return AdvancedMatcher{errorOss, p}.matchOrPrintError<T>(
            name, temp, expected);
    };
}

template <typename T>
[[nodiscard]] auto makeExtractor(std::ostringstream& errorOss, sf::Packet& p)
{
    return [&](const char* name) -> std::optional<T> {
        return AdvancedMatcher{errorOss, p}.extractOrPrintError<T>(name);
    };
}

[[nodiscard]] bool verifyReceivedPacketPreambleAndVersion(
    std::ostringstream& errorOss, sf::Packet& p)
{
    // TODO (P0): should use a protocol version instead of the game version here

    const auto matchByte = makeMatcher<sf::Uint8>(errorOss, p);
    const auto matchByteAlwaysTrue =
        makeAlwaysTrueMatcher<sf::Uint8>(errorOss, p);

    const bool preambleMatches =
        matchByte("preamble 1st byte", preamble1stByte) &&
        matchByte("preamble 2nd byte", preamble2ndByte);

    if(!preambleMatches)
    {
        return false;
    }

    AdvancedMatcher matcher{errorOss, p};

    const std::optional<sf::Uint8> afterPreamble0 =
        matcher.extractOrPrintError<sf::Uint8>("byte0 after preamble");

    const std::optional<sf::Uint8> afterPreamble1 =
        matcher.extractOrPrintError<sf::Uint8>("byte1 after preamble");

    const std::optional<sf::Uint8> afterPreamble2 =
        matcher.extractOrPrintError<sf::Uint8>("byte2 after preamble");

    if(!afterPreamble0 || !afterPreamble1 || !afterPreamble2)
    {
        errorOss << "extraction of after-preamble bytes failed\n";
        return false;
    }

    // TODO (P0): this doesn't work probably because the server rejects the
    // client's preamble

    // 2.0.6 backward compatibility
    if(afterPreamble0 == sf::Uint8{2} &&  //
        afterPreamble1 == sf::Uint8{0} && //
        afterPreamble2 == sf::Uint8{6})
    {
        errorOss << "connected to 2.0.6 (old) server\n";
        return true;
    }

    SSVOH_ASSERT(afterPreamble0.has_value());
    const ProtocolVersion extractedProtocolVersion = afterPreamble0.value();

    if(extractedProtocolVersion != PROTOCOL_VERSION)
    {
        errorOss << "protocol version mismatch\n";
        return false;
    }

    return true;
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

template <typename TData, typename TField>
auto encodeField(sf::Packet& p, const TData& data, const TField& field);

template <typename TData, typename TField>
void encodeFieldImpl(
    sf::Packet& p, const TData& data, const TField& field, long)
{
    if constexpr(boost::pfr::tuple_size_v < TField >> 0)
    {
        boost::pfr::for_each_field(field, [&](const auto& nestedField)
            { encodeField(p, data, nestedField); });
    }
}

template <typename TData, typename TField>
auto encodeFieldImpl(sf::Packet& p, const TData&, const TField& field, int)
    -> decltype((p << field), void())
{
    p << field;
}

template <typename TData, typename TField>
auto encodeField(sf::Packet& p, const TData& data, const TField& field)
{
    encodeFieldImpl(p, data, field, 0);
}

template <typename TData, typename T, std::size_t N>
void encodeField(sf::Packet& p, const TData& data, const std::array<T, N>& arr)
{
    for(std::size_t i = 0; i < arr.size(); ++i)
    {
        encodeField(p, data, arr[i]);
    }
}

template <typename TData, typename T>
void encodeField(sf::Packet& p, const TData& data, const std::vector<T>& vec)
{
    encodeField(p, data, static_cast<sf::Uint64>(vec.size()));

    for(const T& x : vec)
    {
        encodeField(p, data, x);
    }
}

template <typename TData, typename T>
void encodeField(sf::Packet& p, const TData& data, const std::optional<T>& opt)
{
    encodeField(p, data, opt.has_value());

    if(opt.has_value())
    {
        encodeField(p, data, *opt);
    }
}

template <typename TData>
void encodeField(sf::Packet& p, const TData& data, const hg::replay_file& rf)
{
    (void)data;
    (void)rf.serialize_to_packet(p);
}

template <typename TData>
void encodeField(
    sf::Packet& p, const TData& data, const hg::compressed_replay_file& crf)
{
    (void)data;
    (void)crf.serialize_to_packet(p);
}

template <typename TData>
void encodeField(sf::Packet& p, const TData& data, const hg::GameVersion& gv)
{
    (void)data;
    p << static_cast<sf::Int32>(gv.major) << static_cast<sf::Int32>(gv.minor)
      << static_cast<sf::Int32>(gv.micro);
}

template <typename TData>
void encodeField(
    sf::Packet& p, const TData& data, const Impl::CiphertextVectorPtr& field)
{
    encodeFirstNVectorElements(p, *field.ptr, data.ciphertextLength);
}

template <typename T>
void encodeOHPacket(sf::Packet& p, const T& data)
{
    encodePacketType(p, data);

    if constexpr(boost::pfr::tuple_size_v < T >> 0)
    {
        boost::pfr::for_each_field(
            data, [&](const auto& field) { encodeField(p, data, field); });
    }
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

template <typename F, typename T>
[[nodiscard]] bool makeEncryptedPacketImpl(F&& f,
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

    encryptedMsg.ciphertext.ptr = &getStaticCiphertextBuffer();
    encryptedMsg.ciphertext.ptr->resize(encryptedMsg.ciphertextLength);

    if(crypto_secretbox_easy(encryptedMsg.ciphertext.ptr->data(),
           static_cast<const sf::Uint8*>(packetToEncrypt.getData()),
           encryptedMsg.messageLength, encryptedMsg.nonce.data(),
           keyTransmit.data()) != 0)
    {
        return false;
    }

    f(p, encryptedMsg);
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

// ----------------------------------------------------------------------------

template <typename T>
[[nodiscard]] bool makeClientToServerEncryptedPacket(
    const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data)
{
    return makeEncryptedPacketImpl([](auto&&... xs)
        { makeClientToServerPacket(std::forward<decltype(xs)>(xs)...); },
        keyTransmit, p, data);
}

#define INSTANTIATE_MAKE_CTS_ENCRYPTED(mIdx, mData, mArg) \
    template bool makeClientToServerEncryptedPacket(      \
        const SodiumTransmitKeyArray&, sf::Packet&, const mArg&);

VRM_PP_FOREACH_REVERSE(INSTANTIATE_MAKE_CTS_ENCRYPTED, VRM_PP_EMPTY(),
    VRM_PP_TPL_EXPLODE(SSVOH_CTS_PACKETS))

// ----------------------------------------------------------------------------

#define HANDLE_PACKET(type)                                 \
    do                                                      \
    {                                                       \
        if(*pt == getPacketType<type>())                    \
        {                                                   \
            type result;                                    \
                                                            \
            if(!extractAllMembers(result))                  \
            {                                               \
                return {PInvalid{.error = errorOss.str()}}; \
            }                                               \
                                                            \
            return {result};                                \
        }                                                   \
    }                                                       \
    while(false)

#define INJECT_COMMON_PACKET_HANDLING_CODE(function)                     \
    const std::optional<PacketType> pt = extractPacketType(errorOss, p); \
                                                                         \
    if(!pt.has_value())                                                  \
    {                                                                    \
        return {PInvalid{.error = errorOss.str()}};                      \
    }                                                                    \
                                                                         \
    if(*pt == getPacketType<PEncryptedMsg>())                            \
    {                                                                    \
        if(!decodeEncryptedPacket(keyReceive, errorOss, p))              \
        {                                                                \
            return {PInvalid{.error = errorOss.str()}};                  \
        }                                                                \
                                                                         \
        return function(keyReceive, errorOss, getStaticPacketBuffer());  \
    }                                                                    \
                                                                         \
    const auto extractAllMembers = makeExtractAllMembers(errorOss, p)


#define FORIMPL_HANDLE_PACKET(mIdx, mData, mArg) HANDLE_PACKET(mArg);

// ----------------------------------------------------------------------------

static auto makeExtractAllMembers(std::ostringstream& errorOss, sf::Packet& p)
{
    return [&]<typename T>(T& target)
    {
        bool success = true;

        if constexpr((boost::pfr::tuple_size_v<T>) > 0)
        {
            boost::pfr::for_each_field(target,
                [&](auto& field, std::size_t i)
                {
                    if(!extractInto(field, errorOss, p))
                    {
                        errorOss << "Error decoding field #" << i << " \n";
                        success = false;
                    }
                });
        }

        return success;
    };
}

[[nodiscard]] static bool decodeEncryptedPacket(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p)
{
    if(keyReceive == nullptr)
    {
        errorOss << "Cannot decode encrypted message without receive key\n";
        return false;
    }

    if(!decryptPacket(errorOss, p, *keyReceive, getStaticPacketBuffer()))
    {
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------

[[nodiscard]] static PVClientToServer decodeClientToServerPacketInner(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p)
{
    INJECT_COMMON_PACKET_HANDLING_CODE(decodeClientToServerPacketInner);

    VRM_PP_FOREACH_REVERSE(FORIMPL_HANDLE_PACKET, VRM_PP_EMPTY(),
        VRM_PP_TPL_EXPLODE(SSVOH_CTS_PACKETS))

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

// ----------------------------------------------------------------------------

template <typename T>
[[nodiscard]] bool makeServerToClientEncryptedPacket(
    const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data)
{
    return makeEncryptedPacketImpl([](auto&&... xs)
        { makeServerToClientPacket(std::forward<decltype(xs)>(xs)...); },
        keyTransmit, p, data);
}

#define INSTANTIATE_MAKE_STC_ENCRYPTED(mIdx, mData, mArg) \
    template bool makeServerToClientEncryptedPacket(      \
        const SodiumTransmitKeyArray&, sf::Packet&, const mArg&);

VRM_PP_FOREACH_REVERSE(INSTANTIATE_MAKE_STC_ENCRYPTED, VRM_PP_EMPTY(),
    VRM_PP_TPL_EXPLODE(SSVOH_STC_PACKETS))

// ----------------------------------------------------------------------------

[[nodiscard]] static PVServerToClient decodeServerToClientPacketInner(
    const SodiumReceiveKeyArray* keyReceive, std::ostringstream& errorOss,
    sf::Packet& p)
{
    INJECT_COMMON_PACKET_HANDLING_CODE(decodeServerToClientPacketInner);

    VRM_PP_FOREACH_REVERSE(FORIMPL_HANDLE_PACKET, VRM_PP_EMPTY(),
        VRM_PP_TPL_EXPLODE(SSVOH_STC_PACKETS))

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
