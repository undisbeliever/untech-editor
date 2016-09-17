#include "xmlreader.h"
#include "../base64.h"
#include "../file.h"
#include "../string.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Xml;

xml_error::xml_error(const XmlTag& tag, const char* message)
    : std::runtime_error(tag.generateErrorString(message))
    , _filename(tag.xml->filename())
{
}

xml_error::xml_error(const XmlTag& tag, const std::string& aName, const char* message)
    : std::runtime_error(tag.generateErrorString(aName, message))
    , _filename(tag.xml->filename())
{
}

xml_error::xml_error(const XmlReader& xml, const char* message)
    : std::runtime_error(xml.generateErrorString(message))
    , _filename(xml.filename())
{
}

xml_error::xml_error(const XmlReader& xml, const char* message, const std::exception& ex)
    : std::runtime_error(xml.generateErrorString(message, ex))
    , _filename(xml.filename())
{
}

namespace UnTech {
namespace XmlPrivate {

inline bool isWhitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

inline std::string unescapeXmlString(const char* start, const char* end)
{
    std::string ret;
    ret.reserve(end - start);

    const char* source = start;

    while (source < end) {
        if (*source == '&') {
            const char* s = source + 1;

            if (memcmp(s, "lt;", 3) == 0) {
                source += 4;
                ret += '<';
                continue;
            }
            else if (memcmp(s, "gt;", 3) == 0) {
                source += 4;
                ret += '>';
                continue;
            }
            else if (memcmp(s, "amp;", 4) == 0) {
                source += 5;
                ret += '&';
                continue;
            }
            else if (memcmp(s, "apos;", 5) == 0) {
                source += 6;
                ret += '\'';
                continue;
            }
            else if (memcmp(s, "quot;", 5) == 0) {
                source += 6;
                ret += '\"';
                continue;
            }
        }
        ret += *source;
        source++;
    }

    return ret;
}

std::string xmlFilepart(const XmlReader* xml)
{
    auto fp = xml->filepart();
    if (fp.empty()) {
        return "XML";
    }
    return fp;
}
}
}

using namespace UnTech::XmlPrivate;

XmlReader::XmlReader(const std::string& xml, const std::string& filename)
    : _inputString(xml)
    , _filename(filename)
{
    if (xml.empty()) {
        throw std::runtime_error("Empty XML file");
    }

    if (!filename.empty()) {
        std::tie(_dirname, _filepart) = File::splitFilename(filename);
    }
    else {
        _dirname = std::string();
        _filepart = "XML";
    }

    parseDocument();
}

std::unique_ptr<XmlReader> XmlReader::fromFile(const std::string& filename)
{
    std::string xml = File::readUtf8TextFile(filename);
    return std::make_unique<XmlReader>(xml, filename);
}

void XmlReader::parseDocument()
{
    _pos = _inputString.c_str();
    _currentTag = std::string();
    _tagStack = std::stack<std::string>();
    _inSelfClosingTag = false;
    _lineNo = 1;

    skipWhitespace();

    // ignore XML header
    if (memcmp(_pos, "<?xml", 5) == 0) {
        _pos += 5;

        while (*_pos != '>') {
            if (*_pos == '\0') {
                throw xml_error(*this, "Unclosed XML header");
            }
            if (*_pos == '\n') {
                _lineNo++;
            }
            _pos++;
        }
        _pos++;
    }

    skipWhitespace();

    // ignore DOCTYPE
    if (memcmp(_pos, "<!DOCTYPE", 9) == 0) {
        _pos += 9;

        while (*_pos != '>') {
            if (*_pos == '\0') {
                throw xml_error(*this, "Unclosed DOCTYPE header");
            }
            if (*_pos == '\n') {
                _lineNo++;
            }
            _pos++;
        }
        _pos++;
    }
}

std::unique_ptr<XmlTag> XmlReader::parseTag()
{
    if (_inSelfClosingTag) {
        return nullptr;
    }

    // skip whitespace/text
    skipText();
    if (_pos[0] == '<' && _pos[1] == '/') {
        // no more tags
        return nullptr;
    }

    if (*_pos == '\0') {
        throw xml_error(*this, "Unexpected end of file");
    }
    if (*_pos != '<') {
        throw xml_error(*this, "Not a tag");
    }
    _pos++;

    std::string tagName = parseName();

    // tag must be followed by whitespace or a close tag.
    if (!(isWhitespace(*_pos)
          || (_pos[0] == '>')
          || (_pos[0] == '/' || _pos[1] != '>'))) {
        throw xml_error(*this, "Invalid tag name");
    }

    auto tag = std::make_unique<XmlTag>(this, tagName, _lineNo);
    _currentTag = tagName;

    while (*_pos) {
        skipWhitespace();

        if (*_pos == '\0') {
            throw xml_error(*this, "Unclosed tag");
        }

        if (isName(*_pos)) {
            // attribute

            std::string attributeName = parseName();

            skipWhitespace();

            if (*_pos != '=') {
                throw xml_error(*tag, attributeName, "Missing attribute value");
            }
            _pos++;

            skipWhitespace();

            std::string value = parseAttributeValue();

            tag->attributes.insert({ attributeName, value });
        }

        else if (*_pos == '?' || *_pos == '/') {
            // end of self closing tag
            if (_pos[1] != '>') {
                throw xml_error(*this, "Missing `>`");
            }
            _pos += 2;

            _inSelfClosingTag = true;
            return tag;
        }

        else if (*_pos == '>') {
            // end of tag
            _pos++;

            _tagStack.push(tagName);
            return tag;
        }
        else {
            std::string msg = std::string("Unknown character `") + *_pos + '`';
            throw xml_error(*this, msg.c_str());
        }
    }

    throw xml_error(*this, "Incomplete tag");
}

std::string XmlReader::parseText()
{
    if (_inSelfClosingTag) {
        return std::string();
    }

    std::string text;

    const char* startText = _pos;
    while (*_pos) {
        if (memcmp(_pos, "<!--", 4) == 0) {
            text += unescapeXmlString(startText, _pos);

            // skip comment
            while (memcmp(_pos, "-->", 3) != 0) {
                if (*_pos == '\0') {
                    throw xml_error(*this, "Unclosed comment");
                }
                if (*_pos == '\n') {
                    _lineNo++;
                }
                _pos++;
            }
            _pos += 3;
            startText = _pos;
        }

        else if (memcmp(_pos, "<![CDATA[", 9) == 0) {
            text += unescapeXmlString(startText, _pos);

            _pos += 9;
            const char* startCData = _pos;

            while (memcmp(_pos, "]]>", 3) != 0) {
                if (*_pos == '\0') {
                    throw xml_error(*this, "Unclosed CDATA");
                }
                if (*_pos == '\n') {
                    _lineNo++;
                }
                _pos++;
            }
            text.append(startCData, _pos - startCData);

            _pos += 3;
            startText = _pos;
        }
        else if (*_pos == '<') {
            // start/end new tag.
            break;
        }
        if (*_pos == '\n') {
            _lineNo++;
            _pos++;
        }
        else {
            _pos++;
        }
    }

    text += unescapeXmlString(startText, _pos);

    return text;
}

std::vector<uint8_t> XmlReader::parseBase64()
{
    return Base64::decode(parseText());
}

void XmlReader::parseCloseTag()
{
    // generate correct error tag
    if (!_tagStack.empty()) {
        _currentTag = _tagStack.top();
    }

    if (_inSelfClosingTag) {
        _inSelfClosingTag = false;
        return;
    }

    // skip all child nodes of current level
    while (_pos[0] != '<' || _pos[1] != '/') {
        // more nodes/text to parse
        parseTag();

        if (_inSelfClosingTag) {
            // save a function call.
            _inSelfClosingTag = false;
        }
        else {
            parseCloseTag();
        }
    }

    _pos += 2;
    auto closeTagName = parseName();
    auto expectedTagName = _tagStack.top();

    if (closeTagName != expectedTagName) {
        std::string msg = "Missing close tag (expected </" + expectedTagName + ">)";
        throw xml_error(*this, msg.c_str());
    }
    _tagStack.pop();

    skipWhitespace();

    if (*_pos != '>') {
        throw xml_error(*this, "Expected '>'");
    }

    if (!_tagStack.empty()) {
        _currentTag = _tagStack.top();
    }
    else {
        _currentTag = std::string();
    }
}

inline void XmlReader::skipWhitespace()
{
    while (isWhitespace(*_pos)) {
        if (*_pos == '\n') {
            _lineNo++;
        }
        _pos++;
    }
}

void XmlReader::skipText()
{
    if (_inSelfClosingTag) {
        return;
    }

    while (*_pos) {
        if (memcmp(_pos, "<!--", 4) == 0) {
            // skip comment
            while (memcmp(_pos, "-->", 3) != 0) {
                if (*_pos == '\0') {
                    throw xml_error(*this, "Unclosed comment");
                }
                if (*_pos == '\n') {
                    _lineNo++;
                }
                _pos++;
            }
            _pos += 3;
        }

        else if (memcmp(_pos, "<![CDATA[", 9) == 0) {
            _pos += 9;
            while (memcmp(_pos, "]]>", 3) != 0) {
                if (*_pos == '\0') {
                    throw xml_error(*this, "Unclosed CDATA");
                }
                if (*_pos == '\n') {
                    _lineNo++;
                }
                _pos++;
            }
            _pos += 3;
        }
        else if (*_pos == '<') {
            // start/end new tag.
            break;
        }
        if (*_pos == '\n') {
            _lineNo++;
            _pos++;
        }
        else {
            _pos++;
        }
    }
}

inline std::string XmlReader::parseName()
{
    const char* nameStart = _pos;
    while (isName(*_pos)) {
        _pos++;
    }

    if (nameStart == _pos) {
        throw xml_error(*this, "Missing identifier");
    }

    std::string ret(nameStart, _pos - nameStart);
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);

    return ret;
}

inline std::string XmlReader::parseAttributeValue()
{
    if (*_pos != '\'' && *_pos != '\"') {
        throw xml_error(*this, "Attribute not quoted");
    }

    const char terminator = *_pos;
    _pos++;

    const char* valueStart = _pos;
    while (*_pos != terminator) {
        if (*_pos == '\0') {
            throw xml_error(*this, "Incomplete attribute value");
        }
        if (*_pos == '\n') {
            _lineNo++;
        }
        _pos++;
    }

    std::string value = unescapeXmlString(valueStart, _pos);
    _pos++;

    return value;
}

std::string XmlTag::generateErrorString(const char* msg) const
{
    std::stringstream stream;

    stream << xml->filepart() << ":" << lineNo << " <" << name << ">: " << msg;
    return stream.str();
}

std::string XmlTag::generateErrorString(const std::string& aName, const char* msg) const
{
    std::stringstream stream;

    stream << xml->filepart() << ":" << lineNo << " <" << name << " " << aName << ">: " << msg;
    return stream.str();
}

std::string XmlReader::generateErrorString(const char* message) const
{
    std::stringstream stream;

    if (_currentTag.empty()) {
        stream << _filepart << ':' << _lineNo << ": " << message;
    }
    else {
        stream << _filepart << ':' << _lineNo << " <" << _currentTag << ">: " << message;
    }
    return stream.str();
}

std::string XmlReader::generateErrorString(const char* message, const std::exception& ex) const
{
    std::stringstream stream;
    stream << message << "\n  ";

    auto cast = dynamic_cast<const xml_error*>(&ex);
    if (cast && cast->filename() == _filename) {
        stream << cast->what();
    }
    else {
        stream << _filepart << ':' << _lineNo;

        if (_currentTag.empty()) {
            stream << ": " << ex.what();
        }
        else {
            stream << " <" << _currentTag << ">: " << ex.what();
        }
    }
    return stream.str();
}
