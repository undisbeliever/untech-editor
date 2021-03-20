/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "xmlreader.h"
#include "../base64.h"
#include "../file.h"
#include "../string.h"
#include "../stringparser.hpp"
#include <cassert>
#include <stdexcept>

namespace UnTech::Xml {

xml_error::xml_error(const XmlTag& tag, const std::string_view message)
    : std::runtime_error(tag.generateErrorString(message))
    , _filePath(tag.xml->filePath())
{
}

xml_error::xml_error(const XmlTag& tag, const std::string_view aName, const std::string_view message)
    : std::runtime_error(tag.generateErrorString(aName, message))
    , _filePath(tag.xml->filePath())
{
}

xml_error::xml_error(const XmlReader& xml, const std::string_view message)
    : std::runtime_error(xml.generateErrorString(message))
    , _filePath(xml.filePath())
{
}

xml_error::xml_error(const XmlReader& xml, const std::string_view message, const std::exception& ex)
    : std::runtime_error(xml.generateErrorString(message, ex))
    , _filePath(xml.filePath())
{
}

static bool string_iterator_equal(std::string_view::const_iterator it, const std::string_view::const_iterator end,
                                  const std::string_view str)
{
    for (auto c : str) {
        if (it == end) {
            return false;
        }
        if (*it != c) {
            return false;
        }
        it++;
    }
    return true;
}

std::string unescapeXmlString(const std::string_view xmlString)
{
    std::string ret;

    if (xmlString.empty()) {
        return ret;
    }

    ret.reserve(xmlString.size());

    auto source = xmlString.begin();
    const auto end = xmlString.end();

    while (source != end) {
        if (*source == '&') {
            auto s = source + 1;

            if (string_iterator_equal(s, end, "lt;")) {
                source += 4;
                ret += '<';
                continue;
            }
            else if (string_iterator_equal(s, end, "gt;")) {
                source += 4;
                ret += '>';
                continue;
            }
            else if (string_iterator_equal(s, end, "amp;")) {
                source += 5;
                ret += '&';
                continue;
            }
            else if (string_iterator_equal(s, end, "apos;")) {
                source += 6;
                ret += '\'';
                continue;
            }
            else if (string_iterator_equal(s, end, "quot;")) {
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

static inline std::string_view toStringView(const std::string::const_iterator begin, const std::string::const_iterator end)
{
    return std::string_view(&*begin, std::distance(begin, end));
}

std::string XmlReader::filename() const
{
    if (_filePath.empty()) {
        return "XML";
    }
    else {
        return _filePath.filename().string();
    }
}

XmlReader::XmlReader(std::string&& xml, const std::filesystem::path& filePath)
    : _filePath(filePath)
    , _input(std::move(xml))
{
    if (_input.atEnd()) {
        throw std::runtime_error("Empty XML file");
    }

    parseDocument();
}

std::unique_ptr<XmlReader> XmlReader::fromFile(const std::filesystem::path& filePath)
{
    std::string xml = File::readUtf8TextFile(filePath);
    return std::make_unique<XmlReader>(std::move(xml), filePath);
}

void XmlReader::parseDocument()
{
    _input.reset();
    _currentTag = std::string();
    _tagStack = std::stack<std::string_view>();
    _inSelfClosingTag = false;

    _input.skipWhitespace();

    // ignore XML header
    if (_input.testAndConsume("<?xml")) {
        if (_input.skipUntil('>') == false) {
            throw xml_error(*this, "Unclosed XML header");
        }
    }

    _input.skipWhitespace();

    // ignore DOCTYPE
    if (_input.testAndConsume("<!DOCTYPE")) {
        if (_input.skipUntil('>') == false) {
            throw xml_error(*this, "Unclosed DOCTYPE header");
        }
    }
}

std::unique_ptr<XmlTag> XmlReader::parseTag()
{
    if (_inSelfClosingTag) {
        return nullptr;
    }

    // skip whitespace/text
    skipText();
    if (_input.atEnd()) {
        throw xml_error(*this, "Unexpected end of file");
    }
    if (_input.cur() == '<' && _input.peek() == '/') {
        // no more tags
        return nullptr;
    }
    if (_input.cur() != '<') {
        throw xml_error(*this, "Not a tag");
    }
    _input.advance();

    const std::string_view tagName = parseName();

    // tag must be followed by whitespace or a close tag.
    if (!(_input.isWhitespace()
          || (_input.cur() == '>')
          || (_input.cur() == '/' || _input.peek() != '>'))) {
        throw xml_error(*this, "Invalid tag name");
    }

    auto tag = std::make_unique<XmlTag>(this, tagName, lineNo());
    _currentTag = tagName;

    while (!_input.atEnd()) {
        _input.skipWhitespace();

        if (_input.atEnd()) {
            throw xml_error(*this, "Unclosed tag");
        }

        const char c = _input.cur();

        if (isName(c)) {
            // attribute

            const std::string_view attributeName = parseName();

            _input.skipWhitespace();

            if (_input.cur() != '=') {
                throw xml_error(*tag, attributeName, "Missing attribute value");
            }
            _input.advance();

            _input.skipWhitespace();

            const std::string_view value = parseAttributeValue();

            tag->attributes.insert({ attributeName, value });
        }

        else if (c == '?' || c == '/') {
            // end of self closing tag
            if (_input.peek() != '>') {
                throw xml_error(*this, "Missing `>`");
            }
            _input.advance();
            _input.advance();

            _inSelfClosingTag = true;
            return tag;
        }

        else if (c == '>') {
            // end of tag
            _input.advance();

            _tagStack.push(tagName);
            return tag;
        }
        else {
            // stringBuilder does not accept char types, have to convert to a c_string manually.
            const char charStr[] = { '`', c, '`', '\0' };
            throw xml_error(*this, stringBuilder("Unknown character ", charStr));
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

    auto startText = _input.pos();
    while (!_input.atEnd()) {
        const auto oldTextPos = _input.pos();

        if (_input.testAndConsume("<!--")) {
            text += unescapeXmlString(toStringView(startText, oldTextPos));

            if (_input.skipUntil("-->") == false) {
                throw xml_error(*this, "Unclosed comment");
            }

            startText = _input.pos();
        }

        else if (_input.testAndConsume("<![CDATA[")) {
            text += unescapeXmlString(toStringView(startText, oldTextPos));

            const auto startCData = _input.pos();

            if (_input.skipUntil("]]>") == false) {
                throw xml_error(*this, "Unclosed CDATA");
            }

            text.append(startCData, _input.pos() - 3);

            startText = _input.pos();
        }
        else if (_input.cur() == '<') {
            // start/end new tag.
            break;
        }
        else {
            _input.advance();
        }
    }

    text += unescapeXmlString(toStringView(startText, _input.pos()));

    return text;
}

std::vector<uint8_t> XmlReader::parseBase64OfUnknownSize()
{
    return Base64::decode(parseText());
}

std::vector<uint8_t> XmlReader::parseBase64OfKnownSize(const size_t expectedSize)
{
    auto data = parseBase64OfUnknownSize();

    if (data.size() != expectedSize) {
        throw xml_error(*this, stringBuilder("Invalid data size. Got ", data.size(), " bytes, expected ", expectedSize, "."));
    }

    return data;
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
    while (_input.cur() != '<' || _input.peek() != '/') {
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
    _input.advance();
    _input.advance();

    auto closeTagName = parseName();
    auto expectedTagName = _tagStack.top();

    if (closeTagName != expectedTagName) {
        std::string msg = stringBuilder("Missing close tag (expected </", expectedTagName, ">)");
        throw xml_error(*this, msg);
    }
    _tagStack.pop();

    _input.skipWhitespace();

    if (_input.cur() != '>') {
        throw xml_error(*this, "Expected '>'");
    }
    // MUST NOT advance here

    if (!_tagStack.empty()) {
        _currentTag = _tagStack.top();
    }
    else {
        _currentTag = std::string();
    }
}

void XmlReader::skipText()
{
    if (_inSelfClosingTag) {
        return;
    }

    while (!_input.atEnd()) {
        if (_input.testAndConsume("<!--")) {
            if (_input.skipUntil("-->") == false) {
                throw xml_error(*this, "Unclosed comment");
            }
        }
        else if (_input.testAndConsume("<![CDATA[")) {
            if (_input.skipUntil("]]>") == false) {
                throw xml_error(*this, "Unclosed CDATA");
            }
        }
        else if (_input.cur() == '<') {
            // start/end new tag.
            break;
        }
        else {
            _input.advance();
        }
    }
}

inline std::string_view XmlReader::parseName()
{
    const auto nameStart = _input.pos();
    while (isName(_input.cur())) {
        _input.advance();
    }

    if (nameStart == _input.pos()) {
        throw xml_error(*this, "Missing identifier");
    }

    return toStringView(nameStart, _input.pos());
}

inline std::string_view XmlReader::parseAttributeValue()
{
    const char terminator = _input.cur();
    _input.advance();

    if (terminator != '\'' && terminator != '\"') {
        throw xml_error(*this, "Attribute not quoted");
    }

    const auto valueStart = _input.pos();
    if (_input.skipUntil(terminator) == false) {
        throw xml_error(*this, "Incomplete attribute value");
    }

    return toStringView(valueStart, _input.pos() - 1);
}

std::string XmlTag::generateErrorString(const std::string_view msg) const
{
    return stringBuilder(xml->filename(), ":", lineNo, " <", name, ">: ", msg);
}

std::string XmlTag::generateErrorString(const std::string_view aName, const std::string_view msg) const
{
    return stringBuilder(xml->filename(), ":", lineNo, " <", name, " ", aName, ">: ", msg);
}

std::string XmlReader::generateErrorString(const std::string_view message) const
{
    if (_currentTag.empty()) {
        return stringBuilder(filename(), ":", lineNo(), ": ", message);
    }
    else {
        return stringBuilder(filename(), ":", lineNo(), " <", _currentTag, ">: ", message);
    }
}

std::string XmlReader::generateErrorString(const std::string_view message, const std::exception& ex) const
{
    auto cast = dynamic_cast<const xml_error*>(&ex);
    if (cast && cast->filePath() == _filePath) {
        return stringBuilder(message, "\n  ", cast->what());
    }
    else {
        if (_currentTag.empty()) {
            return stringBuilder(message, "\n  ", filename(), ":", lineNo(), ": ", message);
        }
        else {
            return stringBuilder(message, "\n  ", filename(), ":", lineNo(), " <", _currentTag, ">: ", message);
        }
    }
}

}
