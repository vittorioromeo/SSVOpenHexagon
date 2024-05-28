// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0
// Fork of: https://jsoncpp.sourceforge.net/

#ifndef SSVUJ_OH_JSONCPP_JSON
#define SSVUJ_OH_JSONCPP_JSON

// Include as system header to suppress dependency warnings.
#pragma GCC system_header

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <deque>
#include <stack>

#define JSON_FAIL_MESSAGE(message) throw std::runtime_error(message);
#define JSON_ASSERT_MESSAGE(condition, message) \
    if (!(condition))                           \
    {                                           \
        JSON_FAIL_MESSAGE(message)              \
    }
#define JSON_HAS_INT64

namespace Json {
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

using UInt = unsigned int;
using Int64 = long long int;
using UInt64 = unsigned long long int;
using LargestInt = Int64;
using LargestUInt = UInt64;
using ArrayIndex = unsigned int;
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
struct Features
{
    bool allowComments_{true}, strictRoot_{false};
    inline Features() = default;
    inline Features(bool mAllowComments, bool mStrictRoot)
        : allowComments_{mAllowComments}, strictRoot_{mStrictRoot}
    {}
    inline static Features all() noexcept
    {
        return {};
    }
    inline static Features strictMode() noexcept
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
    {}
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
    using Members = std::vector<std::string>;
    using iterator = ValueIterator;
    using const_iterator = ValueConstIterator;

    static constexpr LargestInt minLargestInt{
        LargestInt(~(LargestUInt(-1) / 2))};
    static constexpr LargestInt maxLargestInt{LargestInt(LargestUInt(-1) / 2)};
    static constexpr LargestUInt maxLargestUInt{LargestUInt(-1)};
    static constexpr int minInt{int(~(UInt(-1) / 2))};
    static constexpr int maxInt{int(UInt(-1) / 2)};
    static constexpr UInt maxUInt{UInt(-1)};
#ifdef JSON_HAS_INT64
    static constexpr Int64 minInt64{Int64(~(UInt64(-1) / 2))};
    static constexpr Int64 maxInt64{Int64(UInt64(-1) / 2)};
    static constexpr UInt64 maxUInt64{UInt64(-1)};
#endif

private:
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
        ~CZString();
        CZString& operator=(const CZString& other);
        bool operator<(const CZString& other) const;
        bool operator==(const CZString& other) const;
        ArrayIndex index() const;
        const char* c_str() const;
        bool isStaticString() const;

    private:
        void swap(CZString& other);
        const char* cstr_{nullptr};
        ArrayIndex index_;
    };

public:
    using ObjectValues = std::map<CZString, Value>;
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
    Value(const std::string& value);
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
        if (*this < other) return -1;
        if (*this > other) return 1;
        return 0;
    }
    void swap(Value& other);

    bool operator<(const Value& other) const;
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
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    const char* asCString() const;
    std::string asString() const;
    int asInt() const;
    UInt asUInt() const;
#ifdef JSON_HAS_INT64
    Int64 asInt64() const;
    UInt64 asUInt64() const;
#endif
    LargestInt asLargestInt() const;
    LargestUInt asLargestUInt() const;
    float asFloat() const;
    double asDouble() const;
    bool asBool() const;
    bool isNull() const;
    bool isBool() const;
    bool isInt() const;
    bool isInt64() const;
    bool isUInt() const;
    bool isUInt64() const;
    bool isIntegral() const;
    bool isDouble() const;
    bool isNumeric() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;
    bool isConvertibleTo(ValueType other) const;
    ArrayIndex size() const;
    bool empty() const;
    bool operator!() const;
    void clear();
    void resize(ArrayIndex size);
    Value& operator[](ArrayIndex index);
    Value& operator[](int index);
    const Value& operator[](ArrayIndex index) const;
    const Value& operator[](int index) const;
    Value get(ArrayIndex index, const Value& defaultValue) const;
    bool isValidIndex(ArrayIndex index) const;
    Value& append(const Value& value);
    Value& operator[](const char* key);
    const Value& operator[](const char* key) const;
    Value& operator[](const std::string& key);
    const Value& operator[](const std::string& key) const;
    Value& operator[](const StaticString& key);

    Value get(const char* key, const Value& defaultValue) const;
    Value get(const std::string& key, const Value& defaultValue) const;

    Value removeMember(const char* key);
    Value removeMember(const std::string& key);
    bool isMember(const char* key) const;
    bool isMember(const std::string& key) const;

    Members getMemberNames() const;
    void setComment(const char* comment, CommentPlacement placement);
    void setComment(const std::string& comment, CommentPlacement placement);
    bool hasComment(CommentPlacement placement) const;
    std::string getComment(CommentPlacement placement) const;
    std::string toStyledString() const;
    const_iterator begin() const;
    const_iterator end() const;
    iterator begin();
    iterator end();

private:
    Value& resolveReference(const char* key, bool isStatic);
    struct CommentInfo
    {
        inline CommentInfo() = default;
        ~CommentInfo();
        void setComment(const char* text);
        char* comment_{nullptr};
    };
    union ValueHolder
    {
        LargestInt int_;
        LargestUInt uint_;
        double real_;
        bool bool_;
        char* string_;
        ObjectValues* map_;
    } value_;
    ValueType type_ : 8;
    bool allocated_ : 1;
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
    std::string key_;
    ArrayIndex index_;
    Kind kind_;

public:
    friend class Path;
    PathArgument();
    PathArgument(ArrayIndex index);
    PathArgument(const char* key);
    PathArgument(const std::string& key);
};
class Path
{
private:
    typedef std::vector<const PathArgument*> InArgs;
    typedef std::vector<PathArgument> Args;
    void makePath(const std::string& path, const InArgs& in);
    void addPathInArg(const std::string& path, const InArgs& in,
        InArgs::const_iterator& itInArg, PathArgument::Kind kind);
    void invalidPath(const std::string& path, int location);
    Args args_;

public:
    Path(const std::string& path, const PathArgument& a1 = PathArgument(),
        const PathArgument& a2 = PathArgument(),
        const PathArgument& a3 = PathArgument(),
        const PathArgument& a4 = PathArgument(),
        const PathArgument& a5 = PathArgument());
    const Value& resolve(const Value& root) const;
    Value resolve(const Value& root, const Value& defaultValue) const;
    Value& make(Value& root) const;
};
class ValueIteratorBase
{
public:
    using size_t = unsigned int;
    using difference_type = int;
    using SelfType = ValueIteratorBase;

private:
    Value::ObjectValues::iterator current_;
    bool isNull_;

protected:
    Value& deref() const;
    void increment();
    void decrement();
    difference_type computeDistance(const SelfType& other) const;
    bool isEqual(const SelfType& other) const;
    void copy(const SelfType& other);

public:
    ValueIteratorBase();
    explicit ValueIteratorBase(const Value::ObjectValues::iterator& current);
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
    Value key() const;
    UInt index() const;
    const char* memberName() const;
};
class ValueConstIterator final : public ValueIteratorBase
{
    friend class Value;

private:
    explicit ValueConstIterator(const Value::ObjectValues::iterator& current);

public:
    using size_t = unsigned int;
    using difference_type = int;
    using reference = const Value&;
    using pointer = const Value*;
    using SelfType = ValueConstIterator;
    ValueConstIterator();
    SelfType& operator=(const ValueIteratorBase& other);
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
    explicit ValueIterator(const Value::ObjectValues::iterator& current);

public:
    using size_t = unsigned int;
    using difference_type = int;
    using reference = Value&;
    using pointer = Value*;
    using SelfType = ValueIterator;
    ValueIterator();
    ValueIterator(const ValueConstIterator& other);
    ValueIterator(const ValueIterator& other);
    SelfType& operator=(const SelfType& other);
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
    Reader(const Features& features);
    bool parse(
        const std::string& document, Value& root, bool collectComments = true);
    bool parse(const char* beginDoc, const char* endDoc, Value& root,
        bool collectComments = true);
    bool parse(std::istream& is, Value& root, bool collectComments = true);
    std::string getFormattedErrorMessages() const;

private:
    enum TokenType
    {
        tokenEndOfStream = 0,
        tokenObjectBegin,
        tokenObjectEnd,
        tokenArrayBegin,
        tokenArrayEnd,
        tokenString,
        tokenNumber,
        tokenTrue,
        tokenFalse,
        tokenNull,
        tokenArraySeparator,
        tokenMemberSeparator,
        tokenComment,
        tokenError
    };
    struct Token
    {
        TokenType type_;
        Location start_, end_;
    };
    struct ErrorInfo
    {
        Token token_;
        std::string message_;
        Location extra_;
    };

    typedef std::deque<ErrorInfo> Errors;
    bool expectToken(TokenType type, Token& token, const char* message);
    bool readToken(Token& token);
    void skipSpaces();
    bool match(Location pattern, int patternLength);
    bool readComment();
    bool readCStyleComment();
    bool readCppStyleComment();
    bool readString();
    void readNumber();
    bool readValue();
    bool readObject(Token& token);
    bool readArray(Token& token);
    bool decodeNumber(Token& token);
    bool decodeString(Token& token);
    bool decodeString(Token& token, std::string& decoded);
    bool decodeDouble(Token& token);
    bool decodeUnicodeCodePoint(
        Token& token, Location& current, Location end, unsigned int& unicode);
    bool decodeUnicodeEscapeSequence(
        Token& token, Location& current, Location end, unsigned int& unicode);
    bool addError(const std::string& message, Token& token, Location extra = 0);
    bool recoverFromError(TokenType skipUntilToken);
    bool addErrorAndRecover(
        const std::string& message, Token& token, TokenType skipUntilToken);
    void skipUntilSpace();
    Value& currentValue();
    char getNextChar();
    void getLocationLineAndColumn(
        Location location, int& line, int& column) const;
    std::string getLocationLineAndColumn(Location location) const;
    void addComment(Location begin, Location end, CommentPlacement placement);
    void skipCommentTokens(Token& token);
    typedef std::stack<Value*> Nodes;
    Nodes nodes_;
    Errors errors_;
    std::string document_;
    Location begin_, end_, current_, lastValueEnd_;
    Value* lastValue_;
    std::string commentsBefore_;
    Features features_;
    bool collectComments_;
};
std::istream& operator>>(std::istream&, Value&);

class Value;
struct Writer
{
    virtual ~Writer();
    virtual std::string write(const Value& root) = 0;
};
class FastWriter final : public Writer
{
public:
    FastWriter();
    virtual ~FastWriter()
    {}
    void enableYAMLCompatibility();
    void dropNullPlaceholders();
    virtual std::string write(const Value& root);

private:
    void writeValue(const Value& value);
    std::string document_;
    bool yamlCompatiblityEnabled_;
    bool dropNullPlaceholders_;
};
class StyledWriter final : public Writer
{
public:
    StyledWriter();
    virtual ~StyledWriter()
    {}
    virtual std::string write(const Value& root);

private:
    void writeValue(const Value& value);
    void writeArrayValue(const Value& value);
    bool isMultineArray(const Value& value);
    void pushValue(const std::string& value);
    void writeIndent();
    void writeWithIndent(const std::string& value);
    void indent();
    void unindent();
    void writeCommentBeforeValue(const Value& root);
    void writeCommentAfterValueOnSameLine(const Value& root);
    bool hasCommentForValue(const Value& value);
    static std::string normalizeEOL(const std::string& text);
    typedef std::vector<std::string> ChildValues;
    ChildValues childValues_;
    std::string document_;
    std::string indentString_;
    int rightMargin_;
    int indentSize_;
    bool addChildValues_;
};
class StyledStreamWriter
{
public:
    StyledStreamWriter(std::string indentation = "\t");
    ~StyledStreamWriter()
    {}
    void write(std::ostream& out, const Value& root);

private:
    void writeValue(const Value& value);
    void writeArrayValue(const Value& value);
    bool isMultineArray(const Value& value);
    void pushValue(const std::string& value);
    void writeIndent();
    void writeWithIndent(const std::string& value);
    void indent();
    void unindent();
    void writeCommentBeforeValue(const Value& root);
    void writeCommentAfterValueOnSameLine(const Value& root);
    bool hasCommentForValue(const Value& value);
    static std::string normalizeEOL(const std::string& text);
    typedef std::vector<std::string> ChildValues;
    ChildValues childValues_;
    std::ostream* document_;
    std::string indentString_;
    int rightMargin_;
    std::string indentation_;
    bool addChildValues_;
};
static const Value nullJsonValue{};
#ifdef JSON_HAS_INT64
std::string valueToString(int value);
std::string valueToString(UInt value);
#endif
std::string valueToString(LargestInt value);
std::string valueToString(LargestUInt value);
std::string valueToString(double value);
std::string valueToString(bool value);
std::string valueToQuotedString(const char* value);
std::ostream& operator<<(std::ostream&, const Value& root);
} // namespace Json

#endif
