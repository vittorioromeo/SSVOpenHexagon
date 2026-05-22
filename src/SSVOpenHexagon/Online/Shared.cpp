// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/Replay.hpp"
#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/ProtocolVersion.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Online/Shared.hpp"
#include "SSVOpenHexagon/Online/Sodium.hpp"
#include "sodium/crypto_secretbox.h"

#include "SFML/Network/Packet.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/Array.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtAppendMixin.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp" // IWYU pragma: keep -- fmtTo formats numeric args
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MiniPFR.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/TypePackIndex.hpp"
#include "SFML/Base/Variant.hpp"
#include "SFML/Base/Vector.hpp"

#include <sodium.h>

namespace hg
{

namespace
{

template <typename...>
struct TypeList
{
};

template <typename T, typename... Ts>
[[nodiscard]] consteval bool variantContains(TypeList<sf::base::Variant<Ts...>>)
{
    return (SFML_BASE_IS_SAME(T, Ts) || ...);
}

template <typename T, typename... Ts>
[[nodiscard]] consteval sf::base::SizeT indexOfVariantType(TypeList<sf::base::Variant<Ts...>>)
{
    return sf::base::getTypePackIndex<T, Ts...>();
}

using PacketType = sf::base::U8;

template <typename T>
[[nodiscard]] constexpr PacketType getPacketType()
{
    if constexpr (variantContains<T>(TypeList<PVClientToServer>{}))
    {
        return indexOfVariantType<T>(TypeList<PVClientToServer>{});
    }
    else if constexpr (variantContains<T>(TypeList<PVServerToClient>{}))
    {
        return indexOfVariantType<T>(TypeList<PVServerToClient>{});
    }
    else
    {
        throw;
    }
}

static constexpr sf::base::U8 preamble1stByte{'o'};
static constexpr sf::base::U8 preamble2ndByte{'h'};

void encodePreamble(sf::Packet& p)
{
    p << static_cast<sf::base::U8>(preamble1stByte) << static_cast<sf::base::U8>(preamble2ndByte);
}

void encodeProtocolVersion(sf::Packet& p)
{
    p << static_cast<sf::base::U8>(PROTOCOL_VERSION);
}

void encodeVersion(sf::Packet& p)
{
    p << static_cast<sf::base::U8>(GAME_VERSION.major) << static_cast<sf::base::U8>(GAME_VERSION.minor)
      << static_cast<sf::base::U8>(GAME_VERSION.micro);
}

void clearPacketAndEncodePreambleAndProtocolVersionAndGameVersion(sf::Packet& p)
{
    p.clear();

    encodePreamble(p);
    encodeProtocolVersion(p);
    encodeVersion(p);
}

template <typename T>
void encodePacketType(sf::Packet& p, const T&)
{
    p << static_cast<sf::base::U8>(getPacketType<T>());
}

template <typename T>
[[nodiscard]] bool extractInto(T& target, sf::base::String& errorOss, sf::Packet& p);

template <typename T, typename = void>
struct Extractor
{
    template <typename U = T>
    [[nodiscard]] static auto doExtractIntoImpl(U& target, sf::base::String& errorOss, sf::Packet& p, int)
        -> decltype((p >> target), bool())
    {
        if (!(p >> target))
        {
            errorOss += "Error extracting single object\n";
            return false;
        }

        return true;
    }

    template <typename U = T>
    [[nodiscard]] static bool doExtractIntoImpl(U& target, sf::base::String& errorOss, sf::Packet& p, long)
    {
        bool result = true;

        if constexpr (sf::base::minipfr::numFields<T> > 0)
        {
            sf::base::minipfr::forEachField(target,
                                            [&](auto& nestedField)
            {
                if (!extractInto(nestedField, errorOss, p))
                {
                    result = false;
                }
            });
        }

        return result;
    }

    [[nodiscard]] static bool doExtractInto(T& target, sf::base::String& errorOss, sf::Packet& p)
    {
        return doExtractIntoImpl(target, errorOss, p, 0);
    }
};

template <typename T, sf::base::SizeT N>
struct Extractor<sf::base::Array<T, N>>
{
    using Type = sf::base::Array<T, N>;

    [[nodiscard]] static bool doExtractInto(Type& result, sf::base::String& errorOss, sf::Packet& p)
    {
        for (sf::base::SizeT i = 0; i < N; ++i)
        {
            if (extractInto(result[i], errorOss, p))
            {
                continue;
            }

            errorOss.appendFmt("Error extracting array element at index '{}'\n", i);

            return false;
        }

        return true;
    }
};


// Mirror of the `encodeField(... const sf::base::String&)` overload above.
template <>
struct Extractor<sf::base::String>
{
    [[nodiscard]] static bool doExtractInto(sf::base::String& result, sf::base::String& errorOss, sf::Packet& p)
    {
        if (!(p >> result))
        {
            errorOss += "Error extracting sf::base::String\n";
            return false;
        }
        return true;
    }
};

template <typename T>
struct Extractor<sf::base::Vector<T>>
{
    using Type = sf::base::Vector<T>;

    [[nodiscard]] static bool doExtractInto(Type& result, sf::base::String& errorOss, sf::Packet& p)
    {
        sf::base::U64 size;
        if (!(p >> size))
        {
            errorOss += "Error extracting vector size\n";
            return false;
        }

        result.resize(size);

        for (sf::base::SizeT i = 0; i < size; ++i)
        {
            if (extractInto(result[i], errorOss, p))
            {
                continue;
            }

            errorOss.appendFmt("Error extracting vector element at index '{}'\n", i);

            return false;
        }

        return true;
    }
};

template <typename T>
struct Extractor<sf::base::Optional<T>>
{
    using Type = sf::base::Optional<T>;

    [[nodiscard]] static bool doExtractInto(Type& result, sf::base::String& errorOss, sf::Packet& p)
    {
        bool set;
        if (!(p >> set))
        {
            errorOss += "Error extracting optional set flag\n";
            return false;
        }

        if (!set)
        {
            result.reset();
            return true;
        }

        result.emplace();

        if (!extractInto(*result, errorOss, p))
        {
            errorOss += "Error extracting optional element\n";
            return false;
        }

        return true;
    }
};

template <>
struct Extractor<hg::replay_file>
{
    using Type = hg::replay_file;

    [[nodiscard]] static bool doExtractInto(Type& result, sf::base::String& errorOss, sf::Packet& p)
    {
        if (!result.deserialize_from_packet(p))
        {
            errorOss += "Error deserializing replay\n";
            return false;
        }

        return true;
    }
};

template <>
struct Extractor<hg::compressed_replay_file>
{
    using Type = hg::compressed_replay_file;

    [[nodiscard]] static bool doExtractInto(Type& result, sf::base::String& errorOss, sf::Packet& p)
    {
        if (!result.deserialize_from_packet(p))
        {
            errorOss += "Error deserializing compressed replay\n";
            return false;
        }

        return true;
    }
};

template <>
struct Extractor<hg::GameVersion>
{
    using Type = hg::GameVersion;

    [[nodiscard]] static bool doExtractInto(Type& result, sf::base::String& errorOss, sf::Packet& p)
    {
        sf::base::I32 major;
        if (!(p >> major))
        {
            errorOss += "Error deserializing major version\n";
            return false;
        }

        sf::base::I32 minor;
        if (!(p >> minor))
        {
            errorOss += "Error deserializing minor version\n";
            return false;
        }

        sf::base::I32 micro;
        if (!(p >> micro))
        {
            errorOss += "Error deserializing micro version\n";
            return false;
        }

        result.major = major;
        result.minor = minor;
        result.micro = micro;

        return true;
    }
};

template <typename T>
[[nodiscard]] bool extractInto(T& target, sf::base::String& errorOss, sf::Packet& p)
{
    return Extractor<T>::doExtractInto(target, errorOss, p);
}

template <typename T>
[[nodiscard]] sf::base::Optional<T> extract(sf::base::String& errorOss, sf::Packet& p)
{
    T temp;

    if (!extractInto(temp, errorOss, p))
    {
        return sf::base::nullOpt;
    }

    return sf::base::makeOptional<T>(SFML_BASE_MOVE(temp));
}

template <typename T>
[[nodiscard]] auto makeAlwaysTrueMatcher(sf::base::String& errorOss, sf::Packet& p)
{
    return [&](const char* name) -> bool
    {
        T temp;

        if (!extractInto<T>(temp, errorOss, p))
        {
            errorOss.appendFmt("Error extracting {}\n", name);
            return false;
        }

        return true;
    };
}

class AdvancedMatcher
{
private:
    sf::base::String& _errorOss;
    sf::Packet&          _p;

public:
    [[nodiscard]] explicit AdvancedMatcher(sf::base::String& errorOss, sf::Packet& p) : _errorOss{errorOss}, _p{p}
    {
    }

    template <typename T>
    [[nodiscard]] bool extractIntoOrPrintError(const char* name, T& target)
    {
        if (!extractInto<T>(target, _errorOss, _p))
        {
            _errorOss.appendFmt("Error extracting {}\n", name);
            return false;
        }

        return true;
    }

    template <typename T>
    [[nodiscard]] bool extractAndMatchOrPrintError(const char* name, T& target, const T& expected)
    {
        if (!extractIntoOrPrintError(name, target))
        {
            return false;
        }

        if (target != expected)
        {
            _errorOss.appendFmt("Error, {} has value '{}, which doesn't match expected value '{}'\n",
                                name,
                                target,
                                expected);

            return false;
        }

        return true;
    }

    template <typename T>
    [[nodiscard]] bool skipOrPrintError(const char* name)
    {
        T temp;
        return extractIntoOrPrintError(name, temp);
    }

    template <typename T>
    [[nodiscard]] sf::base::Optional<T> extractOrPrintError(const char* name)
    {
        T temp;

        if (!extractIntoOrPrintError(name, temp))
        {
            return sf::base::nullOpt;
        }

        return sf::base::makeOptional<T>(SFML_BASE_MOVE(temp));
    }

    template <typename T>
    [[nodiscard]] bool matchOrPrintError(const char* name, const T& expected)
    {
        T temp;
        return extractAndMatchOrPrintError(name, temp, expected);
    }
};

template <typename T>
[[nodiscard]] auto makeMatcher(sf::base::String& errorOss, sf::Packet& p)
{
    return [&](const char* name, const T& expected) -> bool
    { return AdvancedMatcher{errorOss, p}.matchOrPrintError<T>(name, expected); };
}

template <typename T>
[[nodiscard]] auto makeExtractor(sf::base::String& errorOss, sf::Packet& p)
{
    return [&](const char* name) -> sf::base::Optional<T>
    { return AdvancedMatcher{errorOss, p}.extractOrPrintError<T>(name); };
}

[[nodiscard]] bool verifyReceivedPacketPreambleAndProtocolVersionAndGameVersion(sf::base::String& errorOss, sf::Packet& p)
{
    AdvancedMatcher m{errorOss, p};

    return
        // Preamble bytes and protocol version must match.
        m.matchOrPrintError<sf::base::U8>("preamble 1st byte", preamble1stByte) &&
        m.matchOrPrintError<sf::base::U8>("preamble 2st byte", preamble2ndByte) &&
        m.matchOrPrintError<sf::base::U8>("protocol version", PROTOCOL_VERSION) &&

        // Game version is currently ignored.
        m.skipOrPrintError<sf::base::U8>("major version") && m.skipOrPrintError<sf::base::U8>("minor version") &&
        m.skipOrPrintError<sf::base::U8>("micro version");
}

[[nodiscard]] sf::base::Optional<PacketType> extractPacketType(sf::base::String& errorOss, sf::Packet& p)
{
    const sf::base::Optional<sf::base::U8> extracted = makeExtractor<sf::base::U8>(errorOss, p)("packet type");

    if (!extracted.hasValue())
    {
        return sf::base::nullOpt;
    }

    return sf::base::makeOptional(static_cast<PacketType>(*extracted));
}

template <typename T>
void encodeFirstNVectorElements(sf::Packet& p, const sf::base::Vector<T>& data, const sf::base::SizeT len)
{
    SSVOH_ASSERT(data.size() >= len);

    for (sf::base::SizeT i = 0; i < len; ++i)
    {
        p << data[i];
    }
}

sf::base::Vector<sf::base::U8>& getStaticMessageBuffer()
{
    thread_local sf::base::Vector<sf::base::U8> result;
    return result;
}

sf::base::Vector<sf::base::U8>& getStaticCiphertextBuffer()
{
    thread_local sf::base::Vector<sf::base::U8> result;
    return result;
}

sf::Packet& getStaticPacketBuffer()
{
    thread_local sf::Packet result;
    return result;
}

template <typename TData, typename TField>
auto encodeField(sf::Packet& p, const TData& data, const TField& field);

// Forward declaration so that recursive calls through the generic template
// can find this overload during phase-2 lookup. Required because
// `sf::base::String` would otherwise be picked up by the generic
// `encodeFieldImpl(... long)` and forwarded to `minipfr::tieAsTuple`, which
// fails since `String` isn't an aggregate.
template <typename TData>
void encodeField(sf::Packet& p, const TData& data, const sf::base::String& s);

template <typename TData, typename TField>
void encodeFieldImpl(sf::Packet& p, const TData& data, const TField& field, long)
{
    if constexpr (sf::base::minipfr::numFields<TField> > 0)
    {
        sf::base::minipfr::forEachField(field, [&](const auto& nestedField) { encodeField(p, data, nestedField); });
    }
}

template <typename TData, typename TField>
auto encodeFieldImpl(sf::Packet& p, const TData&, const TField& field, int) -> decltype((p << field), void())
{
    p << field;
}

template <typename TData, typename TField>
auto encodeField(sf::Packet& p, const TData& data, const TField& field)
{
    encodeFieldImpl(p, data, field, 0);
}

template <typename TData, typename T, sf::base::SizeT N>
void encodeField(sf::Packet& p, const TData& data, const sf::base::Array<T, N>& arr)
{
    for (sf::base::SizeT i = 0; i < arr.size(); ++i)
    {
        encodeField(p, data, arr[i]);
    }
}

template <typename TData, typename T>
void encodeField(sf::Packet& p, const TData& data, const sf::base::Vector<T>& vec)
{
    encodeField(p, data, static_cast<sf::base::U64>(vec.size()));

    for (const T& x : vec)
    {
        encodeField(p, data, x);
    }
}

template <typename TData, typename T>
void encodeField(sf::Packet& p, const TData& data, const sf::base::Optional<T>& opt)
{
    encodeField(p, data, opt.hasValue());

    if (opt.hasValue())
    {
        encodeField(p, data, *opt);
    }
}

template <typename TData>
void encodeField(sf::Packet& p, const TData& data, const sf::base::String& s)
{
    (void)data;
    p << s;
}

template <typename TData>
void encodeField(sf::Packet& p, const TData& data, const hg::replay_file& rf)
{
    (void)data;
    (void)rf.serialize_to_packet(p);
}

template <typename TData>
void encodeField(sf::Packet& p, const TData& data, const hg::compressed_replay_file& crf)
{
    (void)data;
    (void)crf.serialize_to_packet(p);
}

template <typename TData>
void encodeField(sf::Packet& p, const TData& data, const hg::GameVersion& gv)
{
    (void)data;
    p << static_cast<sf::base::I32>(gv.major) << static_cast<sf::base::I32>(gv.minor)
      << static_cast<sf::base::I32>(gv.micro);
}

template <typename TData>
void encodeField(sf::Packet& p, const TData& data, const Impl::CiphertextVectorPtr& field)
{
    encodeFirstNVectorElements(p, *field.ptr, data.ciphertextLength);
}

template <typename T>
void encodeOHPacket(sf::Packet& p, const T& data)
{
    encodePacketType(p, data);

    if constexpr (sf::base::minipfr::numFields<T> > 0)
    {
        sf::base::minipfr::forEachField(data, [&](const auto& field) { encodeField(p, data, field); });
    }
}

[[nodiscard]] bool decryptPacket(sf::base::String&         errorOss,
                                 sf::Packet&                  p,
                                 const SodiumReceiveKeyArray& keyReceive,
                                 sf::Packet&                  decryptedPacket)
{
    SodiumNonceArray nonce;
    if (!extractInto(nonce, errorOss, p))
    {
        errorOss += "Error decoding client nonce\n";
        return false;
    }

    sf::base::U64 messageLength;
    if (!extractInto(messageLength, errorOss, p))
    {
        errorOss += "Error decoding client message length\n";
        return false;
    }

    sf::base::U64 ciphertextLength;
    if (!extractInto(ciphertextLength, errorOss, p))
    {
        errorOss += "Error decoding client ciphertext length\n";
        return false;
    }

    sf::base::Vector<sf::base::U8>& ciphertext = getStaticCiphertextBuffer();
    ciphertext.resize(ciphertextLength);

    for (sf::base::SizeT i = 0; i < ciphertextLength; ++i)
    {
        if (p >> ciphertext[i])
        {
            continue;
        }

        errorOss.appendFmt("Error decoding client ciphertext at index '{}'\n", i);

        return false;
    }

    sf::base::Vector<sf::base::U8>& message = getStaticMessageBuffer();
    message.resize(messageLength);

    if (crypto_secretbox_open_easy(message.data(), ciphertext.data(), ciphertextLength, nonce.data(), keyReceive.data()) != 0)
    {
        errorOss += "Failure decrypting encrypted client message\n";
        return false;
    }

    decryptedPacket.clear();
    decryptedPacket.append(message.data(), messageLength);

    return true;
}

template <typename F, typename T>
[[nodiscard]] bool makeEncryptedPacketImpl(F&& f, const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data)
{
    sf::Packet& packetToEncrypt = getStaticPacketBuffer();
    packetToEncrypt.clear();

    encodeOHPacket(packetToEncrypt, data);

    PEncryptedMsg encryptedMsg{
        .nonce            = generateNonce(),
        .messageLength    = packetToEncrypt.getDataSize(),
        .ciphertextLength = getCiphertextLength(packetToEncrypt.getDataSize())
        //
    };

    encryptedMsg.ciphertext.ptr = &getStaticCiphertextBuffer();
    encryptedMsg.ciphertext.ptr->resize(encryptedMsg.ciphertextLength);

    if (crypto_secretbox_easy(encryptedMsg.ciphertext.ptr->data(),
                              static_cast<const sf::base::U8*>(packetToEncrypt.getData()),
                              encryptedMsg.messageLength,
                              encryptedMsg.nonce.data(),
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
    clearPacketAndEncodePreambleAndProtocolVersionAndGameVersion(p);
    encodeOHPacket(p, data);
}

template void makeClientToServerPacket(sf::Packet&, const PEncryptedMsg&);

#define INSTANTIATE_MAKE_CTS(mArg) template void makeClientToServerPacket(sf::Packet&, const mArg&);

#define NOTHING()

SSVOH_CTS_PACKETS_X(INSTANTIATE_MAKE_CTS, NOTHING)

// ----------------------------------------------------------------------------

template <typename T>
[[nodiscard]] bool makeClientToServerEncryptedPacket(const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data)
{
    return makeEncryptedPacketImpl([](auto&&... xs) {
        makeClientToServerPacket(SFML_BASE_FORWARD(xs)...);
    }, keyTransmit, p, data);
}

#define INSTANTIATE_MAKE_CTS_ENCRYPTED(mArg) \
    template bool makeClientToServerEncryptedPacket(const SodiumTransmitKeyArray&, sf::Packet&, const mArg&);

SSVOH_CTS_PACKETS_X(INSTANTIATE_MAKE_CTS_ENCRYPTED, NOTHING)


// ----------------------------------------------------------------------------

#define HANDLE_PACKET(type)                                                             \
    do                                                                                  \
    {                                                                                   \
        if (*pt == getPacketType<type>())                                               \
        {                                                                               \
            type result;                                                                \
                                                                                        \
            if (!extractAllMembers(result))                                             \
            {                                                                           \
                return VariantType{PInvalid{.error = errorOss}}; \
            }                                                                           \
                                                                                        \
            return VariantType{result};                                                 \
        }                                                                               \
    } while (false)

#define INJECT_COMMON_PACKET_HANDLING_CODE(function)                                \
    const sf::base::Optional<PacketType> pt = extractPacketType(errorOss, p);       \
                                                                                    \
    if (!pt.hasValue())                                                             \
    {                                                                               \
        return VariantType{PInvalid{.error = errorOss}};     \
    }                                                                               \
                                                                                    \
    if (*pt == getPacketType<PEncryptedMsg>())                                      \
    {                                                                               \
        if (!decodeEncryptedPacket(keyReceive, errorOss, p))                        \
        {                                                                           \
            return VariantType{PInvalid{.error = errorOss}}; \
        }                                                                           \
                                                                                    \
        return function(keyReceive, errorOss, getStaticPacketBuffer());             \
    }                                                                               \
                                                                                    \
    const auto extractAllMembers = makeExtractAllMembers(errorOss, p)


#define FORIMPL_HANDLE_PACKET(mIdx, mData, mArg) HANDLE_PACKET(mArg);

// ----------------------------------------------------------------------------

static auto makeExtractAllMembers(sf::base::String& errorOss, sf::Packet& p)
{
    return [&]<typename T>(T& target)
    {
        bool success = true;

        if constexpr (sf::base::minipfr::numFields<T> > 0)
        {
            sf::base::SizeT i = 0;
            sf::base::minipfr::forEachField(target,
                                            [&](auto& field)
            {
                if (!extractInto(field, errorOss, p))
                {
                    errorOss.appendFmt("Error decoding field #{} \n", i);
                    success = false;
                }
                ++i;
            });
        }

        return success;
    };
}

[[nodiscard]] static bool decodeEncryptedPacket(const SodiumReceiveKeyArray* keyReceive,
                                                sf::base::String&         errorOss,
                                                sf::Packet&                  p)
{
    if (keyReceive == nullptr)
    {
        errorOss += "Cannot decode encrypted message without receive key\n";
        return false;
    }

    if (!decryptPacket(errorOss, p, *keyReceive, getStaticPacketBuffer()))
    {
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------

template <typename VariantType, typename... Ts>
VariantType packetHandlerImpl(const SodiumReceiveKeyArray* keyReceive, sf::base::String& errorOss, sf::Packet& p, auto&& func)
{
    const sf::base::Optional<PacketType> pt = extractPacketType(errorOss, p);

    if (!pt.hasValue())
    {
        return VariantType{PInvalid{.error = errorOss}};
    }

    if (*pt == getPacketType<PEncryptedMsg>())
    {
        if (!decodeEncryptedPacket(keyReceive, errorOss, p))
        {
            return VariantType{PInvalid{.error = errorOss}};
        }

        return func(keyReceive, errorOss, getStaticPacketBuffer());
    }

    const auto extractAllMembers = makeExtractAllMembers(errorOss, p);

    VariantType variantResult;
    bool        found = false;

    (...,
     [&]
    {
        if (*pt == getPacketType<Ts>())
        {
            found = true;

            Ts result;

            if (!extractAllMembers(result))
            {
                variantResult = VariantType{PInvalid{.error = errorOss}};
            }

            variantResult = VariantType{result};
        }
    }());

    if (!found)
    {
        errorOss.appendFmt("Unknown packet type '{}'\n", static_cast<int>(*pt));
        return VariantType{PInvalid{.error = errorOss}};
    }

    return variantResult;
}

// ----------------------------------------------------------------------------

[[nodiscard]] static PVClientToServer decodeClientToServerPacketInner(const SodiumReceiveKeyArray* keyReceive,
                                                                      sf::base::String&         errorOss,
                                                                      sf::Packet&                  p)
{
    return packetHandlerImpl<PVClientToServer, SSVOH_CTS_PACKETS>(keyReceive, errorOss, p, decodeClientToServerPacketInner);
}

[[nodiscard]] PVClientToServer decodeClientToServerPacket(const SodiumReceiveKeyArray* keyReceive,
                                                          sf::base::String&         errorOss,
                                                          sf::Packet&                  p)
{
    if (!verifyReceivedPacketPreambleAndProtocolVersionAndGameVersion(errorOss, p))
    {
        return PVClientToServer{PInvalid{.error = errorOss}};
    }

    return decodeClientToServerPacketInner(keyReceive, errorOss, p);
}

// ----------------------------------------------------------------------------

template <typename T>
void makeServerToClientPacket(sf::Packet& p, const T& data)
{
    clearPacketAndEncodePreambleAndProtocolVersionAndGameVersion(p);
    encodeOHPacket(p, data);
}

template void makeServerToClientPacket(sf::Packet&, const PEncryptedMsg&);

#define INSTANTIATE_MAKE_STC(mArg) template void makeServerToClientPacket(sf::Packet&, const mArg&);

SSVOH_STC_PACKETS_X(INSTANTIATE_MAKE_STC, NOTHING)

// ----------------------------------------------------------------------------

template <typename T>
[[nodiscard]] bool makeServerToClientEncryptedPacket(const SodiumTransmitKeyArray& keyTransmit, sf::Packet& p, const T& data)
{
    return makeEncryptedPacketImpl([](auto&&... xs) {
        makeServerToClientPacket(SFML_BASE_FORWARD(xs)...);
    }, keyTransmit, p, data);
}

#define INSTANTIATE_MAKE_STC_ENCRYPTED(mArg) \
    template bool makeServerToClientEncryptedPacket(const SodiumTransmitKeyArray&, sf::Packet&, const mArg&);

SSVOH_STC_PACKETS_X(INSTANTIATE_MAKE_STC_ENCRYPTED, NOTHING)

// ----------------------------------------------------------------------------

[[nodiscard]] static PVServerToClient decodeServerToClientPacketInner(const SodiumReceiveKeyArray* keyReceive,
                                                                      sf::base::String&         errorOss,
                                                                      sf::Packet&                  p)
{
    return packetHandlerImpl<PVServerToClient, SSVOH_STC_PACKETS>(keyReceive, errorOss, p, decodeServerToClientPacketInner);
}

[[nodiscard]] PVServerToClient decodeServerToClientPacket(const SodiumReceiveKeyArray* keyReceive,
                                                          sf::base::String&         errorOss,
                                                          sf::Packet&                  p)
{
    if (!verifyReceivedPacketPreambleAndProtocolVersionAndGameVersion(errorOss, p))
    {
        return PVServerToClient{PInvalid{.error = errorOss}};
    }

    return decodeServerToClientPacketInner(keyReceive, errorOss, p);
}

} // namespace hg
