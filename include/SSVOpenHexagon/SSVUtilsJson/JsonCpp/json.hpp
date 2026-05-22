// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0
// Fork of: https://jsoncpp.sourceforge.net/

#ifndef SSVUJ_OH_JSONCPP_JSON
#define SSVUJ_OH_JSONCPP_JSON

// Include as system header to suppress dependency warnings.
#pragma GCC system_header

#include "SFML/Base/Fmt/FmtSinkRef.hpp"
#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#define JSON_HAS_INT64

namespace Json
{
inline bool in(char c, char c1, char c2, char c3, char c4) noexcept
{
    return c == c1 || c == c2 || c == c3 || c == c4;
}
inline bool in(char c, char c1, char c2, char c3, char c4, char c5) noexcept
{
    return c == c1 || c == c2 || c == c3 || c == c4 || c == c5;
}
inline bool isControlCharacter(char ch) noexcept
{
    return ch > 0 && ch <= 0x1F;
}

using UInt        = unsigned int;
using Int64       = long long int;
using UInt64      = unsigned long long int;
using LargestInt  = Int64;
using LargestUInt = UInt64;
using ArrayIndex  = unsigned int;

class FastWriter;
class StyledWriter;
class Reader;
struct Features;
class StaticString;
class Path;
class PathArgument;
class Value;
class ValueIteratorBase;
class ValueIterator;
class ValueConstIterator;

// Opaque types defined in jsoncpp.cpp.
class ObjectValuesImpl;
struct ObjectValuesIteratorImpl;

struct Features
{
    bool allowComments_{true}, strictRoot_{false};
    inline Features() = default;
    inline Features(bool mAllowComments, bool mStrictRoot) : allowComments_{mAllowComments}, strictRoot_{mStrictRoot}
    {
    }
    static inline Features all() noexcept
    {
        return {};
    }
    static inline Features strictMode() noexcept
    {
        return {false, true};
    }
};

enum ValueType
{
    nullValue = 0,
    intValue,
    uintValue,
    realValue,
    stringValue,
    booleanValue,
    arrayValue,
    objectValue
};
enum CommentPlacement
{
    commentBefore = 0,
    commentAfterOnSameLine,
    commentAfter,
    numberOfCommentPlacement
};
class StaticString
{
private:
    const char* str_;

public:
    inline explicit StaticString(const char* czstring) : str_(czstring)
    {
    }
    inline operator const char*() const noexcept
    {
        return str_;
    }
    inline const char* c_str() const noexcept
    {
        return str_;
    }
};

class Value
{
    friend class ValueIteratorBase;

public:
    using Members        = sf::base::Vector<sf::base::String>;
    using iterator       = ValueIterator;
    using const_iterator = ValueConstIterator;
    using ObjectValues   = ObjectValuesImpl;

    static constexpr LargestInt  minLargestInt{LargestInt(~(LargestUInt(-1) / 2))};
    static constexpr LargestInt  maxLargestInt{LargestInt(LargestUInt(-1) / 2)};
    static constexpr LargestUInt maxLargestUInt{LargestUInt(-1)};
    static constexpr int         minInt{int(~(UInt(-1) / 2))};
    static constexpr int         maxInt{int(UInt(-1) / 2)};
    static constexpr UInt        maxUInt{UInt(-1)};
#ifdef JSON_HAS_INT64
    static constexpr Int64  minInt64{Int64(~(UInt64(-1) / 2))};
    static constexpr Int64  maxInt64{Int64(UInt64(-1) / 2)};
    static constexpr UInt64 maxUInt64{UInt64(-1)};
#endif

public:
    class CZString
    {
    public:
        enum DuplicationPolicy
        {
            noDuplication = 0,
            duplicate,
            duplicateOnCopy
        };
        CZString(ArrayIndex index);
        CZString(const char* cstr, DuplicationPolicy allocate);
        CZString(const CZString& other);
        CZString(CZString&& other) noexcept;
        ~CZString();
        CZString&   operator=(const CZString& other);
        CZString&   operator=(CZString&& other) noexcept;
        bool        operator<(const CZString& other) const;
        bool        operator==(const CZString& other) const;
        ArrayIndex  index() const;
        const char* c_str() const;
        bool        isStaticString() const;

    private:
        void        swap(CZString& other);
        const char* cstr_{nullptr};
        ArrayIndex  index_;
    };

public:
    Value(ValueType type = nullValue);
    Value(int value);
    Value(UInt value);
#ifdef JSON_HAS_INT64
    Value(Int64 value);
    Value(UInt64 value);
#endif
    Value(double value);
    Value(const char* value);
    Value(const char* beginValue, const char* endValue);
    Value(const StaticString& value);
    Value(const sf::base::String& value);
    Value(bool value);
    Value(const Value& other);
    ~Value();
    inline Value& operator=(const Value& other)
    {
        Value temp(other);
        swap(temp);
        return *this;
    }
    inline ValueType type() const noexcept
    {
        return type_;
    }
    inline int compare(const Value& other) const
    {
        if (*this < other)
            return -1;
        if (*this > other)
            return 1;
        return 0;
    }
    void swap(Value& other);

    bool        operator<(const Value& other) const;
    inline bool operator<=(const Value& other) const
    {
        return !(other < *this);
    }
    inline bool operator>=(const Value& other) const
    {
        return !(*this < other);
    }
    inline bool operator>(const Value& other) const
    {
        return other < *this;
    }
    bool             operator==(const Value& other) const;
    bool             operator!=(const Value& other) const;
    const char*      asCString() const;
    sf::base::String asString() const;
    int              asInt() const;
    UInt             asUInt() const;
#ifdef JSON_HAS_INT64
    Int64  asInt64() const;
    UInt64 asUInt64() const;
#endif
    LargestInt   asLargestInt() const;
    LargestUInt  asLargestUInt() const;
    float        asFloat() const;
    double       asDouble() const;
    bool         asBool() const;
    bool         isNull() const;
    bool         isBool() const;
    bool         isInt() const;
    bool         isInt64() const;
    bool         isUInt() const;
    bool         isUInt64() const;
    bool         isIntegral() const;
    bool         isDouble() const;
    bool         isNumeric() const;
    bool         isString() const;
    bool         isArray() const;
    bool         isObject() const;
    bool         isConvertibleTo(ValueType other) const;
    ArrayIndex   size() const;
    bool         empty() const;
    bool         operator!() const;
    void         clear();
    void         resize(ArrayIndex size);
    Value&       operator[](ArrayIndex index);
    Value&       operator[](int index);
    const Value& operator[](ArrayIndex index) const;
    const Value& operator[](int index) const;
    Value        get(ArrayIndex index, const Value& defaultValue) const;
    bool         isValidIndex(ArrayIndex index) const;
    Value&       append(const Value& value);
    Value&       operator[](const char* key);
    const Value& operator[](const char* key) const;
    Value&       operator[](const sf::base::String& key);
    const Value& operator[](const sf::base::String& key) const;
    Value&       operator[](const StaticString& key);

    Value get(const char* key, const Value& defaultValue) const;
    Value get(const sf::base::String& key, const Value& defaultValue) const;

    Value removeMember(const char* key);
    Value removeMember(const sf::base::String& key);
    bool  isMember(const char* key) const;
    bool  isMember(const sf::base::String& key) const;

    Members          getMemberNames() const;
    void             setComment(const char* comment, CommentPlacement placement);
    void             setComment(const sf::base::String& comment, CommentPlacement placement);
    bool             hasComment(CommentPlacement placement) const;
    sf::base::String getComment(CommentPlacement placement) const;
    sf::base::String toStyledString() const;
    const_iterator   begin() const;
    const_iterator   end() const;
    iterator         begin();
    iterator         end();

private:
    Value& resolveReference(const char* key, bool isStatic);
    struct CommentInfo
    {
        inline CommentInfo() = default;
        ~CommentInfo();
        void  setComment(const char* text);
        char* comment_{nullptr};
    };
    union ValueHolder
    {
        LargestInt        int_;
        LargestUInt       uint_;
        double            real_;
        bool              bool_;
        char*             string_;
        ObjectValuesImpl* map_;
    } value_;
    ValueType    type_      : 8;
    bool         allocated_ : 1;
    CommentInfo* comments_{nullptr};
};

class PathArgument
{
private:
    enum Kind
    {
        kindNone = 0,
        kindIndex,
        kindKey
    };
    sf::base::String key_;
    ArrayIndex       index_;
    Kind             kind_;

public:
    friend class Path;
    PathArgument();
    PathArgument(ArrayIndex index);
    PathArgument(const char* key);
    PathArgument(const sf::base::String& key);
};

class Path
{
private:
    using InArgs = sf::base::Vector<const PathArgument*>;
    using Args   = sf::base::Vector<PathArgument>;
    void                                          makePath(const sf::base::String& path, const InArgs& in);
    void addPathInArg(const sf::base::String& path, const InArgs& in, InArgs::const_iterator& itInArg, PathArgument::Kind kind);
    void invalidPath(const sf::base::String& path, int location);
    Args args_;

public:
    Path(const sf::base::String& path,
         const PathArgument&     a1 = PathArgument(),
         const PathArgument&     a2 = PathArgument(),
         const PathArgument&     a3 = PathArgument(),
         const PathArgument&     a4 = PathArgument(),
         const PathArgument&     a5 = PathArgument());
    const Value& resolve(const Value& root) const;
    Value        resolve(const Value& root, const Value& defaultValue) const;
    Value&       make(Value& root) const;
};

class ValueIteratorBase
{
public:
    using size_t          = unsigned int;
    using difference_type = int;
    using SelfType        = ValueIteratorBase;

protected:
    // Buffer is large enough to hold the underlying segmented_map iterator
    // (ankerl::unordered_dense segmented_map iterator is 16 bytes; 32 is a safe upper bound).
    sf::base::InPlacePImpl<ObjectValuesIteratorImpl, 32> current_;
    bool                                                 isNull_;

protected:
    Value&          deref() const;
    void            increment();
    void            decrement();
    difference_type computeDistance(const SelfType& other) const;
    bool            isEqual(const SelfType& other) const;
    void            copy(const SelfType& other);

public:
    ValueIteratorBase();
    explicit ValueIteratorBase(const ObjectValuesIteratorImpl& current);
    ValueIteratorBase(const ValueIteratorBase& other);
    ValueIteratorBase(ValueIteratorBase&& other) noexcept;
    ValueIteratorBase& operator=(const ValueIteratorBase& other);
    ValueIteratorBase& operator=(ValueIteratorBase&& other) noexcept;
    ~ValueIteratorBase();

    inline bool operator==(const SelfType& other) const
    {
        return isEqual(other);
    }
    inline bool operator!=(const SelfType& other) const
    {
        return !isEqual(other);
    }
    inline difference_type operator-(const SelfType& other) const
    {
        return computeDistance(other);
    }
    Value       key() const;
    UInt        index() const;
    const char* memberName() const;
};

class ValueConstIterator final : public ValueIteratorBase
{
    friend class Value;

private:
    explicit ValueConstIterator(const ObjectValuesIteratorImpl& current);

public:
    using size_t          = unsigned int;
    using difference_type = int;
    using reference       = const Value&;
    using pointer         = const Value*;
    using SelfType        = ValueConstIterator;
    ValueConstIterator();
    SelfType&       operator=(const ValueIteratorBase& other);
    inline SelfType operator++(int)
    {
        SelfType temp(*this);
        ++*this;
        return temp;
    }
    inline SelfType operator--(int)
    {
        SelfType temp(*this);
        --*this;
        return temp;
    }
    inline SelfType& operator--()
    {
        decrement();
        return *this;
    }
    inline SelfType& operator++()
    {
        increment();
        return *this;
    }
    inline reference operator*() const
    {
        return deref();
    }
};
class ValueIterator final : public ValueIteratorBase
{
    friend class Value;

private:
    explicit ValueIterator(const ObjectValuesIteratorImpl& current);

public:
    using size_t          = unsigned int;
    using difference_type = int;
    using reference       = Value&;
    using pointer         = Value*;
    using SelfType        = ValueIterator;
    ValueIterator();
    ValueIterator(const ValueConstIterator& other);
    ValueIterator(const ValueIterator& other);
    SelfType&       operator=(const SelfType& other);
    inline SelfType operator++(int)
    {
        SelfType temp(*this);
        ++*this;
        return temp;
    }
    inline SelfType operator--(int)
    {
        SelfType temp(*this);
        --*this;
        return temp;
    }
    inline SelfType& operator--()
    {
        decrement();
        return *this;
    }
    inline SelfType& operator++()
    {
        increment();
        return *this;
    }
    inline reference operator*() const
    {
        return deref();
    }
};

class Reader
{
public:
    using Location = const char*;

    Reader();
    explicit Reader(const Features& features);
    ~Reader();

    Reader(const Reader&)            = delete;
    Reader& operator=(const Reader&) = delete;
    Reader(Reader&&) noexcept;
    Reader& operator=(Reader&&) noexcept;

    bool             parse(const sf::base::String& document, Value& root, bool collectComments = true);
    bool             parse(const char* beginDoc, const char* endDoc, Value& root, bool collectComments = true);
    sf::base::String getFormattedErrorMessages() const;

    struct Impl;

private:
    sf::base::InPlacePImpl<Impl, 256> impl_;
};

struct Writer
{
    virtual ~Writer();
    /// \brief Serialize `root` directly into `sink` (streaming).
    virtual void write(const Value& root, sf::base::FmtSinkRef sink) = 0;
};
class FastWriter final : public Writer
{
public:
    FastWriter();
    virtual ~FastWriter()
    {
    }
    void         enableYAMLCompatibility();
    void         dropNullPlaceholders();
    virtual void write(const Value& root, sf::base::FmtSinkRef sink) override;

private:
    void                 writeValue(const Value& value);
    sf::base::FmtSinkRef sink_; // overwritten in `write()` before any emission
    bool                 yamlCompatiblityEnabled_;
    bool                 dropNullPlaceholders_;
};
class StyledWriter final : public Writer
{
public:
    StyledWriter();
    virtual ~StyledWriter()
    {
    }
    virtual void write(const Value& root, sf::base::FmtSinkRef sink) override;

private:
    void                                      writeValue(const Value& value);
    void                                      writeArrayValue(const Value& value);
    bool                                      isMultineArray(const Value& value);
    void                                      pushValue(const sf::base::String& value);
    void                                      writeIndent();
    void                                      writeWithIndent(const sf::base::String& value);
    void                                      indent();
    void                                      unindent();
    void                                      writeCommentBeforeValue(const Value& root);
    void                                      writeCommentAfterValueOnSameLine(const Value& root);
    bool                                      hasCommentForValue(const Value& value);
    static sf::base::String                   normalizeEOL(const sf::base::String& text);

    // Emit `[data, n)` directly into the active sink (or accumulate into the
    // in-progress child string if `addChildValues_` is set). Tracks the last
    // emitted byte so `writeIndent` can decide whether a newline is needed
    // without peeking back into the sink (which would defeat streaming).
    void emit(const char* data, sf::base::SizeT n);
    void emit(sf::base::StringView s);
    void emit(char c);

    using ChildValues = sf::base::Vector<sf::base::String>;
    ChildValues          childValues_;
    sf::base::String     pendingChild_;             // accumulator used while `addChildValues_` is true
    sf::base::String     indentString_;
    sf::base::FmtSinkRef sink_;                     // overwritten in `write()` before any emission
    char                 lastEmitted_ = '\0';       // last byte written to the sink ('\0' means "nothing yet")
    int                  rightMargin_;
    int                  indentSize_;
    bool                 addChildValues_;
};

inline const Value nullJsonValue{};

#ifdef JSON_HAS_INT64
sf::base::String valueToString(int value);
sf::base::String valueToString(UInt value);
#endif
sf::base::String valueToString(LargestInt value);
sf::base::String valueToString(LargestUInt value);
sf::base::String valueToString(double value);
sf::base::String valueToString(bool value);
sf::base::String valueToQuotedString(const char* value);

} // namespace Json

#endif
