// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#ifndef SSVUJ_OH_JSONCPP_JSONSTREAM
#define SSVUJ_OH_JSONCPP_JSONSTREAM

// Pretty-printer for `Json::Value`. Writes into a `sf::base::String` so
// the vendored jsoncpp stays free of `<iosfwd>` / `std::ostream`.

#pragma GCC system_header

#include "SSVOpenHexagon/SSVUtilsJson/JsonCpp/json.hpp"

#include "SFML/Base/String.hpp"

namespace Json
{

class StyledStreamWriter
{
public:
    StyledStreamWriter(sf::base::String indentation = "\t");
    ~StyledStreamWriter()
    {
    }
    void write(sf::base::String& out, const Value& root);

private:
    void                    writeValue(const Value& value);
    void                    writeArrayValue(const Value& value);
    bool                    isMultineArray(const Value& value);
    void                    pushValue(const sf::base::String& value);
    void                    writeIndent();
    void                    writeWithIndent(const sf::base::String& value);
    void                    indent();
    void                    unindent();
    void                    writeCommentBeforeValue(const Value& root);
    void                    writeCommentAfterValueOnSameLine(const Value& root);
    bool                    hasCommentForValue(const Value& value);
    static sf::base::String normalizeEOL(const sf::base::String& text);
    using ChildValues = sf::base::Vector<sf::base::String>;
    ChildValues       childValues_;
    sf::base::String* document_;
    sf::base::String  indentString_;
    int               rightMargin_;
    sf::base::String  indentation_;
    bool              addChildValues_;
};

} // namespace Json

#endif
