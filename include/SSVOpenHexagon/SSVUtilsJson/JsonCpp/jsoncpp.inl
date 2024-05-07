// Copyright(c) 2013 Vittorio Romeo
// License: Academic Free License("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#ifndef SSVUJ_OH_JSONCPP_INL
#define SSVUJ_OH_JSONCPP_INL

// Include as system header to suppress dependency warnings.
#pragma GCC system_header

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <cstring>
#include <cmath>
#include <iomanip>
#include <sstream>

#define JSON_ASSERT_UNREACHABLE SSVOH_ASSERT(false)

namespace Json {
inline std::string codePointToUTF8(unsigned int cp)
{
    std::string result;
    if(cp <= 0x7f)
    {
        result.resize(1);
        result[0] = static_cast<char>(cp);
    }
    else if(cp <= 0x7FF)
    {
        result.resize(2);
        result[1] = static_cast<char>(0x80 | (0x3f & cp));
        result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
    }
    else if(cp <= 0xFFFF)
    {
        result.resize(3);
        result[2] = static_cast<char>(0x80 | (0x3f & cp));
        result[1] = 0x80 | static_cast<char>((0x3f & (cp >> 6)));
        result[0] = 0xE0 | static_cast<char>((0xf & (cp >> 12)));
    }
    else if(cp <= 0x10FFFF)
    {
        result.resize(4);
        result[3] = static_cast<char>(0x80 | (0x3f & cp));
        result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
        result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
    }
    return result;
}

enum
{
    uintToStringBufferSize = 3 * sizeof(LargestUInt) + 1
};
typedef char UIntToStringBuffer[uintToStringBufferSize];
inline void uintToString(LargestUInt value, char*& current)
{
    *--current = 0;
    do
    {
        *--current = char(value % 10) + '0';
        value /= 10;
    }
    while(value != 0);
}


inline bool containsNewLine(Reader::Location begin, Reader::Location end)
{
    for(; begin < end; ++begin)
        if(*begin == '\n' || *begin == '\r') return true;
    return false;
}
inline Reader::Reader()
    : errors_(),
      document_(),
      begin_(),
      end_(),
      current_(),
      lastValueEnd_(),
      lastValue_(),
      commentsBefore_(),
      features_(Features::all()),
      collectComments_()
{}
inline Reader::Reader(const Features& features)
    : errors_(),
      document_(),
      begin_(),
      end_(),
      current_(),
      lastValueEnd_(),
      lastValue_(),
      commentsBefore_(),
      features_(features),
      collectComments_()
{}
inline bool Reader::parse(
    const std::string& document, Value& root, bool collectComments)
{
    document_ = document;
    const char* begin = document_.c_str();
    const char* end = begin + document_.size();
    return parse(begin, end, root, collectComments);
}
inline bool Reader::parse(std::istream& sin, Value& root, bool collectComments)
{
    std::string doc;
    std::getline(sin, doc, (char)EOF);
    return parse(doc, root, collectComments);
}
inline bool Reader::parse(
    const char* beginDoc, const char* endDoc, Value& root, bool collectComments)
{
    if(!features_.allowComments_) collectComments = false;
    begin_ = beginDoc;
    end_ = endDoc;
    collectComments_ = collectComments;
    current_ = begin_;
    lastValueEnd_ = nullptr;
    lastValue_ = nullptr;
    commentsBefore_ = "";
    errors_.clear();
    while(!nodes_.empty()) nodes_.pop();
    nodes_.push(&root);
    bool successful = readValue();
    Token token;
    skipCommentTokens(token);
    if(collectComments_ && !commentsBefore_.empty())
        root.setComment(commentsBefore_, commentAfter);
    if(features_.strictRoot_)
    {
        if(!root.isArray() && !root.isObject())
        {
            token.type_ = tokenError;
            token.start_ = beginDoc;
            token.end_ = endDoc;
            addError(
                "A valid JSON document must be either an array or an "
                "object value.",
                token);
            return false;
        }
    }
    return successful;
}
inline bool Reader::readValue()
{
    Token token;
    skipCommentTokens(token);
    bool successful{true};
    if(collectComments_ && !commentsBefore_.empty())
    {
        currentValue().setComment(commentsBefore_, commentBefore);
        commentsBefore_ = "";
    }
    switch(token.type_)
    {
        case tokenObjectBegin: successful = readObject(token); break;
        case tokenArrayBegin: successful = readArray(token); break;
        case tokenNumber: successful = decodeNumber(token); break;
        case tokenString: successful = decodeString(token); break;
        case tokenTrue: currentValue() = true; break;
        case tokenFalse: currentValue() = false; break;
        case tokenNull: currentValue() = Value(); break;
        default:
            return addError(
                "Syntax error: value, object or array expected.", token);
    }
    if(collectComments_)
    {
        lastValueEnd_ = current_;
        lastValue_ = &currentValue();
    }
    return successful;
}
inline void Reader::skipCommentTokens(Token& token)
{
    if(features_.allowComments_)
    {
        do
        {
            readToken(token);
        }
        while(token.type_ == tokenComment);
    }
    else
        readToken(token);
}
inline bool Reader::expectToken(
    TokenType type, Token& token, const char* message)
{
    readToken(token);
    if(token.type_ != type) return addError(message, token);
    return true;
}
inline bool Reader::readToken(Token& token)
{
    skipSpaces();
    token.start_ = current_;
    char c{getNextChar()};
    bool ok{true};
    switch(c)
    {
        case '{': token.type_ = tokenObjectBegin; break;
        case '}': token.type_ = tokenObjectEnd; break;
        case '[': token.type_ = tokenArrayBegin; break;
        case ']': token.type_ = tokenArrayEnd; break;
        case '"':
            token.type_ = tokenString;
            ok = readString();
            break;
        case '/':
            token.type_ = tokenComment;
            ok = readComment();
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
            token.type_ = tokenNumber;
            readNumber();
            break;
        case 't':
            token.type_ = tokenTrue;
            ok = match("rue", 3);
            break;
        case 'f':
            token.type_ = tokenFalse;
            ok = match("alse", 4);
            break;
        case 'n':
            token.type_ = tokenNull;
            ok = match("ull", 3);
            break;
        case ',': token.type_ = tokenArraySeparator; break;
        case ':': token.type_ = tokenMemberSeparator; break;
        case 0: token.type_ = tokenEndOfStream; break;
        default: ok = false; break;
    }
    if(!ok) token.type_ = tokenError;
    token.end_ = current_;
    return true;
}
inline void Reader::skipSpaces()
{
    while(current_ != end_)
    {
        char c{*current_};
        if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
            ++current_;
        else
            break;
    }
}
inline bool Reader::match(Location pattern, int patternLength)
{
    if(end_ - current_ < patternLength) return false;
    int index = patternLength;
    while(index--)
        if(current_[index] != pattern[index]) return false;
    current_ += patternLength;
    return true;
}
inline bool Reader::readComment()
{
    Location commentBegin = current_ - 1;
    char c = getNextChar();
    bool successful = false;
    if(c == '*')
        successful = readCStyleComment();
    else if(c == '/')
        successful = readCppStyleComment();
    if(!successful) return false;
    if(collectComments_)
    {
        CommentPlacement placement = commentBefore;
        if(lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin))
            if(c != '*' || !containsNewLine(commentBegin, current_))
                placement = commentAfterOnSameLine;
        addComment(commentBegin, current_, placement);
    }
    return true;
}
inline void Reader::addComment(
    Location begin, Location end, CommentPlacement placement)
{
    SSVOH_ASSERT(collectComments_);
    if(placement == commentAfterOnSameLine)
    {
        SSVOH_ASSERT(lastValue_ != 0);
        lastValue_->setComment(std::string(begin, end), placement);
    }
    else
    {
        if(!commentsBefore_.empty()) commentsBefore_ += "\n";
        commentsBefore_ += std::string(begin, end);
    }
}
inline bool Reader::readCStyleComment()
{
    while(current_ != end_)
    {
        char c = getNextChar();
        if(c == '*' && *current_ == '/') break;
    }
    return getNextChar() == '/';
}
inline bool Reader::readCppStyleComment()
{
    while(current_ != end_)
    {
        char c = getNextChar();
        if(c == '\r' || c == '\n') break;
    }
    return true;
}
inline void Reader::readNumber()
{
    while(current_ != end_)
    {
        if(!(*current_ >= '0' && *current_ <= '9') &&
            !in(*current_, '.', 'e', 'E', '+', '-'))
            break;
        ++current_;
    }
}
inline bool Reader::readString()
{
    char c = 0;
    while(current_ != end_)
    {
        c = getNextChar();
        if(c == '\\')
            getNextChar();
        else if(c == '"')
            break;
    }
    return c == '"';
}
inline bool Reader::readObject(Token&)
{
    Token tokenName;
    std::string name;
    currentValue() = Value(objectValue);
    while(readToken(tokenName))
    {
        bool initialTokenOk = true;
        while(tokenName.type_ == tokenComment && initialTokenOk)
            initialTokenOk = readToken(tokenName);
        if(!initialTokenOk) break;
        if(tokenName.type_ == tokenObjectEnd && name.empty()) return true;
        if(tokenName.type_ != tokenString) break;
        name = "";
        if(!decodeString(tokenName, name))
            return recoverFromError(tokenObjectEnd);
        Token colon;
        if(!readToken(colon) || colon.type_ != tokenMemberSeparator)
        {
            return addErrorAndRecover(
                "Missing ':' after object member name", colon, tokenObjectEnd);
        }
        Value& value = currentValue()[name];
        nodes_.push(&value);
        bool ok = readValue();
        nodes_.pop();
        if(!ok) return recoverFromError(tokenObjectEnd);
        Token comma;
        if(!readToken(comma) || (comma.type_ != tokenObjectEnd &&
                                    comma.type_ != tokenArraySeparator &&
                                    comma.type_ != tokenComment))
        {
            return addErrorAndRecover(
                "Missing ',' or '}' in object declaration", comma,
                tokenObjectEnd);
        }
        bool finalizeTokenOk = true;
        while(comma.type_ == tokenComment && finalizeTokenOk)
            finalizeTokenOk = readToken(comma);
        if(comma.type_ == tokenObjectEnd) return true;
    }
    return addErrorAndRecover(
        "Missing '}' or object member name", tokenName, tokenObjectEnd);
}
inline bool Reader::readArray(Token&)
{
    currentValue() = Value(arrayValue);
    skipSpaces();
    if(*current_ == ']')
    {
        Token endArray;
        readToken(endArray);
        return true;
    }
    int index{0};
    while(true)
    {
        Value& value = currentValue()[index++];
        nodes_.push(&value);
        bool ok = readValue();
        nodes_.pop();
        if(!ok) return recoverFromError(tokenArrayEnd);
        Token token;
        ok = readToken(token);
        while(token.type_ == tokenComment && ok)
        {
            ok = readToken(token);
        }
        bool badTokenType = (token.type_ != tokenArraySeparator &&
                             token.type_ != tokenArrayEnd);
        if(!ok || badTokenType)
        {
            return addErrorAndRecover("Missing ',' or ']' in array declaration",
                token, tokenArrayEnd);
        }
        if(token.type_ == tokenArrayEnd) break;
    }
    return true;
}
inline bool Reader::decodeNumber(Token& token)
{
    bool isDouble = false;
    for(Location inspect = token.start_; inspect != token.end_; ++inspect)
    {
        isDouble = isDouble || in(*inspect, '.', 'e', 'E', '+') ||
                   (*inspect == '-' && inspect != token.start_);
    }
    if(isDouble) return decodeDouble(token);
    Location current = token.start_;
    bool isNegative = *current == '-';
    if(isNegative) ++current;
    LargestUInt maxIntegerValue =
        isNegative ? LargestUInt(Value::minLargestInt) : Value::maxLargestUInt;
    LargestUInt threshold = maxIntegerValue / 10;
    LargestUInt value = 0;
    while(current < token.end_)
    {
        char c = *current++;
        if(c < '0' || c > '9')
            return addError("'" + std::string(token.start_, token.end_) +
                                "' is not a number.",
                token);
        UInt digit(c - '0');
        if(value >= threshold)
        {
            if(value > threshold || current != token.end_ ||
                digit > maxIntegerValue % 10)
                return decodeDouble(token);
        }
        value = value * 10 + digit;
    }
    if(isNegative)
        currentValue() = -LargestInt(value);
    else if(value <= LargestUInt(Value::maxInt))
        currentValue() = LargestInt(value);
    else
        currentValue() = value;
    return true;
}
inline bool Reader::decodeDouble(Token& token)
{
    double value = 0;
    const int bufferSize = 32;
    int count;
    int length = int(token.end_ - token.start_);
    if(length < 0)
    {
        return addError("Unable to parse token length", token);
    }
    char format[]{"%lf"};
    if(length <= bufferSize)
    {
        char buffer[bufferSize + 1];
        memcpy(buffer, token.start_, length);
        buffer[length] = 0;
        count = sscanf(buffer, format, &value);
    }
    else
    {
        std::string buffer(token.start_, token.end_);
        count = sscanf(buffer.c_str(), format, &value);
    }
    if(count != 1)
        return addError(
            "'" + std::string(token.start_, token.end_) + "' is not a number.",
            token);
    currentValue() = value;
    return true;
}
inline bool Reader::decodeString(Token& token)
{
    std::string decoded;
    if(!decodeString(token, decoded)) return false;
    currentValue() = decoded;
    return true;
}
inline bool Reader::decodeString(Token& token, std::string& decoded)
{
    decoded.reserve(token.end_ - token.start_ - 2);
    Location current = token.start_ + 1;
    Location end = token.end_ - 1;
    while(current != end)
    {
        char c = *current++;
        if(c == '"')
            break;
        else if(c == '\\')
        {
            if(current == end)
                return addError(
                    "Empty escape sequence in std::string", token, current);
            char escape = *current++;
            switch(escape)
            {
                case '"': decoded += '"'; break;
                case '/': decoded += '/'; break;
                case '\\': decoded += '\\'; break;
                case 'b': decoded += '\b'; break;
                case 'f': decoded += '\f'; break;
                case 'n': decoded += '\n'; break;
                case 'r': decoded += '\r'; break;
                case 't': decoded += '\t'; break;
                case 'u':
                {
                    unsigned int unicode{0u};
                    if(!decodeUnicodeCodePoint(token, current, end, unicode))
                        return false;
                    decoded += codePointToUTF8(unicode);
                }
                break;
                default:
                    return addError(
                        "Bad escape sequence in std::string", token, current);
            }
        }
        else
        {
            decoded += c;
        }
    }
    return true;
}
inline bool Reader::decodeUnicodeCodePoint(
    Token& token, Location& current, Location end, unsigned int& unicode)
{
    if(!decodeUnicodeEscapeSequence(token, current, end, unicode)) return false;
    if(unicode >= 0xD800 && unicode <= 0xDBFF)
    {
        if(end - current < 6)
            return addError(
                "additional six characters expected to parse unicode "
                "surrogate "
                "pair.",
                token, current);
        unsigned int surrogatePair{};
        if(*(current++) == '\\' && *(current++) == 'u')
        {
            if(decodeUnicodeEscapeSequence(token, current, end, surrogatePair))
            {
                unicode = 0x10000 + ((unicode & 0x3FF) << 10) +
                          (surrogatePair & 0x3FF);
            }
            else
                return false;
        }
        else
            return addError(
                "expecting another \\u token to begin the second half of a "
                "unicode "
                "surrogate pair",
                token, current);
    }
    return true;
}
inline bool Reader::decodeUnicodeEscapeSequence(
    Token& token, Location& current, Location end, unsigned int& unicode)
{
    if(end - current < 4)
        return addError(
            "Bad unicode escape sequence in std::string: four digits "
            "expected.",
            token, current);
    unicode = 0;
    for(int index = 0; index < 4; ++index)
    {
        char c = *current++;
        unicode *= 16;
        if(c >= '0' && c <= '9')
            unicode += c - '0';
        else if(c >= 'a' && c <= 'f')
            unicode += c - 'a' + 10;
        else if(c >= 'A' && c <= 'F')
            unicode += c - 'A' + 10;
        else
            return addError(
                "Bad unicode escape sequence in std::string: hexadecimal "
                "digit "
                "expected.",
                token, current);
    }
    return true;
}
inline bool Reader::addError(
    const std::string& message, Token& token, Location extra)
{
    ErrorInfo info;
    info.token_ = token;
    info.message_ = message;
    info.extra_ = extra;
    errors_.emplace_back(info);
    return false;
}
inline bool Reader::recoverFromError(TokenType skipUntilToken)
{
    int errorCount = int(errors_.size());
    Token skip;
    while(true)
    {
        if(!readToken(skip)) errors_.resize(errorCount);
        if(skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
            break;
    }
    errors_.resize(errorCount);
    return false;
}
inline bool Reader::addErrorAndRecover(
    const std::string& message, Token& token, TokenType skipUntilToken)
{
    addError(message, token);
    return recoverFromError(skipUntilToken);
}
inline Value& Reader::currentValue()
{
    return *(nodes_.top());
}
inline char Reader::getNextChar()
{
    if(current_ == end_) return 0;
    return *current_++;
}
inline void Reader::getLocationLineAndColumn(
    Location location, int& line, int& column) const
{
    Location current = begin_;
    Location lastLineStart = current;
    line = 0;
    while(current < location && current != end_)
    {
        char c = *current++;
        if(c == '\r')
        {
            if(*current == '\n') ++current;
            lastLineStart = current;
            ++line;
        }
        else if(c == '\n')
        {
            lastLineStart = current;
            ++line;
        }
    }
    column = int(location - lastLineStart) + 1;
    ++line;
}
inline std::string Reader::getLocationLineAndColumn(Location location) const
{
    int line, column;
    getLocationLineAndColumn(location, line, column);
    char buffer[18 + 16 + 16 + 1];
    sprintf(buffer, "Line %d, Column %d", line, column);
    return buffer;
}
inline std::string Reader::getFormattedErrorMessages() const
{
    std::string formattedMessage;
    for(Errors::const_iterator itError = errors_.begin();
        itError != errors_.end(); ++itError)
    {
        const ErrorInfo& error = *itError;
        formattedMessage +=
            "* " + getLocationLineAndColumn(error.token_.start_) + "\n";
        formattedMessage += "  " + error.message_ + "\n";
        if(error.extra_)
            formattedMessage += "See " +
                                getLocationLineAndColumn(error.extra_) +
                                " for detail.\n";
    }
    return formattedMessage;
}
inline std::istream& operator>>(std::istream& sin, Value& root)
{
    Json::Reader reader;
    bool ok = reader.parse(sin, root, true);
    if(!ok)
    {
        fprintf(stderr, "Error from reader: %s",
            reader.getFormattedErrorMessages().c_str());
        JSON_FAIL_MESSAGE("reader error");
    }
    return sin;
}

template <typename AllocatedType, const unsigned int objectPerAllocation>
class BatchAllocator
{
public:
    inline BatchAllocator(unsigned int objectsPerPage = 255)
        : freeHead_(0), objectsPerPage_(objectsPerPage)
    {
        SSVOH_ASSERT(sizeof(AllocatedType) * objectPerAllocation >=
                     sizeof(AllocatedType*));
        SSVOH_ASSERT(objectsPerPage >= 16);
        batches_ = allocateBatch(0);
        currentBatch_ = batches_;
    }
    inline ~BatchAllocator()
    {
        for(BatchInfo* batch = batches_; batch;)
        {
            BatchInfo* nextBatch = batch->next_;
            free(batch);
            batch = nextBatch;
        }
    }
    inline AllocatedType* allocate()
    {
        if(freeHead_)
        {
            AllocatedType* object = freeHead_;
            freeHead_ = *(AllocatedType**)object;
            return object;
        }
        if(currentBatch_->used_ == currentBatch_->end_)
        {
            currentBatch_ = currentBatch_->next_;
            while(currentBatch_ && currentBatch_->used_ == currentBatch_->end_)
                currentBatch_ = currentBatch_->next_;
            if(!currentBatch_)
            {
                currentBatch_ = allocateBatch(objectsPerPage_);
                currentBatch_->next_ = batches_;
                batches_ = currentBatch_;
            }
        }
        AllocatedType* allocated = currentBatch_->used_;
        currentBatch_->used_ += objectPerAllocation;
        return allocated;
    }
    inline void release(AllocatedType* object)
    {
        SSVOH_ASSERT(object != 0);
        *(AllocatedType**)object = freeHead_;
        freeHead_ = object;
    }

private:
    struct BatchInfo
    {
        BatchInfo* next_;
        AllocatedType* used_;
        AllocatedType* end_;
        AllocatedType buffer_[objectPerAllocation];
    };
    inline BatchAllocator(const BatchAllocator&);
    inline void operator=(const BatchAllocator&);
    inline static BatchInfo* allocateBatch(unsigned int objectsPerPage)
    {
        const unsigned int mallocSize =
            sizeof(BatchInfo) - sizeof(AllocatedType) * objectPerAllocation +
            sizeof(AllocatedType) * objectPerAllocation * objectsPerPage;
        BatchInfo* batch = static_cast<BatchInfo*>(malloc(mallocSize));
        batch->next_ = 0;
        batch->used_ = batch->buffer_;
        batch->end_ = batch->buffer_ + objectsPerPage;
        return batch;
    }
    BatchInfo* batches_;
    BatchInfo* currentBatch_;
    AllocatedType* freeHead_;
    unsigned int objectsPerPage_;
};

inline ValueIteratorBase::ValueIteratorBase() : current_(), isNull_(true)
{}
inline ValueIteratorBase::ValueIteratorBase(
    const Value::ObjectValues::iterator& current)
    : current_(current), isNull_(false)
{}

inline Value& ValueIteratorBase::deref() const
{
    return current_->second;
}
inline void ValueIteratorBase::increment()
{
    ++current_;
}
inline void ValueIteratorBase::decrement()
{
    --current_;
}
inline ValueIteratorBase::difference_type ValueIteratorBase::computeDistance(
    const SelfType& other) const
{
    if(isNull_ && other.isNull_) return 0;
    difference_type myDistance = 0;
    for(Value::ObjectValues::iterator it = current_; it != other.current_; ++it)
        ++myDistance;
    return myDistance;
}
inline bool ValueIteratorBase::isEqual(const SelfType& other) const
{
    return isNull_ ? other.isNull_ : current_ == other.current_;
}
inline void ValueIteratorBase::copy(const SelfType& other)
{
    current_ = other.current_;
}
inline Value ValueIteratorBase::key() const
{
    const Value::CZString czstring = (*current_).first;
    if(czstring.c_str())
    {
        if(czstring.isStaticString())
            return Value(StaticString(czstring.c_str()));
        return Value(czstring.c_str());
    }
    return Value(czstring.index());
}
inline UInt ValueIteratorBase::index() const
{
    const Value::CZString czstring = (*current_).first;
    if(!czstring.c_str()) return czstring.index();
    return UInt(-1);
}
inline const char* ValueIteratorBase::memberName() const
{
    const char* name = (*current_).first.c_str();
    return name ? name : "";
}
inline ValueConstIterator::ValueConstIterator()
{}
inline ValueConstIterator::ValueConstIterator(
    const Value::ObjectValues::iterator& current)
    : ValueIteratorBase(current)
{}

inline ValueConstIterator& ValueConstIterator::operator=(
    const ValueIteratorBase& other)
{
    copy(other);
    return *this;
}
inline ValueIterator::ValueIterator()
{}
inline ValueIterator::ValueIterator(
    const Value::ObjectValues::iterator& current)
    : ValueIteratorBase(current)
{}

inline ValueIterator::ValueIterator(const ValueConstIterator& other)
    : ValueIteratorBase(other)
{}
inline ValueIterator::ValueIterator(const ValueIterator& other)
    : ValueIteratorBase(other)
{}
inline ValueIterator& ValueIterator::operator=(const SelfType& other)
{
    copy(other);
    return *this;
}


#ifdef JSON_HAS_INT64

static const double maxUInt64AsDouble = 18446744073709551615.0;
#endif

static const unsigned int unknown = (unsigned)-1;
template <typename T, typename U>
inline bool InRange(double d, T min, U max)
{
    return d >= min && d <= max;
}
inline char* duplicateStringValue(
    const char* value, unsigned int length = unknown)
{
    if(length == unknown) length = (unsigned int)strlen(value);
    if(length >= (unsigned)Value::maxInt) length = Value::maxInt - 1;
    char* newString = static_cast<char*>(malloc(length + 1));
    JSON_ASSERT_MESSAGE(
        newString != nullptr, "Failed to allocate std::string value buffer");
    memcpy(newString, value, length);
    newString[length] = 0;
    return newString;
}
inline void releaseStringValue(char* value)
{
    if(value) free(value);
}

inline Value::CommentInfo::~CommentInfo()
{
    if(comment_) releaseStringValue(comment_);
}
inline void Value::CommentInfo::setComment(const char* text)
{
    if(comment_) releaseStringValue(comment_);
    SSVOH_ASSERT(text != 0);
    JSON_ASSERT_MESSAGE(
        text[0] == '\0' || text[0] == '/', "Comments must start with /");
    comment_ = duplicateStringValue(text);
}
inline Value::CZString::CZString(ArrayIndex mIdx) : cstr_(nullptr), index_(mIdx)
{}
inline Value::CZString::CZString(const char* cstr, DuplicationPolicy allocate)
    : cstr_(allocate == duplicate ? duplicateStringValue(cstr) : cstr),
      index_(allocate)
{}
inline Value::CZString::CZString(const CZString& other)
    : cstr_(other.index_ != noDuplication && other.cstr_ != nullptr
                ? duplicateStringValue(other.cstr_)
                : other.cstr_),
      index_(other.cstr_
                 ? (other.index_ == static_cast<unsigned int>(noDuplication)
                           ? static_cast<unsigned int>(noDuplication)
                           : static_cast<unsigned int>(duplicate))
                 : other.index_)
{}
inline Value::CZString::~CZString()
{
    if(cstr_ != nullptr && index_ == duplicate)
        releaseStringValue(const_cast<char*>(cstr_));
}
inline void Value::CZString::swap(CZString& other)
{
    std::swap(cstr_, other.cstr_);
    std::swap(index_, other.index_);
}
inline Value::CZString& Value::CZString::operator=(const CZString& other)
{
    CZString temp(other);
    swap(temp);
    return *this;
}
inline bool Value::CZString::operator<(const CZString& other) const
{
    if(cstr_ != nullptr && other.cstr_ != nullptr)
        return std::strcmp(cstr_, other.cstr_) < 0;

    return index_ < other.index_;
}
inline bool Value::CZString::operator==(const CZString& other) const
{
    if(cstr_ != nullptr && other.cstr_ != nullptr)
        return std::strcmp(cstr_, other.cstr_) == 0;

    return index_ == other.index_;
}
inline ArrayIndex Value::CZString::index() const
{
    return index_;
}
inline const char* Value::CZString::c_str() const
{
    return cstr_;
}
inline bool Value::CZString::isStaticString() const
{
    return index_ == noDuplication;
}

inline Value::Value(ValueType mType) : type_(mType), allocated_(false)
{
    switch(mType)
    {
        case nullValue: break;
        case intValue:
        case uintValue: value_.int_ = 0; break;
        case realValue: value_.real_ = 0.0; break;
        case stringValue: value_.string_ = nullptr; break;
        case arrayValue:
        case objectValue: value_.map_ = new ObjectValues(); break;
        case booleanValue: value_.bool_ = false; break;
        default: JSON_ASSERT_UNREACHABLE;
    }
}
inline Value::Value(UInt value) : type_(uintValue), allocated_(false)
{
    value_.uint_ = value;
}
inline Value::Value(int value) : type_(intValue), allocated_(false)
{
    value_.int_ = value;
}
#ifdef JSON_HAS_INT64
inline Value::Value(Int64 value) : type_(intValue), allocated_(false)
{
    value_.int_ = value;
}
inline Value::Value(UInt64 value) : type_(uintValue), allocated_(false)
{
    value_.uint_ = value;
}
#endif
inline Value::Value(double value) : type_(realValue), allocated_(false)
{
    value_.real_ = value;
}
inline Value::Value(const char* value) : type_(stringValue), allocated_(true)
{
    value_.string_ = duplicateStringValue(value);
}
inline Value::Value(const char* beginValue, const char* endValue)
    : type_(stringValue), allocated_(true)
{
    value_.string_ =
        duplicateStringValue(beginValue, (unsigned int)(endValue - beginValue));
}
inline Value::Value(const std::string& value)
    : type_(stringValue), allocated_(true)
{
    value_.string_ =
        duplicateStringValue(value.c_str(), (unsigned int)value.size());
}
inline Value::Value(const StaticString& value)
    : type_(stringValue), allocated_(false)
{
    value_.string_ = const_cast<char*>(value.c_str());
}
inline Value::Value(bool value) : type_(booleanValue), allocated_(false)
{
    value_.bool_ = value;
}
inline Value::Value(const Value& other) : type_(other.type_), allocated_(false)
{
    switch(type_)
    {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue: value_ = other.value_; break;
        case stringValue:
            if(other.allocated_)
            {
                if(other.value_.string_)
                {
                    value_.string_ = duplicateStringValue(other.value_.string_);
                    allocated_ = true;
                }
                else
                    value_.string_ = 0;
            }
            else
            {
                value_.string_ = other.value_.string_;
                allocated_ = false;
            }

            break;
        case arrayValue:
        case objectValue:
            value_.map_ = new ObjectValues(*other.value_.map_);
            break;
        default: JSON_ASSERT_UNREACHABLE;
    }
    if(other.comments_)
    {
        comments_ = new CommentInfo[numberOfCommentPlacement];
        for(int comment = 0; comment < numberOfCommentPlacement; ++comment)
        {
            const CommentInfo& otherComment = other.comments_[comment];
            if(otherComment.comment_)
                comments_[comment].setComment(otherComment.comment_);
        }
    }
}
inline Value::~Value()
{
    switch(type_)
    {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue: break;
        case stringValue:
            if(allocated_) releaseStringValue(value_.string_);
            break;
        case arrayValue:
        case objectValue: delete value_.map_; break;
        default: JSON_ASSERT_UNREACHABLE;
    }
    if(comments_) delete[] comments_;
}

inline void Value::swap(Value& other)
{
    ValueType temp = type_;
    type_ = other.type_;
    other.type_ = temp;
    std::swap(value_, other.value_);
    int temp2 = allocated_;
    allocated_ = other.allocated_;
    other.allocated_ = temp2;
}

inline bool Value::operator<(const Value& other) const
{
    int typeDelta = type_ - other.type_;
    if(typeDelta) return typeDelta < 0 ? true : false;
    switch(type_)
    {
        case nullValue: return false;
        case intValue: return value_.int_ < other.value_.int_;
        case uintValue: return value_.uint_ < other.value_.uint_;
        case realValue: return value_.real_ < other.value_.real_;
        case booleanValue: return value_.bool_ < other.value_.bool_;
        case stringValue:
            return (value_.string_ == nullptr && other.value_.string_) ||
                   (other.value_.string_ && value_.string_ &&
                       std::strcmp(value_.string_, other.value_.string_) < 0);
        case arrayValue:
        case objectValue:
        {
            int delta = int(value_.map_->size() - other.value_.map_->size());
            if(delta) return delta < 0;
            return (*value_.map_) < (*other.value_.map_);
        }

        default: JSON_ASSERT_UNREACHABLE;
    }
    return false;
}
inline bool Value::operator==(const Value& other) const
{
    int temp = other.type_;
    if(type_ != temp) return false;
    switch(type_)
    {
        case nullValue: return true;
        case intValue: return value_.int_ == other.value_.int_;
        case uintValue: return value_.uint_ == other.value_.uint_;
        case realValue: return value_.real_ == other.value_.real_;
        case booleanValue: return value_.bool_ == other.value_.bool_;
        case stringValue:
            return (value_.string_ == other.value_.string_) ||
                   (other.value_.string_ && value_.string_ &&
                       std::strcmp(value_.string_, other.value_.string_) == 0);
        case arrayValue:
        case objectValue:
            return value_.map_->size() == other.value_.map_->size() &&
                   (*value_.map_) == (*other.value_.map_);
        default: JSON_ASSERT_UNREACHABLE;
    }
    return false;
}
inline bool Value::operator!=(const Value& other) const
{
    return !(*this == other);
}
inline const char* Value::asCString() const
{
    SSVOH_ASSERT(type_ == stringValue);
    return value_.string_;
}
inline std::string Value::asString() const
{
    switch(type_)
    {
        case nullValue: return "";
        case stringValue: return value_.string_ ? value_.string_ : "";
        case booleanValue: return value_.bool_ ? "true" : "false";
        case intValue: return valueToString(value_.int_);
        case uintValue: return valueToString(value_.uint_);
        case realValue: return valueToString(value_.real_);
        default: JSON_FAIL_MESSAGE("Type is not convertible to std::string");
    }
}

inline int Value::asInt() const
{
    switch(type_)
    {
        case intValue:
            JSON_ASSERT_MESSAGE(isInt(), "LargestInt out of int range");
            return int(value_.int_);
        case uintValue:
            JSON_ASSERT_MESSAGE(isInt(), "LargestUInt out of int range");
            return int(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt, maxInt),
                "double out of int range");
            return int(value_.real_);
        case nullValue: return 0;
        case booleanValue: return value_.bool_ ? 1 : 0;
        default: break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to int.");
}
inline UInt Value::asUInt() const
{
    switch(type_)
    {
        case intValue:
            JSON_ASSERT_MESSAGE(isUInt(), "LargestInt out of UInt range");
            return UInt(value_.int_);
        case uintValue:
            JSON_ASSERT_MESSAGE(isUInt(), "LargestUInt out of UInt range");
            return UInt(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(
                InRange(value_.real_, 0, maxUInt), "double out of UInt range");
            return UInt(value_.real_);
        case nullValue: return 0;
        case booleanValue: return value_.bool_ ? 1 : 0;
        default: JSON_FAIL_MESSAGE("Value is not convertible to UInt.");
    }
}
#ifdef JSON_HAS_INT64
inline Int64 Value::asInt64() const
{
    switch(type_)
    {
        case intValue: return Int64(value_.int_);
        case uintValue:
            JSON_ASSERT_MESSAGE(isInt64(), "LargestUInt out of Int64 range");
            return Int64(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt64, maxInt64),
                "double out of Int64 range");
            return Int64(value_.real_);
        case nullValue: return 0;
        case booleanValue: return value_.bool_ ? 1 : 0;
        default: break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to Int64.");
}
inline UInt64 Value::asUInt64() const
{
    switch(type_)
    {
        case intValue:
            JSON_ASSERT_MESSAGE(isUInt64(), "LargestInt out of UInt64 range");
            return UInt64(value_.int_);
        case uintValue: return UInt64(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt64),
                "double out of UInt64 range");
            return UInt64(value_.real_);
        case nullValue: return 0;
        case booleanValue: return value_.bool_ ? 1 : 0;
        default: break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to UInt64.");
}
#endif
inline LargestInt Value::asLargestInt() const
{
#ifdef JSON_NO_INT64
    return asInt();
#else
    return asInt64();
#endif
}
inline LargestUInt Value::asLargestUInt() const
{
#ifdef JSON_NO_INT64
    return asUInt();
#else
    return asUInt64();
#endif
}
inline double Value::asDouble() const
{
    switch(type_)
    {
        case intValue: return static_cast<double>(value_.int_);
        case uintValue: return static_cast<double>(value_.uint_);
        case realValue: return value_.real_;
        case nullValue: return 0.0;
        case booleanValue: return value_.bool_ ? 1.0 : 0.0;
        default: break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to double.");
}
inline float Value::asFloat() const
{
    switch(type_)
    {
        case intValue: return static_cast<float>(value_.int_);
        case uintValue: return static_cast<float>(value_.uint_);
        case realValue: return static_cast<float>(value_.real_);
        case nullValue: return 0.f;
        case booleanValue: return value_.bool_ ? 1.f : 0.f;
        default: break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to float.");
}
inline bool Value::asBool() const
{
    switch(type_)
    {
        case booleanValue: return value_.bool_;
        case nullValue: return false;
        case intValue: return value_.int_ ? true : false;
        case uintValue: return value_.uint_ ? true : false;
        case realValue: return value_.real_ ? true : false;
        default: break;
    }
    JSON_FAIL_MESSAGE("Value is not convertible to bool.");
}
inline bool Value::isConvertibleTo(ValueType other) const
{
    switch(other)
    {
        case nullValue:
            return (isNumeric() && asDouble() == 0.0) ||
                   (type_ == booleanValue && value_.bool_ == false) ||
                   (type_ == stringValue && asString() == "") ||
                   (type_ == arrayValue && value_.map_->size() == 0) ||
                   (type_ == objectValue && value_.map_->size() == 0) ||
                   type_ == nullValue;
        case intValue:
            return isInt() ||
                   (type_ == realValue &&
                       InRange(value_.real_, minInt, maxInt)) ||
                   type_ == booleanValue || type_ == nullValue;
        case uintValue:
            return isUInt() ||
                   (type_ == realValue && InRange(value_.real_, 0, maxUInt)) ||
                   type_ == booleanValue || type_ == nullValue;
        case realValue:
            return isNumeric() || type_ == booleanValue || type_ == nullValue;
        case booleanValue:
            return isNumeric() || type_ == booleanValue || type_ == nullValue;
        case stringValue:
            return isNumeric() || type_ == booleanValue ||
                   type_ == stringValue || type_ == nullValue;
        case arrayValue: return type_ == arrayValue || type_ == nullValue;
        case objectValue: return type_ == objectValue || type_ == nullValue;
    }
    JSON_ASSERT_UNREACHABLE;
    return false;
}
inline ArrayIndex Value::size() const
{
    switch(type_)
    {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
        case stringValue: return 0;
        case objectValue: return ArrayIndex(value_.map_->size());

        case arrayValue:
            if(!value_.map_->empty())
            {
                auto itLast = value_.map_->end();
                --itLast;
                return (*itLast).first.index() + 1;
            }
            return 0;
    }
    JSON_ASSERT_UNREACHABLE;
    return 0;
}
inline bool Value::empty() const
{
    if(isNull() || isArray() || isObject()) return size() == 0u;
    return false;
}
inline bool Value::operator!() const
{
    return isNull();
}
inline void Value::clear()
{
    SSVOH_ASSERT(
        type_ == nullValue || type_ == arrayValue || type_ == objectValue);
    switch(type_)
    {
        case arrayValue:
        case objectValue: value_.map_->clear(); break;
        default: break;
    }
}
inline void Value::resize(ArrayIndex newSize)
{
    SSVOH_ASSERT(type_ == nullValue || type_ == arrayValue);
    if(type_ == nullValue) *this = Value(arrayValue);
    ArrayIndex oldSize = size();
    if(newSize == 0)
        clear();
    else if(newSize > oldSize)
        (*this)[newSize - 1];
    else
    {
        for(ArrayIndex index = newSize; index < oldSize; ++index)
            value_.map_->erase(index);
        SSVOH_ASSERT(size() == newSize);
    }
}
inline Value& Value::operator[](ArrayIndex index)
{
    SSVOH_ASSERT(type_ == nullValue || type_ == arrayValue);
    if(type_ == nullValue) *this = Value(arrayValue);
    CZString key(index);
    ObjectValues::iterator it = value_.map_->lower_bound(key);
    if(it != value_.map_->end() && (*it).first == key) return (*it).second;
    ObjectValues::value_type defaultValue(key, nullJsonValue);
    it = value_.map_->insert(it, defaultValue);
    return (*it).second;
}
inline Value& Value::operator[](int index)
{
    SSVOH_ASSERT(index >= 0);
    return (*this)[ArrayIndex(index)];
}
inline const Value& Value::operator[](ArrayIndex index) const
{
    SSVOH_ASSERT(type_ == nullValue || type_ == arrayValue);
    if(type_ == nullValue) return nullJsonValue;
    CZString key(index);
    auto it = value_.map_->find(key);
    if(it == value_.map_->end()) return nullJsonValue;
    return (*it).second;
}
inline const Value& Value::operator[](int index) const
{
    SSVOH_ASSERT(index >= 0);
    return (*this)[ArrayIndex(index)];
}
inline Value& Value::operator[](const char* key)
{
    return resolveReference(key, false);
}
inline Value& Value::resolveReference(const char* key, bool isStatic)
{
    SSVOH_ASSERT(type_ == nullValue || type_ == objectValue);
    if(type_ == nullValue) *this = Value(objectValue);
    CZString actualKey(
        key, isStatic ? CZString::noDuplication : CZString::duplicateOnCopy);
    ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
    if(it != value_.map_->end() && (*it).first == actualKey)
        return (*it).second;
    ObjectValues::value_type defaultValue(actualKey, nullJsonValue);
    it = value_.map_->insert(it, defaultValue);
    return (*it).second;
}
inline Value Value::get(ArrayIndex index, const Value& defaultValue) const
{
    const Value* value = &((*this)[index]);
    return value == &nullJsonValue ? defaultValue : *value;
}
inline bool Value::isValidIndex(ArrayIndex index) const
{
    return index < size();
}
inline const Value& Value::operator[](const char* key) const
{
    SSVOH_ASSERT(type_ == nullValue || type_ == objectValue);
    if(type_ == nullValue) return nullJsonValue;
    CZString actualKey(key, CZString::noDuplication);
    auto it = value_.map_->find(actualKey);
    if(it != value_.map_->end()) return it->second;
    return nullJsonValue;
}
inline Value& Value::operator[](const std::string& key)
{
    return (*this)[key.c_str()];
}
inline const Value& Value::operator[](const std::string& key) const
{
    return (*this)[key.c_str()];
}
inline Value& Value::operator[](const StaticString& key)
{
    return resolveReference(key, true);
}

inline Value& Value::append(const Value& value)
{
    return (*this)[size()] = value;
}
inline Value Value::get(const char* key, const Value& defaultValue) const
{
    const Value* value = &((*this)[key]);
    return value == &nullJsonValue ? defaultValue : *value;
}
inline Value Value::get(const std::string& key, const Value& defaultValue) const
{
    return get(key.c_str(), defaultValue);
}
inline Value Value::removeMember(const char* key)
{
    SSVOH_ASSERT(type_ == nullValue || type_ == objectValue);
    if(type_ == nullValue) return nullJsonValue;
    CZString actualKey(key, CZString::noDuplication);
    ObjectValues::iterator it = value_.map_->find(actualKey);
    if(it == value_.map_->end()) return nullJsonValue;
    Value old(it->second);
    value_.map_->erase(it);
    return old;
}
inline Value Value::removeMember(const std::string& key)
{
    return removeMember(key.c_str());
}

inline bool Value::isMember(const char* key) const
{
    CZString actualKey(key, CZString::noDuplication);
    auto it = value_.map_->find(actualKey);
    return it != value_.map_->end();
}
inline bool Value::isMember(const std::string& key) const
{
    return isMember(key.c_str());
}

inline Value::Members Value::getMemberNames() const
{
    SSVOH_ASSERT(type_ == nullValue || type_ == objectValue);
    if(type_ == nullValue) return Value::Members();
    Members members;
    members.reserve(value_.map_->size());
    auto it = value_.map_->begin();
    auto itEnd = value_.map_->end();
    for(; it != itEnd; ++it)
        members.emplace_back(std::string((*it).first.c_str()));

    return members;
}
inline bool IsIntegral(double d)
{
    double integral_part;
    return modf(d, &integral_part) == 0.0;
}
inline bool Value::isNull() const
{
    return type_ == nullValue;
}
inline bool Value::isBool() const
{
    return type_ == booleanValue;
}
inline bool Value::isInt() const
{
    switch(type_)
    {
        case intValue: return value_.int_ >= minInt && value_.int_ <= maxInt;
        case uintValue: return value_.uint_ <= UInt(maxInt);
        case realValue:
            return value_.real_ >= minInt && value_.real_ <= maxInt &&
                   IsIntegral(value_.real_);
        default: break;
    }
    return false;
}
inline bool Value::isUInt() const
{
    switch(type_)
    {
        case intValue:
            return value_.int_ >= 0 &&
                   LargestUInt(value_.int_) <= LargestUInt(maxUInt);
        case uintValue: return value_.uint_ <= maxUInt;
        case realValue:
            return value_.real_ >= 0 && value_.real_ <= maxUInt &&
                   IsIntegral(value_.real_);
        default: break;
    }
    return false;
}
inline bool Value::isInt64() const
{
#ifdef JSON_HAS_INT64
    switch(type_)
    {
        case intValue: return true;
        case uintValue: return value_.uint_ <= UInt64(maxInt64);
        case realValue:
            return value_.real_ >= double(minInt64) &&
                   value_.real_ < double(maxInt64) && IsIntegral(value_.real_);
        default: break;
    }
#endif
    return false;
}
inline bool Value::isUInt64() const
{
#ifdef JSON_HAS_INT64
    switch(type_)
    {
        case intValue: return value_.int_ >= 0;
        case uintValue: return true;
        case realValue:
            return value_.real_ >= 0 && value_.real_ < maxUInt64AsDouble &&
                   IsIntegral(value_.real_);
        default: break;
    }
#endif
    return false;
}
inline bool Value::isIntegral() const
{
#ifdef JSON_HAS_INT64
    return isInt64() || isUInt64();
#else
    return isInt() || isUInt();
#endif
}
inline bool Value::isDouble() const
{
    return type_ == realValue || isIntegral();
}
inline bool Value::isNumeric() const
{
    return isIntegral() || isDouble();
}
inline bool Value::isString() const
{
    return type_ == stringValue;
}
inline bool Value::isArray() const
{
    return type_ == arrayValue;
}
inline bool Value::isObject() const
{
    return type_ == objectValue;
}
inline void Value::setComment(const char* comment, CommentPlacement placement)
{
    if(!comments_) comments_ = new CommentInfo[numberOfCommentPlacement];
    comments_[placement].setComment(comment);
}
inline void Value::setComment(
    const std::string& comment, CommentPlacement placement)
{
    setComment(comment.c_str(), placement);
}
inline bool Value::hasComment(CommentPlacement placement) const
{
    return comments_ != nullptr && comments_[placement].comment_ != nullptr;
}
inline std::string Value::getComment(CommentPlacement placement) const
{
    if(hasComment(placement)) return comments_[placement].comment_;
    return "";
}
inline std::string Value::toStyledString() const
{
    StyledWriter writer;
    return writer.write(*this);
}
inline Value::const_iterator Value::begin() const
{
    switch(type_)
    {
        case arrayValue:
        case objectValue:
            if(value_.map_) return const_iterator(value_.map_->begin());
            break;
        default: break;
    }
    return const_iterator();
}
inline Value::const_iterator Value::end() const
{
    switch(type_)
    {
        case arrayValue:
        case objectValue:
            if(value_.map_) return const_iterator(value_.map_->end());
            break;
        default: break;
    }
    return const_iterator();
}
inline Value::iterator Value::begin()
{
    switch(type_)
    {
        case arrayValue:
        case objectValue:
            if(value_.map_) return iterator(value_.map_->begin());
            break;
        default: break;
    }
    return iterator();
}
inline Value::iterator Value::end()
{
    switch(type_)
    {
        case arrayValue:
        case objectValue:
            if(value_.map_) return iterator(value_.map_->end());
            break;
        default: break;
    }
    return iterator();
}
inline PathArgument::PathArgument() : key_(), index_(), kind_(kindNone)
{}
inline PathArgument::PathArgument(ArrayIndex index)
    : key_(), index_(index), kind_(kindIndex)
{}
inline PathArgument::PathArgument(const char* key)
    : key_(key), index_(), kind_(kindKey)
{}
inline PathArgument::PathArgument(const std::string& key)
    : key_(key.c_str()), index_(), kind_(kindKey)
{}
inline Path::Path(const std::string& path, const PathArgument& a1,
    const PathArgument& a2, const PathArgument& a3, const PathArgument& a4,
    const PathArgument& a5)
{
    InArgs in;
    in.emplace_back(&a1);
    in.emplace_back(&a2);
    in.emplace_back(&a3);
    in.emplace_back(&a4);
    in.emplace_back(&a5);
    makePath(path, in);
}
inline void Path::makePath(const std::string& path, const InArgs& in)
{
    const char* current = path.c_str();
    const char* end = current + path.size();
    InArgs::const_iterator itInArg = in.begin();
    while(current != end)
    {
        if(*current == '[')
        {
            ++current;
            if(*current == '%')
                addPathInArg(path, in, itInArg, PathArgument::kindIndex);
            else
            {
                ArrayIndex index = 0;
                for(; current != end && *current >= '0' && *current <= '9';
                    ++current)
                    index = index * 10 + ArrayIndex(*current - '0');
                args_.emplace_back(index);
            }
            if(current == end || *current++ != ']')
                invalidPath(path, int(current - path.c_str()));
        }
        else if(*current == '%')
        {
            addPathInArg(path, in, itInArg, PathArgument::kindKey);
            ++current;
        }
        else if(*current == '.')
            ++current;
        else
        {
            const char* beginName = current;
            while(current != end && !strchr("[.", *current)) ++current;
            args_.emplace_back(std::string(beginName, current));
        }
    }
}
inline void Path::addPathInArg(const std::string&, const InArgs& in,
    InArgs::const_iterator& itInArg, PathArgument::Kind kind)
{
    if(itInArg == in.end())
    {
    }
    else if((*itInArg)->kind_ != kind)
    {
    }
    else
        args_.emplace_back(**itInArg);
}
inline void Path::invalidPath(const std::string&, int)
{}
inline const Value& Path::resolve(const Value& root) const
{
    const Value* node = &root;
    for(Args::const_iterator it = args_.begin(); it != args_.end(); ++it)
    {
        const PathArgument& arg = *it;
        if(arg.kind_ == PathArgument::kindIndex)
            node = &((*node)[arg.index_]);
        else if(arg.kind_ == PathArgument::kindKey)
            node = &((*node)[arg.key_]);
    }
    return *node;
}
inline Value Path::resolve(const Value& root, const Value& defaultValue) const
{
    const Value* node = &root;
    for(Args::const_iterator it = args_.begin(); it != args_.end(); ++it)
    {
        const PathArgument& arg = *it;
        if(arg.kind_ == PathArgument::kindIndex)
        {
            if(!node->isArray() || !node->isValidIndex(arg.index_))
                return defaultValue;
            node = &((*node)[arg.index_]);
        }
        else if(arg.kind_ == PathArgument::kindKey)
        {
            if(!node->isObject()) return defaultValue;
            node = &((*node)[arg.key_]);
            if(node == &nullJsonValue) return defaultValue;
        }
    }
    return *node;
}
inline Value& Path::make(Value& root) const
{
    Value* node = &root;
    for(Args::const_iterator it = args_.begin(); it != args_.end(); ++it)
    {
        const PathArgument& arg = *it;
        if(arg.kind_ == PathArgument::kindIndex)
            node = &((*node)[arg.index_]);
        else if(arg.kind_ == PathArgument::kindKey)
            node = &((*node)[arg.key_]);
    }
    return *node;
}

inline bool containsControlCharacter(const char* str)
{
    while(*str)
        if(isControlCharacter(*(str++))) return true;
    return false;
}
inline std::string valueToString(LargestInt value)
{
    UIntToStringBuffer buffer;
    char* current = buffer + sizeof(buffer);
    bool isNegative = value < 0;
    if(isNegative) value = -value;
    uintToString(LargestUInt(value), current);
    if(isNegative) *--current = '-';
    SSVOH_ASSERT(current >= buffer);
    return current;
}
inline std::string valueToString(LargestUInt value)
{
    UIntToStringBuffer buffer;
    char* current = buffer + sizeof(buffer);
    uintToString(value, current);
    SSVOH_ASSERT(current >= buffer);
    return current;
}
#ifdef JSON_HAS_INT64
inline std::string valueToString(int value)
{
    return valueToString(LargestInt(value));
}
inline std::string valueToString(UInt value)
{
    return valueToString(LargestUInt(value));
}
#endif
inline std::string valueToString(double value)
{
    char buffer[32];
    sprintf(buffer, "%#.16g", value);
    char* ch = buffer + strlen(buffer) - 1;
    if(*ch != '0') return buffer;
    while(ch > buffer && *ch == '0') --ch;
    char* last_nonzero = ch;
    while(ch >= buffer)
    {
        switch(*ch)
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
            case '9': --ch; continue;
            case '.': *(last_nonzero + 2) = '\0'; return buffer;
            default: return buffer;
        }
    }
    return buffer;
}
inline std::string valueToString(bool value)
{
    return value ? "true" : "false";
}
inline std::string valueToQuotedString(const char* value)
{
    if(value == nullptr) return "";
    if(strpbrk(value, "\"\\\b\f\n\r\t") == nullptr &&
        !containsControlCharacter(value))
        return std::string("\"") + value + "\"";
    std::string::size_type maxsize = strlen(value) * 2 + 3;
    std::string result;
    result.reserve(maxsize);
    result += "\"";
    for(const char* c = value; *c != 0; ++c)
    {
        switch(*c)
        {
            case '\"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if(isControlCharacter(*c))
                {
                    std::ostringstream oss;
                    oss << "\\u" << std::hex << std::uppercase
                        << std::setfill('0') << std::setw(4)
                        << static_cast<int>(*c);
                    result += oss.str();
                }
                else
                    result += *c;
                break;
        }
    }
    result += "\"";
    return result;
}
inline Writer::~Writer()
{}
inline FastWriter::FastWriter()
    : yamlCompatiblityEnabled_(false), dropNullPlaceholders_(false)
{}
inline void FastWriter::enableYAMLCompatibility()
{
    yamlCompatiblityEnabled_ = true;
}
inline void FastWriter::dropNullPlaceholders()
{
    dropNullPlaceholders_ = true;
}
inline std::string FastWriter::write(const Value& root)
{
    document_ = "";
    writeValue(root);
    document_ += "\n";
    return document_;
}
inline void FastWriter::writeValue(const Value& value)
{
    switch(value.type())
    {
        case nullValue:
            if(!dropNullPlaceholders_) document_ += "null";
            break;
        case intValue: document_ += valueToString(value.asLargestInt()); break;
        case uintValue:
            document_ += valueToString(value.asLargestUInt());
            break;
        case realValue: document_ += valueToString(value.asDouble()); break;
        case stringValue:
            document_ += valueToQuotedString(value.asCString());
            break;
        case booleanValue: document_ += valueToString(value.asBool()); break;
        case arrayValue:
        {
            document_ += "[";
            int size = value.size();
            for(int index = 0; index < size; ++index)
            {
                if(index > 0) document_ += ",";
                writeValue(value[index]);
            }
            document_ += "]";
        }
        break;
        case objectValue:
        {
            Value::Members members(value.getMemberNames());
            document_ += "{";
            for(Value::Members::iterator it = members.begin();
                it != members.end(); ++it)
            {
                const std::string& name = *it;
                if(it != members.begin()) document_ += ",";
                document_ += valueToQuotedString(name.c_str());
                document_ += yamlCompatiblityEnabled_ ? ": " : ":";
                writeValue(value[name]);
            }
            document_ += "}";
        }
        break;
    }
}
inline StyledWriter::StyledWriter()
    : rightMargin_(74), indentSize_(3), addChildValues_()
{}
inline std::string StyledWriter::write(const Value& root)
{
    document_ = "";
    addChildValues_ = false;
    indentString_ = "";
    writeCommentBeforeValue(root);
    writeValue(root);
    writeCommentAfterValueOnSameLine(root);
    document_ += "\n";
    return document_;
}
inline void StyledWriter::writeValue(const Value& value)
{
    switch(value.type())
    {
        case nullValue: pushValue("null"); break;
        case intValue: pushValue(valueToString(value.asLargestInt())); break;
        case uintValue: pushValue(valueToString(value.asLargestUInt())); break;
        case realValue: pushValue(valueToString(value.asDouble())); break;
        case stringValue:
            pushValue(valueToQuotedString(value.asCString()));
            break;
        case booleanValue: pushValue(valueToString(value.asBool())); break;
        case arrayValue: writeArrayValue(value); break;
        case objectValue:
        {
            Value::Members members(value.getMemberNames());
            if(members.empty())
                pushValue("{}");
            else
            {
                writeWithIndent("{");
                indent();
                Value::Members::iterator it = members.begin();
                while(true)
                {
                    const std::string& name = *it;
                    const Value& childValue = value[name];
                    writeCommentBeforeValue(childValue);
                    writeWithIndent(valueToQuotedString(name.c_str()));
                    document_ += " : ";
                    writeValue(childValue);
                    if(++it == members.end())
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
inline void StyledWriter::writeArrayValue(const Value& value)
{
    unsigned size = value.size();
    if(size == 0)
        pushValue("[]");
    else
    {
        bool isArrayMultiLine = isMultineArray(value);
        if(isArrayMultiLine)
        {
            writeWithIndent("[");
            indent();
            bool hasChildValue = !childValues_.empty();
            unsigned index = 0;
            while(true)
            {
                const Value& childValue = value[index];
                writeCommentBeforeValue(childValue);
                if(hasChildValue)
                    writeWithIndent(childValues_[index]);
                else
                {
                    writeIndent();
                    writeValue(childValue);
                }
                if(++index == size)
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
            SSVOH_ASSERT(childValues_.size() == size);
            document_ += "[ ";
            for(unsigned index = 0; index < size; ++index)
            {
                if(index > 0) document_ += ", ";
                document_ += childValues_[index];
            }
            document_ += " ]";
        }
    }
}
inline bool StyledWriter::isMultineArray(const Value& value)
{
    int size = value.size();
    bool isMultiLine = size * 3 >= rightMargin_;
    childValues_.clear();
    for(int index = 0; index < size && !isMultiLine; ++index)
    {
        const Value& childValue = value[index];
        isMultiLine =
            isMultiLine || ((childValue.isArray() || childValue.isObject()) &&
                               childValue.size() > 0);
    }
    if(!isMultiLine)
    {
        childValues_.reserve(size);
        addChildValues_ = true;
        int lineLength = 4 + (size - 1) * 2;
        for(int index = 0; index < size && !isMultiLine; ++index)
        {
            writeValue(value[index]);
            lineLength += int(childValues_[index].size());
            isMultiLine = isMultiLine && hasCommentForValue(value[index]);
        }
        addChildValues_ = false;
        isMultiLine = isMultiLine || lineLength >= rightMargin_;
    }
    return isMultiLine;
}
inline void StyledWriter::pushValue(const std::string& value)
{
    if(addChildValues_)
        childValues_.emplace_back(value);
    else
        document_ += value;
}
inline void StyledWriter::writeIndent()
{
    if(!document_.empty())
    {
        char last = document_[document_.size() - 1];
        if(last == ' ') return;
        if(last != '\n') document_ += '\n';
    }
    document_ += indentString_;
}
inline void StyledWriter::writeWithIndent(const std::string& value)
{
    writeIndent();
    document_ += value;
}
inline void StyledWriter::indent()
{
    indentString_ += std::string(indentSize_, ' ');
}
inline void StyledWriter::unindent()
{
    SSVOH_ASSERT(int(indentString_.size()) >= indentSize_);
    indentString_.resize(indentString_.size() - indentSize_);
}
inline void StyledWriter::writeCommentBeforeValue(const Value& root)
{
    if(!root.hasComment(commentBefore)) return;
    document_ += normalizeEOL(root.getComment(commentBefore));
    document_ += "\n";
}
inline void StyledWriter::writeCommentAfterValueOnSameLine(const Value& root)
{
    if(root.hasComment(commentAfterOnSameLine))
        document_ +=
            " " + normalizeEOL(root.getComment(commentAfterOnSameLine));
    if(root.hasComment(commentAfter))
    {
        document_ += "\n";
        document_ += normalizeEOL(root.getComment(commentAfter));
        document_ += "\n";
    }
}
inline bool StyledWriter::hasCommentForValue(const Value& value)
{
    return value.hasComment(commentBefore) ||
           value.hasComment(commentAfterOnSameLine) ||
           value.hasComment(commentAfter);
}
inline std::string StyledWriter::normalizeEOL(const std::string& text)
{
    std::string normalized;
    normalized.reserve(text.size());
    const char* begin = text.c_str();
    const char* end = begin + text.size();
    const char* current = begin;
    while(current != end)
    {
        char c = *current++;
        if(c == '\r')
        {
            if(*current == '\n') ++current;
            normalized += '\n';
        }
        else
            normalized += c;
    }
    return normalized;
}
inline StyledStreamWriter::StyledStreamWriter(std::string indentation)
    : document_(nullptr),
      rightMargin_(74),
      indentation_(indentation),
      addChildValues_()
{}
inline void StyledStreamWriter::write(std::ostream& out, const Value& root)
{
    document_ = &out;
    addChildValues_ = false;
    indentString_ = "";
    writeCommentBeforeValue(root);
    writeValue(root);
    writeCommentAfterValueOnSameLine(root);
    *document_ << '\n';
    document_ = nullptr;
}
inline void StyledStreamWriter::writeValue(const Value& value)
{
    switch(value.type())
    {
        case nullValue: pushValue("null"); break;
        case intValue: pushValue(valueToString(value.asLargestInt())); break;
        case uintValue: pushValue(valueToString(value.asLargestUInt())); break;
        case realValue: pushValue(valueToString(value.asDouble())); break;
        case stringValue:
            pushValue(valueToQuotedString(value.asCString()));
            break;
        case booleanValue: pushValue(valueToString(value.asBool())); break;
        case arrayValue: writeArrayValue(value); break;
        case objectValue:
        {
            Value::Members members(value.getMemberNames());
            if(members.empty())
                pushValue("{}");
            else
            {
                writeWithIndent("{");
                indent();
                Value::Members::iterator it = members.begin();
                while(true)
                {
                    const std::string& name = *it;
                    const Value& childValue = value[name];
                    writeCommentBeforeValue(childValue);
                    writeWithIndent(valueToQuotedString(name.c_str()));
                    *document_ << " : ";
                    writeValue(childValue);
                    if(++it == members.end())
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
inline void StyledStreamWriter::writeArrayValue(const Value& value)
{
    unsigned size = value.size();
    if(size == 0)
        pushValue("[]");
    else
    {
        bool isArrayMultiLine = isMultineArray(value);
        if(isArrayMultiLine)
        {
            writeWithIndent("[");
            indent();
            bool hasChildValue = !childValues_.empty();
            unsigned index = 0;
            while(true)
            {
                const Value& childValue = value[index];
                writeCommentBeforeValue(childValue);
                if(hasChildValue)
                    writeWithIndent(childValues_[index]);
                else
                {
                    writeIndent();
                    writeValue(childValue);
                }
                if(++index == size)
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
            SSVOH_ASSERT(childValues_.size() == size);
            *document_ << "[ ";
            for(unsigned index = 0; index < size; ++index)
            {
                if(index > 0) *document_ << ", ";
                *document_ << childValues_[index];
            }
            *document_ << " ]";
        }
    }
}
inline bool StyledStreamWriter::isMultineArray(const Value& value)
{
    int size = value.size();
    bool isMultiLine = size * 3 >= rightMargin_;
    childValues_.clear();
    for(int index = 0; index < size && !isMultiLine; ++index)
    {
        const Value& childValue = value[index];
        isMultiLine =
            isMultiLine || ((childValue.isArray() || childValue.isObject()) &&
                               childValue.size() > 0);
    }
    if(!isMultiLine)
    {
        childValues_.reserve(size);
        addChildValues_ = true;
        int lineLength = 4 + (size - 1) * 2;
        for(int index = 0; index < size && !isMultiLine; ++index)
        {
            writeValue(value[index]);
            lineLength += int(childValues_[index].size());
            isMultiLine = isMultiLine && hasCommentForValue(value[index]);
        }
        addChildValues_ = false;
        isMultiLine = isMultiLine || lineLength >= rightMargin_;
    }
    return isMultiLine;
}
inline void StyledStreamWriter::pushValue(const std::string& value)
{
    if(addChildValues_)
        childValues_.emplace_back(value);
    else
        *document_ << value;
}
inline void StyledStreamWriter::writeIndent()
{
    *document_ << '\n' << indentString_;
}
inline void StyledStreamWriter::writeWithIndent(const std::string& value)
{
    writeIndent();
    *document_ << value;
}
inline void StyledStreamWriter::indent()
{
    indentString_ += indentation_;
}
inline void StyledStreamWriter::unindent()
{
    SSVOH_ASSERT(indentString_.size() >= indentation_.size());
    indentString_.resize(indentString_.size() - indentation_.size());
}
inline void StyledStreamWriter::writeCommentBeforeValue(const Value& root)
{
    if(!root.hasComment(commentBefore)) return;
    *document_ << normalizeEOL(root.getComment(commentBefore));
    *document_ << '\n';
}
inline void StyledStreamWriter::writeCommentAfterValueOnSameLine(
    const Value& root)
{
    if(root.hasComment(commentAfterOnSameLine))
        *document_ << " " +
                          normalizeEOL(root.getComment(commentAfterOnSameLine));
    if(root.hasComment(commentAfter))
    {
        *document_ << '\n';
        *document_ << normalizeEOL(root.getComment(commentAfter));
        *document_ << '\n';
    }
}
inline bool StyledStreamWriter::hasCommentForValue(const Value& value)
{
    return value.hasComment(commentBefore) ||
           value.hasComment(commentAfterOnSameLine) ||
           value.hasComment(commentAfter);
}
inline std::string StyledStreamWriter::normalizeEOL(const std::string& text)
{
    std::string normalized;
    normalized.reserve(text.size());
    const char* begin = text.c_str();
    const char* end = begin + text.size();
    const char* current = begin;
    while(current != end)
    {
        char c = *current++;
        if(c == '\r')
        {
            if(*current == '\n') ++current;
            normalized += '\n';
        }
        else
            normalized += c;
    }
    return normalized;
}
inline std::ostream& operator<<(std::ostream& sout, const Value& root)
{
    Json::StyledStreamWriter writer;
    writer.write(sout, root);
    return sout;
}
} // namespace Json

#endif
