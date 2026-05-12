// Copyright(c) 2013 Vittorio Romeo
// License: Academic Free License("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/JsonCpp/JsonStream.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/JsonCpp/json.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Math/Floor.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <ios>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define JSON_FAIL_MESSAGE(message) throw std::runtime_error(message);
#define JSON_ASSERT_MESSAGE(condition, message) \
    if (!(condition))                           \
    {                                           \
        JSON_FAIL_MESSAGE(message)              \
    }
#define JSON_ASSERT_UNREACHABLE SSVOH_ASSERT(false)

namespace Json
{

////////////////////////////////////////////////////////////
// Hash and equality for CZString
////////////////////////////////////////////////////////////

struct CZStringHash
{
    using is_avalanching = void;

    sf::base::SizeT operator()(const Value::CZString& cz) const noexcept
    {
        if (const char* c = cz.c_str())
        {
            // FNV-1a (good distribution, fits the avalanching contract reasonably)
            sf::base::SizeT h = 14'695'981'039'346'656'037ULL;
            for (; *c; ++c)
            {
                h ^= static_cast<unsigned char>(*c);
                h *= 1'099'511'628'211ULL;
            }
            return h;
        }

        // Mix the index; bias with a sentinel so it doesn't collide with empty-string keys.
        return ankerl::unordered_dense::detail::wyhash::hash(
            static_cast<sf::base::SizeT>(cz.index()) ^ 0x9E'37'79'B9'7F'4A'7C'15ULL);
    }
};

////////////////////////////////////////////////////////////
// Pimpl storage type
////////////////////////////////////////////////////////////

// Note: non-segmented map. Pointer-stability is not required by the parser/users:
//   the parser only pushes a freshly-returned reference onto its node stack, recurses into the
//   *child*, and pops before inserting any sibling. Container resizes that follow only invalidate
//   already-popped pointers. Iteration order is insertion order (matches segmented_map's, which is
//   what the writers depend on for object key order).
class ObjectValuesImpl : public ankerl::unordered_dense::map<Value::CZString, Value, CZStringHash>
{
public:
    using base = ankerl::unordered_dense::map<Value::CZString, Value, CZStringHash>;
    using base::base;
};

struct ObjectValuesIteratorImpl
{
    using underlying = ObjectValuesImpl::iterator;
    underlying it{};

    ObjectValuesIteratorImpl() noexcept = default;
    explicit ObjectValuesIteratorImpl(underlying i) noexcept : it{i}
    {
    }
};

////////////////////////////////////////////////////////////
// Internal helpers
////////////////////////////////////////////////////////////

namespace
{
enum
{
    uintToStringBufferSize = 3 * sizeof(LargestUInt) + 1
};

using UIntToStringBuffer = char[uintToStringBufferSize];

void uintToString(LargestUInt value, char*& current)
{
    *--current = 0;
    do
    {
        *--current = char(value % 10) + '0';
        value /= 10;
    } while (value != 0);
}

bool containsControlCharacter(const char* str)
{
    while (*str)
        if (isControlCharacter(*(str++)))
            return true;
    return false;
}

const unsigned int unknown = (unsigned)-1;
template <typename T, typename U>
bool InRange(double d, T min, U max)
{
    return d >= min && d <= max;
}

char* duplicateStringValue(const char* value, unsigned int length = unknown)
{
    if (length == unknown)
        length = (unsigned int)strlen(value);
    if (length >= (unsigned)Value::maxInt)
        length = Value::maxInt - 1;
    char* newString = static_cast<char*>(malloc(length + 1));
    JSON_ASSERT_MESSAGE(newString != nullptr, "Failed to allocate string value buffer");
    memcpy(newString, value, length);
    newString[length] = 0;
    return newString;
}

void releaseStringValue(char* value)
{
    if (value)
        free(value);
}

bool IsIntegral(double d)
{
    return d == SFML_BASE_MATH_FLOOR(d);
}

#ifdef JSON_HAS_INT64
const double maxUInt64AsDouble = 18446744073709551615.0;
#endif

} // namespace

std::string codePointToUTF8(unsigned int cp)
{
    std::string result;
    if (cp <= 0x7f)
    {
        result.resize(1);
        result[0] = static_cast<char>(cp);
    }
    else if (cp <= 0x7'FF)
    {
        result.resize(2);
        result[1] = static_cast<char>(0x80 | (0x3f & cp));
        result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
    }
    else if (cp <= 0xFF'FF)
    {
        result.resize(3);
        result[2] = static_cast<char>(0x80 | (0x3f & cp));
        result[1] = 0x80 | static_cast<char>((0x3f & (cp >> 6)));
        result[0] = 0xE0 | static_cast<char>((0xf & (cp >> 12)));
    }
    else if (cp <= 0x10'FF'FF)
    {
        result.resize(4);
        result[3] = static_cast<char>(0x80 | (0x3f & cp));
        result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
        result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
    }
    return result;
}

bool containsNewLine(Reader::Location begin, Reader::Location end)
{
    for (; begin < end; ++begin)
        if (*begin == '\n' || *begin == '\r')
            return true;
    return false;
}

////////////////////////////////////////////////////////////
// Reader::Impl
////////////////////////////////////////////////////////////

struct Reader::Impl
{
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
        TokenType        type_;
        Reader::Location start_, end_;
    };

    struct ErrorInfo
    {
        Token            token_;
        sf::base::String message_;
        Reader::Location extra_;
    };

    sf::base::Vector<Value*>    nodes_;
    sf::base::Vector<ErrorInfo> errors_;
    sf::base::String            document_;
    Reader::Location            begin_{}, end_{}, current_{}, lastValueEnd_{};
    Value*                      lastValue_{};
    sf::base::String            commentsBefore_;
    Features                    features_{Features::all()};
    bool                        collectComments_{};
};

////////////////////////////////////////////////////////////
// Reader
////////////////////////////////////////////////////////////

Reader::Reader() : impl_{}
{
}

Reader::Reader(const Features& features) : impl_{}
{
    impl_->features_ = features;
}

Reader::~Reader() = default;

Reader::Reader(Reader&&) noexcept            = default;
Reader& Reader::operator=(Reader&&) noexcept = default;

namespace
{
struct ParseState
{
    Reader::Impl& impl;

    bool readValue();
    bool readToken(Reader::Impl::Token& token);
    void skipSpaces();
    bool match(Reader::Location pattern, int patternLength);
    bool readComment();
    bool readCStyleComment();
    bool readCppStyleComment();
    bool readString();
    void readNumber();
    bool readObject(Reader::Impl::Token&);
    bool readArray(Reader::Impl::Token&);
    bool decodeNumber(Reader::Impl::Token& token);
    bool decodeString(Reader::Impl::Token& token);
    bool decodeString(Reader::Impl::Token& token, sf::base::String& decoded);
    bool decodeDouble(Reader::Impl::Token& token);
    bool decodeUnicodeCodePoint(Reader::Impl::Token& token, Reader::Location& current, Reader::Location end, unsigned int& unicode);
    bool decodeUnicodeEscapeSequence(Reader::Impl::Token& token,
                                     Reader::Location&    current,
                                     Reader::Location     end,
                                     unsigned int&        unicode);
    bool addError(const sf::base::String& message, Reader::Impl::Token& token, Reader::Location extra = nullptr);
    bool recoverFromError(Reader::Impl::TokenType skipUntilToken);
    bool addErrorAndRecover(const sf::base::String& message, Reader::Impl::Token& token, Reader::Impl::TokenType skipUntilToken);
    Value& currentValue();
    char   getNextChar();
    bool   expectToken(Reader::Impl::TokenType type, Reader::Impl::Token& token, const char* message);
    void   skipCommentTokens(Reader::Impl::Token& token);
    void   addComment(Reader::Location begin, Reader::Location end, CommentPlacement placement);
};

bool ParseState::readValue()
{
    Reader::Impl::Token token;
    skipCommentTokens(token);
    bool successful{true};
    if (impl.collectComments_ && !impl.commentsBefore_.empty())
    {
        currentValue().setComment(impl.commentsBefore_, commentBefore);
        impl.commentsBefore_ = "";
    }
    switch (token.type_)
    {
        case Reader::Impl::tokenObjectBegin:
            successful = readObject(token);
            break;
        case Reader::Impl::tokenArrayBegin:
            successful = readArray(token);
            break;
        case Reader::Impl::tokenNumber:
            successful = decodeNumber(token);
            break;
        case Reader::Impl::tokenString:
            successful = decodeString(token);
            break;
        case Reader::Impl::tokenTrue:
            currentValue() = true;
            break;
        case Reader::Impl::tokenFalse:
            currentValue() = false;
            break;
        case Reader::Impl::tokenNull:
            currentValue() = Value();
            break;
        default:
            return addError("Syntax error: value, object or array expected.", token);
    }
    if (impl.collectComments_)
    {
        impl.lastValueEnd_ = impl.current_;
        impl.lastValue_    = &currentValue();
    }
    return successful;
}

void ParseState::skipCommentTokens(Reader::Impl::Token& token)
{
    if (impl.features_.allowComments_)
    {
        do
        {
            readToken(token);
        } while (token.type_ == Reader::Impl::tokenComment);
    }
    else
        readToken(token);
}

bool ParseState::expectToken(Reader::Impl::TokenType type, Reader::Impl::Token& token, const char* message)
{
    readToken(token);
    if (token.type_ != type)
        return addError(message, token);
    return true;
}

bool ParseState::readToken(Reader::Impl::Token& token)
{
    skipSpaces();
    token.start_ = impl.current_;
    char c{getNextChar()};
    bool ok{true};
    switch (c)
    {
        case '{':
            token.type_ = Reader::Impl::tokenObjectBegin;
            break;
        case '}':
            token.type_ = Reader::Impl::tokenObjectEnd;
            break;
        case '[':
            token.type_ = Reader::Impl::tokenArrayBegin;
            break;
        case ']':
            token.type_ = Reader::Impl::tokenArrayEnd;
            break;
        case '"':
            token.type_ = Reader::Impl::tokenString;
            ok          = readString();
            break;
        case '/':
            token.type_ = Reader::Impl::tokenComment;
            ok          = readComment();
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            token.type_ = Reader::Impl::tokenNumber;
            readNumber();
            break;
        case 't':
            token.type_ = Reader::Impl::tokenTrue;
            ok          = match("rue", 3);
            break;
        case 'f':
            token.type_ = Reader::Impl::tokenFalse;
            ok          = match("alse", 4);
            break;
        case 'n':
            token.type_ = Reader::Impl::tokenNull;
            ok          = match("ull", 3);
            break;
        case ',':
            token.type_ = Reader::Impl::tokenArraySeparator;
            break;
        case ':':
            token.type_ = Reader::Impl::tokenMemberSeparator;
            break;
        case 0:
            token.type_ = Reader::Impl::tokenEndOfStream;
            break;
        default:
            ok = false;
            break;
    }
    if (!ok)
        token.type_ = Reader::Impl::tokenError;
    token.end_ = impl.current_;
    return true;
}

void ParseState::skipSpaces()
{
    while (impl.current_ != impl.end_)
    {
        char c{*impl.current_};
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            ++impl.current_;
        else
            break;
    }
}

bool ParseState::match(Reader::Location pattern, int patternLength)
{
    if (impl.end_ - impl.current_ < patternLength)
        return false;
    int index = patternLength;
    while (index--)
        if (impl.current_[index] != pattern[index])
            return false;
    impl.current_ += patternLength;
    return true;
}

bool ParseState::readComment()
{
    Reader::Location commentBegin = impl.current_ - 1;
    char             c            = getNextChar();
    bool             successful   = false;
    if (c == '*')
        successful = readCStyleComment();
    else if (c == '/')
        successful = readCppStyleComment();
    if (!successful)
        return false;
    if (impl.collectComments_)
    {
        CommentPlacement placement = commentBefore;
        if (impl.lastValueEnd_ && !containsNewLine(impl.lastValueEnd_, commentBegin))
            if (c != '*' || !containsNewLine(commentBegin, impl.current_))
                placement = commentAfterOnSameLine;
        addComment(commentBegin, impl.current_, placement);
    }
    return true;
}

void ParseState::addComment(Reader::Location begin, Reader::Location end, CommentPlacement placement)
{
    SSVOH_ASSERT(impl.collectComments_);
    if (placement == commentAfterOnSameLine)
    {
        SSVOH_ASSERT(impl.lastValue_ != 0);
        impl.lastValue_->setComment(sf::base::String(begin, static_cast<sf::base::SizeT>(end - begin)), placement);
    }
    else
    {
        if (!impl.commentsBefore_.empty())
            impl.commentsBefore_ += "\n";
        impl.commentsBefore_ += sf::base::String(begin, static_cast<sf::base::SizeT>(end - begin));
    }
}

bool ParseState::readCStyleComment()
{
    while (impl.current_ != impl.end_)
    {
        char c = getNextChar();
        if (c == '*' && *impl.current_ == '/')
            break;
    }
    return getNextChar() == '/';
}

bool ParseState::readCppStyleComment()
{
    while (impl.current_ != impl.end_)
    {
        char c = getNextChar();
        if (c == '\r' || c == '\n')
            break;
    }
    return true;
}

void ParseState::readNumber()
{
    while (impl.current_ != impl.end_)
    {
        if (!(*impl.current_ >= '0' && *impl.current_ <= '9') && !in(*impl.current_, '.', 'e', 'E', '+', '-'))
            break;
        ++impl.current_;
    }
}

bool ParseState::readString()
{
    char c = 0;
    while (impl.current_ != impl.end_)
    {
        c = getNextChar();
        if (c == '\\')
            getNextChar();
        else if (c == '"')
            break;
    }
    return c == '"';
}

bool ParseState::readObject(Reader::Impl::Token&)
{
    Reader::Impl::Token tokenName;
    sf::base::String    name;
    currentValue() = Value(objectValue);
    while (readToken(tokenName))
    {
        bool initialTokenOk = true;
        while (tokenName.type_ == Reader::Impl::tokenComment && initialTokenOk)
            initialTokenOk = readToken(tokenName);
        if (!initialTokenOk)
            break;
        if (tokenName.type_ == Reader::Impl::tokenObjectEnd && name.empty())
            return true;
        if (tokenName.type_ != Reader::Impl::tokenString)
            break;
        name = "";
        if (!decodeString(tokenName, name))
            return recoverFromError(Reader::Impl::tokenObjectEnd);
        Reader::Impl::Token colon;
        if (!readToken(colon) || colon.type_ != Reader::Impl::tokenMemberSeparator)
        {
            return addErrorAndRecover("Missing ':' after object member name", colon, Reader::Impl::tokenObjectEnd);
        }
        Value& value = currentValue()[name];
        impl.nodes_.emplaceBack(&value);
        bool ok = readValue();
        impl.nodes_.popBack();
        if (!ok)
            return recoverFromError(Reader::Impl::tokenObjectEnd);
        Reader::Impl::Token comma;
        if (!readToken(comma) ||
            (comma.type_ != Reader::Impl::tokenObjectEnd && comma.type_ != Reader::Impl::tokenArraySeparator &&
             comma.type_ != Reader::Impl::tokenComment))
        {
            return addErrorAndRecover("Missing ',' or '}' in object declaration", comma, Reader::Impl::tokenObjectEnd);
        }
        bool finalizeTokenOk = true;
        while (comma.type_ == Reader::Impl::tokenComment && finalizeTokenOk)
            finalizeTokenOk = readToken(comma);
        if (comma.type_ == Reader::Impl::tokenObjectEnd)
            return true;
    }
    return addErrorAndRecover("Missing '}' or object member name", tokenName, Reader::Impl::tokenObjectEnd);
}

bool ParseState::readArray(Reader::Impl::Token&)
{
    currentValue() = Value(arrayValue);
    skipSpaces();
    if (*impl.current_ == ']')
    {
        Reader::Impl::Token endArray;
        readToken(endArray);
        return true;
    }
    int index{0};
    while (true)
    {
        Value& value = currentValue()[index++];
        impl.nodes_.emplaceBack(&value);
        bool ok = readValue();
        impl.nodes_.popBack();
        if (!ok)
            return recoverFromError(Reader::Impl::tokenArrayEnd);
        Reader::Impl::Token token;
        ok = readToken(token);
        while (token.type_ == Reader::Impl::tokenComment && ok)
        {
            ok = readToken(token);
        }
        bool badTokenType = (token.type_ != Reader::Impl::tokenArraySeparator && token.type_ != Reader::Impl::tokenArrayEnd);
        if (!ok || badTokenType)
        {
            return addErrorAndRecover("Missing ',' or ']' in array declaration", token, Reader::Impl::tokenArrayEnd);
        }
        if (token.type_ == Reader::Impl::tokenArrayEnd)
            break;
    }
    return true;
}

bool ParseState::decodeNumber(Reader::Impl::Token& token)
{
    bool isDouble = false;
    for (Reader::Location inspect = token.start_; inspect != token.end_; ++inspect)
    {
        isDouble = isDouble || in(*inspect, '.', 'e', 'E', '+') || (*inspect == '-' && inspect != token.start_);
    }
    if (isDouble)
        return decodeDouble(token);
    Reader::Location current    = token.start_;
    bool             isNegative = *current == '-';
    if (isNegative)
        ++current;
    LargestUInt maxIntegerValue = isNegative ? LargestUInt(Value::minLargestInt) : Value::maxLargestUInt;
    LargestUInt threshold       = maxIntegerValue / 10;
    LargestUInt value           = 0;
    while (current < token.end_)
    {
        char c = *current++;
        if (c < '0' || c > '9')
            return addError(sf::base::String("'") +
                                sf::base::String(token.start_, static_cast<sf::base::SizeT>(token.end_ - token.start_)) +
                                "' is not a number.",
                            token);
        UInt digit(c - '0');
        if (value >= threshold)
        {
            if (value > threshold || current != token.end_ || digit > maxIntegerValue % 10)
                return decodeDouble(token);
        }
        value = value * 10 + digit;
    }
    if (isNegative)
        currentValue() = -LargestInt(value);
    else if (value <= LargestUInt(Value::maxInt))
        currentValue() = LargestInt(value);
    else
        currentValue() = value;
    return true;
}

bool ParseState::decodeDouble(Reader::Impl::Token& token)
{
    double    value      = 0;
    const int bufferSize = 32;
    int       count;
    int       length = int(token.end_ - token.start_);
    if (length < 0)
    {
        return addError("Unable to parse token length", token);
    }
    char format[]{"%lf"};
    if (length <= bufferSize)
    {
        char buffer[bufferSize + 1];
        memcpy(buffer, token.start_, length);
        buffer[length] = 0;
        count          = sscanf(buffer, format, &value);
    }
    else
    {
        std::string buffer(token.start_, token.end_);
        count = sscanf(buffer.c_str(), format, &value);
    }
    if (count != 1)
        return addError(sf::base::String("'") +
                            sf::base::String(token.start_, static_cast<sf::base::SizeT>(token.end_ - token.start_)) +
                            "' is not a number.",
                        token);
    currentValue() = value;
    return true;
}

bool ParseState::decodeString(Reader::Impl::Token& token)
{
    sf::base::String decoded;
    if (!decodeString(token, decoded))
        return false;
    currentValue() = decoded;
    return true;
}

bool ParseState::decodeString(Reader::Impl::Token& token, sf::base::String& decoded)
{
    decoded.reserve(static_cast<sf::base::SizeT>(token.end_ - token.start_ - 2));
    Reader::Location current = token.start_ + 1;
    Reader::Location end     = token.end_ - 1;
    while (current != end)
    {
        char c = *current++;
        if (c == '"')
            break;
        else if (c == '\\')
        {
            if (current == end)
                return addError("Empty escape sequence in string", token, current);
            char escape = *current++;
            switch (escape)
            {
                case '"':
                    decoded += '"';
                    break;
                case '/':
                    decoded += '/';
                    break;
                case '\\':
                    decoded += '\\';
                    break;
                case 'b':
                    decoded += '\b';
                    break;
                case 'f':
                    decoded += '\f';
                    break;
                case 'n':
                    decoded += '\n';
                    break;
                case 'r':
                    decoded += '\r';
                    break;
                case 't':
                    decoded += '\t';
                    break;
                case 'u':
                {
                    unsigned int unicode{0u};
                    if (!decodeUnicodeCodePoint(token, current, end, unicode))
                        return false;
                    const std::string utf8 = codePointToUTF8(unicode);
                    decoded.append(utf8.data(), utf8.size());
                }
                break;
                default:
                    return addError("Bad escape sequence in string", token, current);
            }
        }
        else
        {
            decoded += c;
        }
    }
    return true;
}

bool ParseState::decodeUnicodeCodePoint(Reader::Impl::Token& token,
                                        Reader::Location&    current,
                                        Reader::Location     end,
                                        unsigned int&        unicode)
{
    if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
        return false;
    if (unicode >= 0xD8'00 && unicode <= 0xDB'FF)
    {
        if (end - current < 6)
            return addError("additional six characters expected to parse unicode surrogate pair.", token, current);
        unsigned int surrogatePair{};
        if (*(current++) == '\\' && *(current++) == 'u')
        {
            if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair))
            {
                unicode = 0x1'00'00 + ((unicode & 0x3'FF) << 10) + (surrogatePair & 0x3'FF);
            }
            else
                return false;
        }
        else
            return addError("expecting another \\u token to begin the second half of a unicode surrogate pair", token, current);
    }
    return true;
}

bool ParseState::decodeUnicodeEscapeSequence(Reader::Impl::Token& token,
                                             Reader::Location&    current,
                                             Reader::Location     end,
                                             unsigned int&        unicode)
{
    if (end - current < 4)
        return addError("Bad unicode escape sequence in string: four digits expected.", token, current);
    unicode = 0;
    for (int index = 0; index < 4; ++index)
    {
        char c = *current++;
        unicode *= 16;
        if (c >= '0' && c <= '9')
            unicode += c - '0';
        else if (c >= 'a' && c <= 'f')
            unicode += c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            unicode += c - 'A' + 10;
        else
            return addError("Bad unicode escape sequence in string: hexadecimal digit expected.", token, current);
    }
    return true;
}

bool ParseState::addError(const sf::base::String& message, Reader::Impl::Token& token, Reader::Location extra)
{
    Reader::Impl::ErrorInfo info;
    info.token_   = token;
    info.message_ = message;
    info.extra_   = extra;
    impl.errors_.emplaceBack(info);
    return false;
}

bool ParseState::recoverFromError(Reader::Impl::TokenType skipUntilToken)
{
    int                 errorCount = int(impl.errors_.size());
    Reader::Impl::Token skip;
    while (true)
    {
        if (!readToken(skip))
            impl.errors_.resize(errorCount);
        if (skip.type_ == skipUntilToken || skip.type_ == Reader::Impl::tokenEndOfStream)
            break;
    }
    impl.errors_.resize(errorCount);
    return false;
}

bool ParseState::addErrorAndRecover(const sf::base::String& message, Reader::Impl::Token& token, Reader::Impl::TokenType skipUntilToken)
{
    addError(message, token);
    return recoverFromError(skipUntilToken);
}

Value& ParseState::currentValue()
{
    return *impl.nodes_.back();
}

char ParseState::getNextChar()
{
    if (impl.current_ == impl.end_)
        return 0;
    return *impl.current_++;
}

void getLocationLineAndColumn(Reader::Location begin, Reader::Location end, Reader::Location location, int& line, int& column)
{
    Reader::Location current       = begin;
    Reader::Location lastLineStart = current;
    line                           = 0;
    while (current < location && current != end)
    {
        char c = *current++;
        if (c == '\r')
        {
            if (*current == '\n')
                ++current;
            lastLineStart = current;
            ++line;
        }
        else if (c == '\n')
        {
            lastLineStart = current;
            ++line;
        }
    }
    column = int(location - lastLineStart) + 1;
    ++line;
}

sf::base::String formatLocationLineAndColumn(Reader::Location begin, Reader::Location end, Reader::Location location)
{
    int line, column;
    getLocationLineAndColumn(begin, end, location, line, column);
    char buffer[18 + 16 + 16 + 1];
    sprintf(buffer, "Line %d, Column %d", line, column);
    return sf::base::String(buffer);
}

} // namespace

bool Reader::parse(const sf::base::String& document, Value& root, bool collectComments)
{
    impl_->document_  = document;
    const char* begin = impl_->document_.data();
    const char* end   = begin + impl_->document_.size();
    return parse(begin, end, root, collectComments);
}

bool Reader::parse(const char* beginDoc, const char* endDoc, Value& root, bool collectComments)
{
    if (!impl_->features_.allowComments_)
        collectComments = false;
    impl_->begin_           = beginDoc;
    impl_->end_             = endDoc;
    impl_->collectComments_ = collectComments;
    impl_->current_         = impl_->begin_;
    impl_->lastValueEnd_    = nullptr;
    impl_->lastValue_       = nullptr;
    impl_->commentsBefore_  = "";
    impl_->errors_.clear();
    impl_->nodes_.clear();
    impl_->nodes_.emplaceBack(&root);
    ParseState  st{*impl_};
    bool        successful = st.readValue();
    Impl::Token token;
    st.skipCommentTokens(token);
    if (impl_->collectComments_ && !impl_->commentsBefore_.empty())
        root.setComment(impl_->commentsBefore_, commentAfter);
    if (impl_->features_.strictRoot_)
    {
        if (!root.isArray() && !root.isObject())
        {
            token.type_  = Impl::tokenError;
            token.start_ = beginDoc;
            token.end_   = endDoc;
            st.addError("A valid JSON document must be either an array or an object value.", token);
            return false;
        }
    }
    return successful;
}

sf::base::String Reader::getFormattedErrorMessages() const
{
    sf::base::String formattedMessage;
    for (const auto& error : impl_->errors_)
    {
        formattedMessage += "* " + formatLocationLineAndColumn(impl_->begin_, impl_->end_, error.token_.start_) + "\n";
        formattedMessage += "  " + error.message_ + "\n";
        if (error.extra_)
            formattedMessage += "See " + formatLocationLineAndColumn(impl_->begin_, impl_->end_, error.extra_) +
                                " for detail.\n";
    }
    return formattedMessage;
}

////////////////////////////////////////////////////////////
// JsonStream support
////////////////////////////////////////////////////////////

bool parse(Reader& reader, std::istream& sin, Value& root, bool collectComments)
{
    std::string doc;
    std::getline(sin, doc, (char)EOF);
    return reader.parse(sf::base::String(doc), root, collectComments);
}

std::istream& operator>>(std::istream& sin, Value& root)
{
    Json::Reader reader;
    bool         ok = parse(reader, sin, root, true);
    if (!ok)
    {
        const sf::base::String msg = reader.getFormattedErrorMessages();
        fprintf(stderr, "Error from reader: %.*s", static_cast<int>(msg.size()), msg.data());
        JSON_FAIL_MESSAGE("reader error");
    }
    return sin;
}

////////////////////////////////////////////////////////////
// ValueIteratorBase
////////////////////////////////////////////////////////////

ValueIteratorBase::ValueIteratorBase() : current_{}, isNull_(true)
{
}

ValueIteratorBase::ValueIteratorBase(const ObjectValuesIteratorImpl& current) : current_{current}, isNull_(false)
{
}

ValueIteratorBase::ValueIteratorBase(const ValueIteratorBase&)                = default;
ValueIteratorBase::ValueIteratorBase(ValueIteratorBase&&) noexcept            = default;
ValueIteratorBase& ValueIteratorBase::operator=(const ValueIteratorBase&)     = default;
ValueIteratorBase& ValueIteratorBase::operator=(ValueIteratorBase&&) noexcept = default;
ValueIteratorBase::~ValueIteratorBase()                                       = default;

Value& ValueIteratorBase::deref() const
{
    return current_->it->second;
}

void ValueIteratorBase::increment()
{
    ++current_->it;
}

void ValueIteratorBase::decrement()
{
    // segmented_vector iterator does not provide operator--, synthesize via offset.
    current_->it = current_->it + (-1);
}

ValueIteratorBase::difference_type ValueIteratorBase::computeDistance(const SelfType& other) const
{
    if (isNull_ && other.isNull_)
        return 0;
    return static_cast<difference_type>(other.current_->it - current_->it);
}

bool ValueIteratorBase::isEqual(const SelfType& other) const
{
    if (isNull_)
        return other.isNull_;
    if (other.isNull_)
        return false;
    // Take non-const copies so we can call the iterator's operator== (which is templated and
    // can confuse some compilers when invoked on const-qualified instances).
    ObjectValuesIteratorImpl::underlying a = current_->it;
    ObjectValuesIteratorImpl::underlying b = other.current_->it;
    return a == b;
}

void ValueIteratorBase::copy(const SelfType& other)
{
    *current_ = *other.current_;
    isNull_   = other.isNull_;
}

Value ValueIteratorBase::key() const
{
    const Value::CZString& czstring = current_->it->first;
    if (czstring.c_str())
    {
        if (czstring.isStaticString())
            return Value(StaticString(czstring.c_str()));
        return Value(czstring.c_str());
    }
    return Value(czstring.index());
}

UInt ValueIteratorBase::index() const
{
    const Value::CZString& czstring = current_->it->first;
    if (!czstring.c_str())
        return czstring.index();
    return UInt(-1);
}

const char* ValueIteratorBase::memberName() const
{
    const char* name = current_->it->first.c_str();
    return name ? name : "";
}

ValueConstIterator::ValueConstIterator()
{
}
ValueConstIterator::ValueConstIterator(const ObjectValuesIteratorImpl& current) : ValueIteratorBase(current)
{
}

ValueConstIterator& ValueConstIterator::operator=(const ValueIteratorBase& other)
{
    copy(other);
    return *this;
}
ValueIterator::ValueIterator()
{
}
ValueIterator::ValueIterator(const ObjectValuesIteratorImpl& current) : ValueIteratorBase(current)
{
}

ValueIterator::ValueIterator(const ValueConstIterator& other) : ValueIteratorBase(other)
{
}
ValueIterator::ValueIterator(const ValueIterator& other) : ValueIteratorBase(other)
{
}
ValueIterator& ValueIterator::operator=(const SelfType& other)
{
    copy(other);
    return *this;
}

////////////////////////////////////////////////////////////
// CommentInfo / CZString / Value
////////////////////////////////////////////////////////////

Value::CommentInfo::~CommentInfo()
{
    if (comment_)
        releaseStringValue(comment_);
}
void Value::CommentInfo::setComment(const char* text)
{
    if (comment_)
        releaseStringValue(comment_);
    SSVOH_ASSERT(text != 0);
    JSON_ASSERT_MESSAGE(text[0] == '\0' || text[0] == '/', "Comments must start with /");
    comment_ = duplicateStringValue(text);
}

Value::CZString::CZString(ArrayIndex mIdx) : cstr_(nullptr), index_(mIdx)
{
}

Value::CZString::CZString(const char* cstr, DuplicationPolicy allocate) :
    cstr_(allocate == duplicate ? duplicateStringValue(cstr) : cstr),
    index_(allocate)
{
}

Value::CZString::CZString(const CZString& other) :
    cstr_(other.index_ != noDuplication && other.cstr_ != nullptr ? duplicateStringValue(other.cstr_) : other.cstr_),
    index_(other.cstr_ ? (other.index_ == static_cast<unsigned int>(noDuplication)
                              ? static_cast<unsigned int>(noDuplication)
                              : static_cast<unsigned int>(duplicate))
                       : other.index_)
{
}

Value::CZString::CZString(CZString&& other) noexcept : cstr_(other.cstr_), index_(other.index_)
{
    other.cstr_  = nullptr;
    other.index_ = 0;
}

Value::CZString::~CZString()
{
    if (cstr_ != nullptr && index_ == duplicate)
        releaseStringValue(const_cast<char*>(cstr_));
}

void Value::CZString::swap(CZString& other)
{
    auto* tmpStr = cstr_;
    auto  tmpIdx = index_;
    cstr_        = other.cstr_;
    index_       = other.index_;
    other.cstr_  = tmpStr;
    other.index_ = tmpIdx;
}

Value::CZString& Value::CZString::operator=(const CZString& other)
{
    CZString temp(other);
    swap(temp);
    return *this;
}

Value::CZString& Value::CZString::operator=(CZString&& other) noexcept
{
    if (this != &other)
    {
        if (cstr_ != nullptr && index_ == duplicate)
            releaseStringValue(const_cast<char*>(cstr_));
        cstr_        = other.cstr_;
        index_       = other.index_;
        other.cstr_  = nullptr;
        other.index_ = 0;
    }
    return *this;
}

bool Value::CZString::operator<(const CZString& other) const
{
    if (cstr_ != nullptr && other.cstr_ != nullptr)
        return std::strcmp(cstr_, other.cstr_) < 0;

    return index_ < other.index_;
}

bool Value::CZString::operator==(const CZString& other) const
{
    if (cstr_ != nullptr && other.cstr_ != nullptr)
        return std::strcmp(cstr_, other.cstr_) == 0;
    if (cstr_ == nullptr && other.cstr_ == nullptr)
        return index_ == other.index_;
    return false;
}

ArrayIndex Value::CZString::index() const
{
    return index_;
}
const char* Value::CZString::c_str() const
{
    return cstr_;
}
bool Value::CZString::isStaticString() const
{
    return index_ == noDuplication;
}

Value::Value(ValueType mType) : type_(mType), allocated_(false)
{
    switch (mType)
    {
        case nullValue:
            break;
        case intValue:
        case uintValue:
            value_.int_ = 0;
            break;
        case realValue:
            value_.real_ = 0.0;
            break;
        case stringValue:
            value_.string_ = nullptr;
            break;
        case arrayValue:
        case objectValue:
            value_.map_ = new ObjectValuesImpl();
            break;
        case booleanValue:
            value_.bool_ = false;
            break;
        default:
            JSON_ASSERT_UNREACHABLE;
    }
}

Value::Value(UInt value) : type_(uintValue), allocated_(false)
{
    value_.uint_ = value;
}
Value::Value(int value) : type_(intValue), allocated_(false)
{
    value_.int_ = value;
}
#ifdef JSON_HAS_INT64
Value::Value(Int64 value) : type_(intValue), allocated_(false)
{
    value_.int_ = value;
}
Value::Value(UInt64 value) : type_(uintValue), allocated_(false)
{
    value_.uint_ = value;
}
#endif
Value::Value(double value) : type_(realValue), allocated_(false)
{
    value_.real_ = value;
}
Value::Value(const char* value) : type_(stringValue), allocated_(true)
{
    value_.string_ = duplicateStringValue(value);
}
Value::Value(const char* beginValue, const char* endValue) : type_(stringValue), allocated_(true)
{
    value_.string_ = duplicateStringValue(beginValue, (unsigned int)(endValue - beginValue));
}
Value::Value(const sf::base::String& value) : type_(stringValue), allocated_(true)
{
    value_.string_ = duplicateStringValue(value.data(), (unsigned int)value.size());
}
Value::Value(const StaticString& value) : type_(stringValue), allocated_(false)
{
    value_.string_ = const_cast<char*>(value.c_str());
}
Value::Value(bool value) : type_(booleanValue), allocated_(false)
{
    value_.bool_ = value;
}
Value::Value(const Value& other) : type_(other.type_), allocated_(false)
{
    switch (type_)
    {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
            value_ = other.value_;
            break;
        case stringValue:
            if (other.allocated_)
            {
                if (other.value_.string_)
                {
                    value_.string_ = duplicateStringValue(other.value_.string_);
                    allocated_     = true;
                }
                else
                    value_.string_ = nullptr;
            }
            else
            {
                value_.string_ = other.value_.string_;
                allocated_     = false;
            }

            break;
        case arrayValue:
        case objectValue:
            value_.map_ = new ObjectValuesImpl(*other.value_.map_);
            break;
        default:
            JSON_ASSERT_UNREACHABLE;
    }
    if (other.comments_)
    {
        comments_ = new CommentInfo[numberOfCommentPlacement];
        for (int comment = 0; comment < numberOfCommentPlacement; ++comment)
        {
            const CommentInfo& otherComment = other.comments_[comment];
            if (otherComment.comment_)
                comments_[comment].setComment(otherComment.comment_);
        }
    }
}
Value::~Value()
{
    switch (type_)
    {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
            break;
        case stringValue:
            if (allocated_)
                releaseStringValue(value_.string_);
            break;
        case arrayValue:
        case objectValue:
            delete value_.map_;
            break;
        default:
            JSON_ASSERT_UNREACHABLE;
    }
    if (comments_)
        delete[] comments_;
}

void Value::swap(Value& other)
{
    ValueType temp        = type_;
    type_                 = other.type_;
    other.type_           = temp;
    ValueHolder valueTemp = value_;
    value_                = other.value_;
    other.value_          = valueTemp;
    int temp2             = allocated_;
    allocated_            = other.allocated_;
    other.allocated_      = temp2;
}

bool Value::operator<(const Value& other) const
{
    int typeDelta = type_ - other.type_;
    if (typeDelta)
        return typeDelta < 0 ? true : false;
    switch (type_)
    {
        case nullValue:
            return false;
        case intValue:
            return value_.int_ < other.value_.int_;
        case uintValue:
            return value_.uint_ < other.value_.uint_;
        case realValue:
            return value_.real_ < other.value_.real_;
        case booleanValue:
            return value_.bool_ < other.value_.bool_;
        case stringValue:
            return (value_.string_ == nullptr && other.value_.string_) ||
                   (other.value_.string_ && value_.string_ && std::strcmp(value_.string_, other.value_.string_) < 0);
        case arrayValue:
        case objectValue:
        {
            int delta = int(value_.map_->size() - other.value_.map_->size());
            if (delta)
                return delta < 0;
            // Lexicographic compare in insertion order. Loses the ordered semantics of std::map but
            // matches the original size-then-content shape closely enough for the project's use.
            auto       it      = value_.map_->begin();
            auto       itOther = other.value_.map_->begin();
            const auto end     = value_.map_->end();
            while (it != end)
            {
                if (it->first < itOther->first)
                    return true;
                if (itOther->first < it->first)
                    return false;
                if (it->second < itOther->second)
                    return true;
                if (itOther->second < it->second)
                    return false;
                ++it;
                ++itOther;
            }
            return false;
        }

        default:
            JSON_ASSERT_UNREACHABLE;
    }
    return false;
}

bool Value::operator==(const Value& other) const
{
    int temp = other.type_;
    if (type_ != temp)
        return false;
    switch (type_)
    {
        case nullValue:
            return true;
        case intValue:
            return value_.int_ == other.value_.int_;
        case uintValue:
            return value_.uint_ == other.value_.uint_;
        case realValue:
            return value_.real_ == other.value_.real_;
        case booleanValue:
            return value_.bool_ == other.value_.bool_;
        case stringValue:
            return (value_.string_ == other.value_.string_) ||
                   (other.value_.string_ && value_.string_ && std::strcmp(value_.string_, other.value_.string_) == 0);
        case arrayValue:
        case objectValue:
            return *value_.map_ == *other.value_.map_;
        default:
            JSON_ASSERT_UNREACHABLE;
    }
    return false;
}

bool Value::operator!=(const Value& other) const
{
    return !(*this == other);
}

const char* Value::asCString() const
{
    SSVOH_ASSERT(type_ == stringValue);
    return value_.string_;
}

sf::base::String Value::asString() const
{
    switch (type_)
    {
        case nullValue:
            return "";
        case stringValue:
            return value_.string_ ? sf::base::String(value_.string_) : sf::base::String("");
        case booleanValue:
            return value_.bool_ ? "true" : "false";
        case intValue:
            return valueToString(value_.int_);
        case uintValue:
            return valueToString(value_.uint_);
        case realValue:
            return valueToString(value_.real_);
        default:
            JSON_FAIL_MESSAGE("Type is not convertible to string");
    }
}

int Value::asInt() const
{
    switch (type_)
    {
        case intValue:
            JSON_ASSERT_MESSAGE(isInt(), "LargestInt out of int range");
            return int(value_.int_);
        case uintValue:
            JSON_ASSERT_MESSAGE(isInt(), "LargestUInt out of int range");
            return int(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt, maxInt), "double out of int range");
            return int(value_.real_);
        case nullValue:
            return 0;
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        default:
            break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to int.");
}

UInt Value::asUInt() const
{
    switch (type_)
    {
        case intValue:
            JSON_ASSERT_MESSAGE(isUInt(), "LargestInt out of UInt range");
            return UInt(value_.int_);
        case uintValue:
            JSON_ASSERT_MESSAGE(isUInt(), "LargestUInt out of UInt range");
            return UInt(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt), "double out of UInt range");
            return UInt(value_.real_);
        case nullValue:
            return 0;
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        default:
            JSON_FAIL_MESSAGE("Value is not convertible to UInt.");
    }
}

#ifdef JSON_HAS_INT64
Int64 Value::asInt64() const
{
    switch (type_)
    {
        case intValue:
            return Int64(value_.int_);
        case uintValue:
            JSON_ASSERT_MESSAGE(isInt64(), "LargestUInt out of Int64 range");
            return Int64(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt64, maxInt64), "double out of Int64 range");
            return Int64(value_.real_);
        case nullValue:
            return 0;
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        default:
            break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to Int64.");
}
UInt64 Value::asUInt64() const
{
    switch (type_)
    {
        case intValue:
            JSON_ASSERT_MESSAGE(isUInt64(), "LargestInt out of UInt64 range");
            return UInt64(value_.int_);
        case uintValue:
            return UInt64(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt64), "double out of UInt64 range");
            return UInt64(value_.real_);
        case nullValue:
            return 0;
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        default:
            break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to UInt64.");
}
#endif
LargestInt Value::asLargestInt() const
{
#ifdef JSON_NO_INT64
    return asInt();
#else
    return asInt64();
#endif
}
LargestUInt Value::asLargestUInt() const
{
#ifdef JSON_NO_INT64
    return asUInt();
#else
    return asUInt64();
#endif
}
double Value::asDouble() const
{
    switch (type_)
    {
        case intValue:
            return static_cast<double>(value_.int_);
        case uintValue:
            return static_cast<double>(value_.uint_);
        case realValue:
            return value_.real_;
        case nullValue:
            return 0.0;
        case booleanValue:
            return value_.bool_ ? 1.0 : 0.0;
        default:
            break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to double.");
}
float Value::asFloat() const
{
    switch (type_)
    {
        case intValue:
            return static_cast<float>(value_.int_);
        case uintValue:
            return static_cast<float>(value_.uint_);
        case realValue:
            return static_cast<float>(value_.real_);
        case nullValue:
            return 0.f;
        case booleanValue:
            return value_.bool_ ? 1.f : 0.f;
        default:
            break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to float.");
}
bool Value::asBool() const
{
    switch (type_)
    {
        case booleanValue:
            return value_.bool_;
        case nullValue:
            return false;
        case intValue:
            return value_.int_ ? true : false;
        case uintValue:
            return value_.uint_ ? true : false;
        case realValue:
            return value_.real_ ? true : false;
        default:
            break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to bool.");
}

bool Value::isConvertibleTo(ValueType other) const
{
    switch (other)
    {
        case nullValue:
            return (isNumeric() && asDouble() == 0.0) || (type_ == booleanValue && value_.bool_ == false) ||
                   (type_ == stringValue && asString() == "") || (type_ == arrayValue && value_.map_->size() == 0) ||
                   (type_ == objectValue && value_.map_->size() == 0) || type_ == nullValue;
        case intValue:
            return isInt() || (type_ == realValue && InRange(value_.real_, minInt, maxInt)) || type_ == booleanValue ||
                   type_ == nullValue;
        case uintValue:
            return isUInt() || (type_ == realValue && InRange(value_.real_, 0, maxUInt)) || type_ == booleanValue ||
                   type_ == nullValue;
        case realValue:
            return isNumeric() || type_ == booleanValue || type_ == nullValue;
        case booleanValue:
            return isNumeric() || type_ == booleanValue || type_ == nullValue;
        case stringValue:
            return isNumeric() || type_ == booleanValue || type_ == stringValue || type_ == nullValue;
        case arrayValue:
            return type_ == arrayValue || type_ == nullValue;
        case objectValue:
            return type_ == objectValue || type_ == nullValue;
    }
    JSON_ASSERT_UNREACHABLE;
    return false;
}

ArrayIndex Value::size() const
{
    switch (type_)
    {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
        case stringValue:
            return 0;
        case objectValue:
            return ArrayIndex(value_.map_->size());

        case arrayValue:
        {
            ArrayIndex maxIndex = 0;
            bool       any      = false;
            for (const auto& kv : *value_.map_)
            {
                const ArrayIndex idx = kv.first.index() + 1;
                if (!any || idx > maxIndex)
                    maxIndex = idx;
                any = true;
            }
            return any ? maxIndex : 0;
        }
    }
    JSON_ASSERT_UNREACHABLE;
    return 0;
}

bool Value::empty() const
{
    if (isNull() || isArray() || isObject())
        return size() == 0u;
    return false;
}
bool Value::operator!() const
{
    return isNull();
}
void Value::clear()
{
    SSVOH_ASSERT(type_ == nullValue || type_ == arrayValue || type_ == objectValue);
    switch (type_)
    {
        case arrayValue:
        case objectValue:
            value_.map_->clear();
            break;
        default:
            break;
    }
}

void Value::resize(ArrayIndex newSize)
{
    SSVOH_ASSERT(type_ == nullValue || type_ == arrayValue);
    if (type_ == nullValue)
        *this = Value(arrayValue);
    ArrayIndex oldSize = size();
    if (newSize == 0)
        clear();
    else if (newSize > oldSize)
        (*this)[newSize - 1];
    else
    {
        for (ArrayIndex index = newSize; index < oldSize; ++index)
            value_.map_->erase(CZString(index));
        SSVOH_ASSERT(size() == newSize);
    }
}

Value& Value::operator[](ArrayIndex index)
{
    SSVOH_ASSERT(type_ == nullValue || type_ == arrayValue);
    if (type_ == nullValue)
        *this = Value(arrayValue);
    CZString key(index);
    auto     it = value_.map_->find(key);
    if (it != value_.map_->end())
        return it->second;
    auto inserted = value_.map_->try_emplace(key, nullJsonValue);
    return inserted.first->second;
}

Value& Value::operator[](int index)
{
    SSVOH_ASSERT(index >= 0);
    return (*this)[ArrayIndex(index)];
}

const Value& Value::operator[](ArrayIndex index) const
{
    SSVOH_ASSERT(type_ == nullValue || type_ == arrayValue);
    if (type_ == nullValue)
        return nullJsonValue;
    CZString key(index);
    auto     it = value_.map_->find(key);
    if (it == value_.map_->end())
        return nullJsonValue;
    return it->second;
}

const Value& Value::operator[](int index) const
{
    SSVOH_ASSERT(index >= 0);
    return (*this)[ArrayIndex(index)];
}

Value& Value::operator[](const char* key)
{
    return resolveReference(key, false);
}

Value& Value::resolveReference(const char* key, bool isStatic)
{
    SSVOH_ASSERT(type_ == nullValue || type_ == objectValue);
    if (type_ == nullValue)
        *this = Value(objectValue);
    CZString actualKey(key, isStatic ? CZString::noDuplication : CZString::duplicateOnCopy);
    auto     it = value_.map_->find(actualKey);
    if (it != value_.map_->end())
        return it->second;
    auto inserted = value_.map_->try_emplace(actualKey, nullJsonValue);
    return inserted.first->second;
}

Value Value::get(ArrayIndex index, const Value& defaultValue) const
{
    const Value* value = &((*this)[index]);
    return value == &nullJsonValue ? defaultValue : *value;
}

bool Value::isValidIndex(ArrayIndex index) const
{
    return index < size();
}

const Value& Value::operator[](const char* key) const
{
    SSVOH_ASSERT(type_ == nullValue || type_ == objectValue);
    if (type_ == nullValue)
        return nullJsonValue;
    CZString actualKey(key, CZString::noDuplication);
    auto     it = value_.map_->find(actualKey);
    if (it != value_.map_->end())
        return it->second;
    return nullJsonValue;
}

Value& Value::operator[](const sf::base::String& key)
{
    return (*this)[key.cStr()];
}
const Value& Value::operator[](const sf::base::String& key) const
{
    return (*this)[key.cStr()];
}
Value& Value::operator[](const StaticString& key)
{
    return resolveReference(key, true);
}

Value& Value::append(const Value& value)
{
    return (*this)[size()] = value;
}
Value Value::get(const char* key, const Value& defaultValue) const
{
    const Value* value = &((*this)[key]);
    return value == &nullJsonValue ? defaultValue : *value;
}
Value Value::get(const sf::base::String& key, const Value& defaultValue) const
{
    return get(key.cStr(), defaultValue);
}
Value Value::removeMember(const char* key)
{
    SSVOH_ASSERT(type_ == nullValue || type_ == objectValue);
    if (type_ == nullValue)
        return nullJsonValue;
    CZString actualKey(key, CZString::noDuplication);
    auto     it = value_.map_->find(actualKey);
    if (it == value_.map_->end())
        return nullJsonValue;
    Value old(it->second);
    value_.map_->erase(it);
    return old;
}
Value Value::removeMember(const sf::base::String& key)
{
    return removeMember(key.cStr());
}

bool Value::isMember(const char* key) const
{
    if (type_ == nullValue)
        return false;
    CZString actualKey(key, CZString::noDuplication);
    auto     it = value_.map_->find(actualKey);
    return it != value_.map_->end();
}
bool Value::isMember(const sf::base::String& key) const
{
    return isMember(key.cStr());
}

Value::Members Value::getMemberNames() const
{
    SSVOH_ASSERT(type_ == nullValue || type_ == objectValue);
    if (type_ == nullValue)
        return Value::Members();
    Members members;
    members.reserve(static_cast<sf::base::SizeT>(value_.map_->size()));
    for (const auto& kv : *value_.map_)
        members.emplaceBack(sf::base::String(kv.first.c_str()));
    return members;
}

bool Value::isNull() const
{
    return type_ == nullValue;
}
bool Value::isBool() const
{
    return type_ == booleanValue;
}
bool Value::isInt() const
{
    switch (type_)
    {
        case intValue:
            return value_.int_ >= minInt && value_.int_ <= maxInt;
        case uintValue:
            return value_.uint_ <= UInt(maxInt);
        case realValue:
            return value_.real_ >= minInt && value_.real_ <= maxInt && IsIntegral(value_.real_);
        default:
            break;
    }
    return false;
}
bool Value::isUInt() const
{
    switch (type_)
    {
        case intValue:
            return value_.int_ >= 0 && LargestUInt(value_.int_) <= LargestUInt(maxUInt);
        case uintValue:
            return value_.uint_ <= maxUInt;
        case realValue:
            return value_.real_ >= 0 && value_.real_ <= maxUInt && IsIntegral(value_.real_);
        default:
            break;
    }
    return false;
}
bool Value::isInt64() const
{
#ifdef JSON_HAS_INT64
    switch (type_)
    {
        case intValue:
            return true;
        case uintValue:
            return value_.uint_ <= UInt64(maxInt64);
        case realValue:
            return value_.real_ >= double(minInt64) && value_.real_ < double(maxInt64) && IsIntegral(value_.real_);
        default:
            break;
    }
#endif
    return false;
}
bool Value::isUInt64() const
{
#ifdef JSON_HAS_INT64
    switch (type_)
    {
        case intValue:
            return value_.int_ >= 0;
        case uintValue:
            return true;
        case realValue:
            return value_.real_ >= 0 && value_.real_ < maxUInt64AsDouble && IsIntegral(value_.real_);
        default:
            break;
    }
#endif
    return false;
}
bool Value::isIntegral() const
{
#ifdef JSON_HAS_INT64
    return isInt64() || isUInt64();
#else
    return isInt() || isUInt();
#endif
}
bool Value::isDouble() const
{
    return type_ == realValue || isIntegral();
}
bool Value::isNumeric() const
{
    return isIntegral() || isDouble();
}
bool Value::isString() const
{
    return type_ == stringValue;
}
bool Value::isArray() const
{
    return type_ == arrayValue;
}
bool Value::isObject() const
{
    return type_ == objectValue;
}
void Value::setComment(const char* comment, CommentPlacement placement)
{
    if (!comments_)
        comments_ = new CommentInfo[numberOfCommentPlacement];
    comments_[placement].setComment(comment);
}
void Value::setComment(const sf::base::String& comment, CommentPlacement placement)
{
    setComment(comment.cStr(), placement);
}
bool Value::hasComment(CommentPlacement placement) const
{
    return comments_ != nullptr && comments_[placement].comment_ != nullptr;
}
sf::base::String Value::getComment(CommentPlacement placement) const
{
    if (hasComment(placement))
        return sf::base::String(comments_[placement].comment_);
    return "";
}
sf::base::String Value::toStyledString() const
{
    StyledWriter writer;
    return writer.write(*this);
}
Value::const_iterator Value::begin() const
{
    switch (type_)
    {
        case arrayValue:
        case objectValue:
            if (value_.map_)
                return const_iterator(ObjectValuesIteratorImpl(const_cast<ObjectValuesImpl*>(value_.map_)->begin()));
            break;
        default:
            break;
    }
    return const_iterator();
}
Value::const_iterator Value::end() const
{
    switch (type_)
    {
        case arrayValue:
        case objectValue:
            if (value_.map_)
                return const_iterator(ObjectValuesIteratorImpl(const_cast<ObjectValuesImpl*>(value_.map_)->end()));
            break;
        default:
            break;
    }
    return const_iterator();
}
Value::iterator Value::begin()
{
    switch (type_)
    {
        case arrayValue:
        case objectValue:
            if (value_.map_)
                return iterator(ObjectValuesIteratorImpl(value_.map_->begin()));
            break;
        default:
            break;
    }
    return iterator();
}
Value::iterator Value::end()
{
    switch (type_)
    {
        case arrayValue:
        case objectValue:
            if (value_.map_)
                return iterator(ObjectValuesIteratorImpl(value_.map_->end()));
            break;
        default:
            break;
    }
    return iterator();
}

////////////////////////////////////////////////////////////
// PathArgument / Path
////////////////////////////////////////////////////////////

PathArgument::PathArgument() : key_(), index_(), kind_(kindNone)
{
}
PathArgument::PathArgument(ArrayIndex index) : key_(), index_(index), kind_(kindIndex)
{
}
PathArgument::PathArgument(const char* key) : key_(key), index_(), kind_(kindKey)
{
}
PathArgument::PathArgument(const sf::base::String& key) : key_(key.cStr()), index_(), kind_(kindKey)
{
}
Path::Path(const sf::base::String& path,
           const PathArgument&     a1,
           const PathArgument&     a2,
           const PathArgument&     a3,
           const PathArgument&     a4,
           const PathArgument&     a5)
{
    InArgs in;
    in.emplaceBack(&a1);
    in.emplaceBack(&a2);
    in.emplaceBack(&a3);
    in.emplaceBack(&a4);
    in.emplaceBack(&a5);
    makePath(path, in);
}

void Path::makePath(const sf::base::String& path, const InArgs& in)
{
    const char*            current = path.cStr();
    const char*            end     = current + path.size();
    InArgs::const_iterator itInArg = in.begin();
    while (current != end)
    {
        if (*current == '[')
        {
            ++current;
            if (*current == '%')
                addPathInArg(path, in, itInArg, PathArgument::kindIndex);
            else
            {
                ArrayIndex index = 0;
                for (; current != end && *current >= '0' && *current <= '9'; ++current)
                    index = index * 10 + ArrayIndex(*current - '0');
                args_.emplaceBack(index);
            }
            if (current == end || *current++ != ']')
                invalidPath(path, int(current - path.cStr()));
        }
        else if (*current == '%')
        {
            addPathInArg(path, in, itInArg, PathArgument::kindKey);
            ++current;
        }
        else if (*current == '.')
            ++current;
        else
        {
            const char* beginName = current;
            while (current != end && !strchr("[.", *current))
                ++current;
            args_.emplaceBack(sf::base::String(beginName, static_cast<sf::base::SizeT>(current - beginName)));
        }
    }
}

void Path::addPathInArg(const sf::base::String&, const InArgs& in, InArgs::const_iterator& itInArg, PathArgument::Kind kind)
{
    if (itInArg == in.end())
    {
    }
    else if ((*itInArg)->kind_ != kind)
    {
    }
    else
        args_.emplaceBack(**itInArg);
}

void Path::invalidPath(const sf::base::String&, int)
{
}

const Value& Path::resolve(const Value& root) const
{
    const Value* node = &root;
    for (auto it = args_.begin(); it != args_.end(); ++it)
    {
        const PathArgument& arg = *it;
        if (arg.kind_ == PathArgument::kindIndex)
            node = &((*node)[arg.index_]);
        else if (arg.kind_ == PathArgument::kindKey)
            node = &((*node)[arg.key_]);
    }
    return *node;
}

Value Path::resolve(const Value& root, const Value& defaultValue) const
{
    const Value* node = &root;
    for (auto it = args_.begin(); it != args_.end(); ++it)
    {
        const PathArgument& arg = *it;
        if (arg.kind_ == PathArgument::kindIndex)
        {
            if (!node->isArray() || !node->isValidIndex(arg.index_))
                return defaultValue;
            node = &((*node)[arg.index_]);
        }
        else if (arg.kind_ == PathArgument::kindKey)
        {
            if (!node->isObject())
                return defaultValue;
            node = &((*node)[arg.key_]);
            if (node == &nullJsonValue)
                return defaultValue;
        }
    }
    return *node;
}

Value& Path::make(Value& root) const
{
    Value* node = &root;
    for (auto it = args_.begin(); it != args_.end(); ++it)
    {
        const PathArgument& arg = *it;
        if (arg.kind_ == PathArgument::kindIndex)
            node = &((*node)[arg.index_]);
        else if (arg.kind_ == PathArgument::kindKey)
            node = &((*node)[arg.key_]);
    }
    return *node;
}

////////////////////////////////////////////////////////////
// valueTo* helpers
////////////////////////////////////////////////////////////

sf::base::String valueToString(LargestInt value)
{
    UIntToStringBuffer buffer;
    char*              current    = buffer + sizeof(buffer);
    bool               isNegative = value < 0;
    if (isNegative)
        value = -value;
    uintToString(LargestUInt(value), current);
    if (isNegative)
        *--current = '-';
    SSVOH_ASSERT(current >= buffer);
    return sf::base::String(current);
}

sf::base::String valueToString(LargestUInt value)
{
    UIntToStringBuffer buffer;
    char*              current = buffer + sizeof(buffer);
    uintToString(value, current);
    SSVOH_ASSERT(current >= buffer);
    return sf::base::String(current);
}

#ifdef JSON_HAS_INT64
sf::base::String valueToString(int value)
{
    return valueToString(LargestInt(value));
}
sf::base::String valueToString(UInt value)
{
    return valueToString(LargestUInt(value));
}
#endif

sf::base::String valueToString(double value)
{
    char buffer[32];
    sprintf(buffer, "%#.16g", value);
    char* ch = buffer + strlen(buffer) - 1;
    if (*ch != '0')
        return sf::base::String(buffer);
    while (ch > buffer && *ch == '0')
        --ch;
    char* last_nonzero = ch;
    while (ch >= buffer)
    {
        switch (*ch)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                --ch;
                continue;
            case '.':
                *(last_nonzero + 2) = '\0';
                return sf::base::String(buffer);
            default:
                return sf::base::String(buffer);
        }
    }
    return sf::base::String(buffer);
}

sf::base::String valueToString(bool value)
{
    return value ? "true" : "false";
}

sf::base::String valueToQuotedString(const char* value)
{
    if (value == nullptr)
        return "";
    if (strpbrk(value, "\"\\\b\f\n\r\t") == nullptr && !containsControlCharacter(value))
        return sf::base::String("\"") + value + "\"";

    sf::base::SizeT  maxsize = strlen(value) * 2 + 3;
    sf::base::String result;
    result.reserve(static_cast<sf::base::SizeT>(maxsize));
    result += "\"";
    for (const char* c = value; *c != 0; ++c)
    {
        switch (*c)
        {
            case '\"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (isControlCharacter(*c))
                {
                    static constexpr char kHex[] = "0123456789ABCDEF";
                    const unsigned int    v      = static_cast<unsigned char>(*c);
                    char buf[6] = {'\\', 'u', kHex[(v >> 12) & 0xF], kHex[(v >> 8) & 0xF], kHex[(v >> 4) & 0xF], kHex[v & 0xF]};
                    result.append(buf, sizeof(buf));
                }
                else
                    result += *c;
                break;
        }
    }
    result += "\"";
    return result;
}

////////////////////////////////////////////////////////////
// Writer / FastWriter / StyledWriter
////////////////////////////////////////////////////////////

Writer::~Writer() = default;

FastWriter::FastWriter() : yamlCompatiblityEnabled_(false), dropNullPlaceholders_(false)
{
}

void FastWriter::enableYAMLCompatibility()
{
    yamlCompatiblityEnabled_ = true;
}

void FastWriter::dropNullPlaceholders()
{
    dropNullPlaceholders_ = true;
}

sf::base::String FastWriter::write(const Value& root)
{
    document_ = "";
    writeValue(root);
    document_ += "\n";
    return document_;
}

void FastWriter::writeValue(const Value& value)
{
    switch (value.type())
    {
        case nullValue:
            if (!dropNullPlaceholders_)
                document_ += "null";
            break;
        case intValue:
            document_ += valueToString(value.asLargestInt());
            break;
        case uintValue:
            document_ += valueToString(value.asLargestUInt());
            break;
        case realValue:
            document_ += valueToString(value.asDouble());
            break;
        case stringValue:
            document_ += valueToQuotedString(value.asCString());
            break;
        case booleanValue:
            document_ += valueToString(value.asBool());
            break;
        case arrayValue:
        {
            document_ += "[";
            int sz = value.size();
            for (int index = 0; index < sz; ++index)
            {
                if (index > 0)
                    document_ += ",";
                writeValue(value[index]);
            }
            document_ += "]";
        }
        break;
        case objectValue:
        {
            Value::Members members(value.getMemberNames());
            document_ += "{";
            for (auto it = members.begin(); it != members.end(); ++it)
            {
                const sf::base::String& name = *it;
                if (it != members.begin())
                    document_ += ",";
                document_ += valueToQuotedString(name.cStr());
                document_ += yamlCompatiblityEnabled_ ? ": " : ":";
                writeValue(value[name]);
            }
            document_ += "}";
        }
        break;
    }
}

StyledWriter::StyledWriter() : rightMargin_(74), indentSize_(3), addChildValues_()
{
}

sf::base::String StyledWriter::write(const Value& root)
{
    document_       = "";
    addChildValues_ = false;
    indentString_   = "";
    writeCommentBeforeValue(root);
    writeValue(root);
    writeCommentAfterValueOnSameLine(root);
    document_ += "\n";
    return document_;
}

void StyledWriter::writeValue(const Value& value)
{
    switch (value.type())
    {
        case nullValue:
            pushValue("null");
            break;
        case intValue:
            pushValue(valueToString(value.asLargestInt()));
            break;
        case uintValue:
            pushValue(valueToString(value.asLargestUInt()));
            break;
        case realValue:
            pushValue(valueToString(value.asDouble()));
            break;
        case stringValue:
            pushValue(valueToQuotedString(value.asCString()));
            break;
        case booleanValue:
            pushValue(valueToString(value.asBool()));
            break;
        case arrayValue:
            writeArrayValue(value);
            break;
        case objectValue:
        {
            Value::Members members(value.getMemberNames());
            if (members.empty())
                pushValue("{}");
            else
            {
                writeWithIndent("{");
                indent();
                auto it = members.begin();
                while (true)
                {
                    const sf::base::String& name       = *it;
                    const Value&            childValue = value[name];
                    writeCommentBeforeValue(childValue);
                    writeWithIndent(valueToQuotedString(name.cStr()));
                    document_ += " : ";
                    writeValue(childValue);
                    if (++it == members.end())
                    {
                        writeCommentAfterValueOnSameLine(childValue);
                        break;
                    }
                    document_ += ",";
                    writeCommentAfterValueOnSameLine(childValue);
                }
                unindent();
                writeWithIndent("}");
            }
        }
        break;
    }
}

void StyledWriter::writeArrayValue(const Value& value)
{
    unsigned sz = value.size();
    if (sz == 0)
        pushValue("[]");
    else
    {
        bool isArrayMultiLine = isMultineArray(value);
        if (isArrayMultiLine)
        {
            writeWithIndent("[");
            indent();
            bool     hasChildValue = !childValues_.empty();
            unsigned index         = 0;
            while (true)
            {
                const Value& childValue = value[index];
                writeCommentBeforeValue(childValue);
                if (hasChildValue)
                    writeWithIndent(childValues_[index]);
                else
                {
                    writeIndent();
                    writeValue(childValue);
                }
                if (++index == sz)
                {
                    writeCommentAfterValueOnSameLine(childValue);
                    break;
                }
                document_ += ",";
                writeCommentAfterValueOnSameLine(childValue);
            }
            unindent();
            writeWithIndent("]");
        }
        else
        {
            SSVOH_ASSERT(childValues_.size() == sz);
            document_ += "[ ";
            for (unsigned index = 0; index < sz; ++index)
            {
                if (index > 0)
                    document_ += ", ";
                document_ += childValues_[index];
            }
            document_ += " ]";
        }
    }
}

bool StyledWriter::isMultineArray(const Value& value)
{
    int  sz          = value.size();
    bool isMultiLine = sz * 3 >= rightMargin_;
    childValues_.clear();
    for (int index = 0; index < sz && !isMultiLine; ++index)
    {
        const Value& childValue = value[index];
        isMultiLine = isMultiLine || ((childValue.isArray() || childValue.isObject()) && childValue.size() > 0);
    }
    if (!isMultiLine)
    {
        childValues_.reserve(sz);
        addChildValues_ = true;
        int lineLength  = 4 + (sz - 1) * 2;
        for (int index = 0; index < sz && !isMultiLine; ++index)
        {
            writeValue(value[index]);
            lineLength += int(childValues_[index].size());
            isMultiLine = isMultiLine && hasCommentForValue(value[index]);
        }
        addChildValues_ = false;
        isMultiLine     = isMultiLine || lineLength >= rightMargin_;
    }
    return isMultiLine;
}

void StyledWriter::pushValue(const sf::base::String& value)
{
    if (addChildValues_)
        childValues_.emplaceBack(value);
    else
        document_ += value;
}

void StyledWriter::writeIndent()
{
    if (!document_.empty())
    {
        char last = document_[document_.size() - 1];
        if (last == ' ')
            return;
        if (last != '\n')
            document_ += '\n';
    }
    document_ += indentString_;
}

void StyledWriter::writeWithIndent(const sf::base::String& value)
{
    writeIndent();
    document_ += value;
}

void StyledWriter::indent()
{
    indentString_ += sf::base::String(std::string(indentSize_, ' '));
}

void StyledWriter::unindent()
{
    SSVOH_ASSERT(int(indentString_.size()) >= indentSize_);
    indentString_.resize(indentString_.size() - indentSize_);
}

void StyledWriter::writeCommentBeforeValue(const Value& root)
{
    if (!root.hasComment(commentBefore))
        return;
    document_ += normalizeEOL(root.getComment(commentBefore));
    document_ += "\n";
}

void StyledWriter::writeCommentAfterValueOnSameLine(const Value& root)
{
    if (root.hasComment(commentAfterOnSameLine))
        document_ += sf::base::String(" ") + normalizeEOL(root.getComment(commentAfterOnSameLine));
    if (root.hasComment(commentAfter))
    {
        document_ += "\n";
        document_ += normalizeEOL(root.getComment(commentAfter));
        document_ += "\n";
    }
}

bool StyledWriter::hasCommentForValue(const Value& value)
{
    return value.hasComment(commentBefore) || value.hasComment(commentAfterOnSameLine) || value.hasComment(commentAfter);
}

sf::base::String StyledWriter::normalizeEOL(const sf::base::String& text)
{
    sf::base::String normalized;
    normalized.reserve(text.size());
    const char* begin   = text.cStr();
    const char* end     = begin + text.size();
    const char* current = begin;
    while (current != end)
    {
        char c = *current++;
        if (c == '\r')
        {
            if (*current == '\n')
                ++current;
            normalized += '\n';
        }
        else
            normalized += c;
    }
    return normalized;
}

////////////////////////////////////////////////////////////
// StyledStreamWriter (in JsonStream.hpp)
////////////////////////////////////////////////////////////

StyledStreamWriter::StyledStreamWriter(sf::base::String indentation) :
    document_(nullptr),
    rightMargin_(74),
    indentation_(indentation),
    addChildValues_()
{
}

void StyledStreamWriter::write(std::ostream& out, const Value& root)
{
    document_       = &out;
    addChildValues_ = false;
    indentString_   = "";
    writeCommentBeforeValue(root);
    writeValue(root);
    writeCommentAfterValueOnSameLine(root);
    *document_ << '\n';
    document_ = nullptr;
}

namespace
{
inline std::ostream& writeStr(std::ostream& os, const sf::base::String& s)
{
    return os.write(s.data(), static_cast<std::streamsize>(s.size()));
}
} // namespace

void StyledStreamWriter::writeValue(const Value& value)
{
    switch (value.type())
    {
        case nullValue:
            pushValue("null");
            break;
        case intValue:
            pushValue(valueToString(value.asLargestInt()));
            break;
        case uintValue:
            pushValue(valueToString(value.asLargestUInt()));
            break;
        case realValue:
            pushValue(valueToString(value.asDouble()));
            break;
        case stringValue:
            pushValue(valueToQuotedString(value.asCString()));
            break;
        case booleanValue:
            pushValue(valueToString(value.asBool()));
            break;
        case arrayValue:
            writeArrayValue(value);
            break;
        case objectValue:
        {
            Value::Members members(value.getMemberNames());
            if (members.empty())
                pushValue("{}");
            else
            {
                writeWithIndent("{");
                indent();
                auto it = members.begin();
                while (true)
                {
                    const sf::base::String& name       = *it;
                    const Value&            childValue = value[name];
                    writeCommentBeforeValue(childValue);
                    writeWithIndent(valueToQuotedString(name.cStr()));
                    *document_ << " : ";
                    writeValue(childValue);
                    if (++it == members.end())
                    {
                        writeCommentAfterValueOnSameLine(childValue);
                        break;
                    }
                    *document_ << ",";
                    writeCommentAfterValueOnSameLine(childValue);
                }
                unindent();
                writeWithIndent("}");
            }
        }
        break;
    }
}

void StyledStreamWriter::writeArrayValue(const Value& value)
{
    unsigned sz = value.size();
    if (sz == 0)
        pushValue("[]");
    else
    {
        bool isArrayMultiLine = isMultineArray(value);
        if (isArrayMultiLine)
        {
            writeWithIndent("[");
            indent();
            bool     hasChildValue = !childValues_.empty();
            unsigned index         = 0;
            while (true)
            {
                const Value& childValue = value[index];
                writeCommentBeforeValue(childValue);
                if (hasChildValue)
                    writeWithIndent(childValues_[index]);
                else
                {
                    writeIndent();
                    writeValue(childValue);
                }
                if (++index == sz)
                {
                    writeCommentAfterValueOnSameLine(childValue);
                    break;
                }
                *document_ << ",";
                writeCommentAfterValueOnSameLine(childValue);
            }
            unindent();
            writeWithIndent("]");
        }
        else
        {
            SSVOH_ASSERT(childValues_.size() == sz);
            *document_ << "[ ";
            for (unsigned index = 0; index < sz; ++index)
            {
                if (index > 0)
                    *document_ << ", ";
                writeStr(*document_, childValues_[index]);
            }
            *document_ << " ]";
        }
    }
}

bool StyledStreamWriter::isMultineArray(const Value& value)
{
    int  sz          = value.size();
    bool isMultiLine = sz * 3 >= rightMargin_;
    childValues_.clear();
    for (int index = 0; index < sz && !isMultiLine; ++index)
    {
        const Value& childValue = value[index];
        isMultiLine = isMultiLine || ((childValue.isArray() || childValue.isObject()) && childValue.size() > 0);
    }
    if (!isMultiLine)
    {
        childValues_.reserve(sz);
        addChildValues_ = true;
        int lineLength  = 4 + (sz - 1) * 2;
        for (int index = 0; index < sz && !isMultiLine; ++index)
        {
            writeValue(value[index]);
            lineLength += int(childValues_[index].size());
            isMultiLine = isMultiLine && hasCommentForValue(value[index]);
        }
        addChildValues_ = false;
        isMultiLine     = isMultiLine || lineLength >= rightMargin_;
    }
    return isMultiLine;
}

void StyledStreamWriter::pushValue(const sf::base::String& value)
{
    if (addChildValues_)
        childValues_.emplaceBack(value);
    else
        writeStr(*document_, value);
}

void StyledStreamWriter::writeIndent()
{
    *document_ << '\n';
    writeStr(*document_, indentString_);
}

void StyledStreamWriter::writeWithIndent(const sf::base::String& value)
{
    writeIndent();
    writeStr(*document_, value);
}

void StyledStreamWriter::indent()
{
    indentString_ += indentation_;
}

void StyledStreamWriter::unindent()
{
    SSVOH_ASSERT(indentString_.size() >= indentation_.size());
    indentString_.resize(indentString_.size() - indentation_.size());
}

void StyledStreamWriter::writeCommentBeforeValue(const Value& root)
{
    if (!root.hasComment(commentBefore))
        return;
    writeStr(*document_, normalizeEOL(root.getComment(commentBefore)));
    *document_ << '\n';
}

void StyledStreamWriter::writeCommentAfterValueOnSameLine(const Value& root)
{
    if (root.hasComment(commentAfterOnSameLine))
    {
        *document_ << ' ';
        writeStr(*document_, normalizeEOL(root.getComment(commentAfterOnSameLine)));
    }
    if (root.hasComment(commentAfter))
    {
        *document_ << '\n';
        writeStr(*document_, normalizeEOL(root.getComment(commentAfter)));
        *document_ << '\n';
    }
}

bool StyledStreamWriter::hasCommentForValue(const Value& value)
{
    return value.hasComment(commentBefore) || value.hasComment(commentAfterOnSameLine) || value.hasComment(commentAfter);
}

sf::base::String StyledStreamWriter::normalizeEOL(const sf::base::String& text)
{
    sf::base::String normalized;
    normalized.reserve(text.size());
    const char* begin   = text.cStr();
    const char* end     = begin + text.size();
    const char* current = begin;
    while (current != end)
    {
        char c = *current++;
        if (c == '\r')
        {
            if (*current == '\n')
                ++current;
            normalized += '\n';
        }
        else
            normalized += c;
    }
    return normalized;
}

std::ostream& operator<<(std::ostream& sout, const Value& root)
{
    Json::StyledStreamWriter writer;
    writer.write(sout, root);
    return sout;
}

} // namespace Json
