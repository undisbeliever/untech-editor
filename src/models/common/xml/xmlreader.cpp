/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "xmlreader.h"
#include "xml-is-name.h"
#include "../base64.h"
#include "../file.h"
#include "../stringparser.hpp"
#include "../u8strings.h"
#include <cassert>

namespace UnTech::Xml {

xml_error::xml_error(const XmlTag& tag, const std::u8string_view message)
    : runtime_error(tag.generateErrorString(message))
    , _filePath(tag.xml->filePath())
{
}

xml_error::xml_error(const XmlTag& tag, const std::u8string_view aName, const std::u8string_view message)
    : runtime_error(tag.generateErrorString(aName, message))
    , _filePath(tag.xml->filePath())
{
}

xml_error::xml_error(const XmlReader& xml, const std::u8string_view message)
    : runtime_error(xml.generateErrorString(message))
    , _filePath(xml.filePath())
{
}

xml_error::xml_error(const XmlReader& xml, const std::u8string_view message, const std::exception& ex)
    : runtime_error(xml.generateErrorString(message, ex))
    , _filePath(xml.filePath())
{
}

std::u8string unescapeXmlString(const std::u8string_view xmlString)
{
    using namespace std::string_view_literals;

    std::u8string ret;

    if (xmlString.empty()) {
        return ret;
    }

    ret.reserve(xmlString.size());

    size_t start = 0;
    size_t p = xmlString.find(u8'&');

    while (p != xmlString.npos) {
        ret.append(xmlString.substr(start, p - start));

        auto test = [&](const std::u8string_view es) -> bool {
            if (xmlString.compare(p, es.size(), es) == 0) {
                start = p + es.size();
                return true;
            }
            else {
                return false;
            }
        };

        if (test(u8"&amp;"sv)) {
            ret += u8'&';
        }
        else if (test(u8"&lt;"sv)) {
            ret += u8'<';
        }
        else if (test(u8"&gt;"sv)) {
            ret += u8'>';
        }
        else if (test(u8"&quot;"sv)) {
            ret += u8'"';
        }
        else if (test(u8"&apos;"sv)) {
            ret += u8'\'';
        }
        else {
            start = p + 1;
            ret += u8'&';
        }

        p = xmlString.find(u8'&', start);
    }

    ret.append(xmlString.substr(start));

    return ret;
}

std::u8string XmlReader::filename() const
{
    if (_filePath.empty()) {
        return u8"XML";
    }
    else {
        return _filePath.filename().u8string();
    }
}

XmlReader::XmlReader(std::u8string&& xml, const std::filesystem::path& filePath)
    : _filePath(filePath)
    , _input(std::move(xml))
{
    if (_input.atEnd()) {
        throw runtime_error(u8"Empty XML file");
    }

    parseDocument();
}

std::unique_ptr<XmlReader> XmlReader::fromFile(const std::filesystem::path& filePath)
{
    std::u8string xml = File::readUtf8TextFile(filePath);
    return std::make_unique<XmlReader>(std::move(xml), filePath);
}

void XmlReader::parseDocument()
{
    _input.reset();
    _currentTag = std::u8string();
    _tagStack = std::stack<std::u8string_view>();
    _inSelfClosingTag = false;

    _input.skipWhitespace();

    // ignore XML header
    if (_input.testAndConsume(u8"<?xml")) {
        if (_input.skipUntil(u8'>') == false) {
            throw xml_error(*this, u8"Unclosed XML header");
        }
    }

    _input.skipWhitespace();

    // ignore DOCTYPE
    if (_input.testAndConsume(u8"<!DOCTYPE")) {
        if (_input.skipUntil(u8'>') == false) {
            throw xml_error(*this, u8"Unclosed DOCTYPE header");
        }
    }
}

// Splitting up tag parsing allows for NRVO in parseTag().
// Returns an empty string_view if there is no tag to parse.
inline std::u8string_view XmlReader::parseTagStart()
{
    std::u8string_view tagName;

    if (_inSelfClosingTag) {
        return tagName;
    }

    // skip whitespace/text
    skipText();
    if (_input.atEnd()) {
        throw xml_error(*this, u8"Unexpected end of file");
    }
    if (_input.cur() == u8'<' && _input.peek() == u8'/') {
        // no more tags
        return tagName;
    }
    if (_input.cur() != u8'<') {
        throw xml_error(*this, u8"Not a tag");
    }
    _input.advance();

    tagName = parseName();

    // tag must be followed by whitespace or a close tag.
    if (!(_input.isWhitespace()
          || (_input.cur() == u8'>')
          || (_input.cur() == u8'/' || _input.peek() != u8'>'))) {
        throw xml_error(*this, u8"Invalid tag name");
    }

    return tagName;
}

XmlTag XmlReader::parseTag()
{
    XmlTag tag(this, parseTagStart(), lineNo());

    if (tag.name.empty()) {
        return tag;
    }

    while (!_input.atEnd()) {
        _input.skipWhitespace();

        if (_input.atEnd()) {
            throw xml_error(*this, u8"Unclosed tag");
        }

        const char8_t c = _input.cur();

        if (isName(c)) {
            // attribute

            const std::u8string_view attributeName = parseName();

            _input.skipWhitespace();

            if (_input.cur() != u8'=') {
                throw xml_error(tag, attributeName, u8"Missing attribute value");
            }
            _input.advance();

            _input.skipWhitespace();

            const std::u8string_view value = parseAttributeValue();

            tag.addAttribute(attributeName, value);
        }

        else if (c == u8'?' || c == u8'/') {
            // end of self closing tag
            if (_input.peek() != u8'>') {
                throw xml_error(*this, u8"Missing `>`");
            }
            _input.advance();
            _input.advance();

            _inSelfClosingTag = true;
            return tag;
        }

        else if (c == u8'>') {
            // end of tag
            _input.advance();

            _tagStack.push(tag.name);
            return tag;
        }
        else {
            if (c >= 0x20 && c < 0x80) {
                // c is printable.
                // `stringBuilder()` does not accept char types.  Have to manually covert `c` to a u8string_view.
                throw xml_error(*this, stringBuilder(u8"Unknown character `", std::u8string_view(&c, 1), u8"`"));
            }
            else {
                // c is not printable.
                throw xml_error(*this, stringBuilder(u8"Unknown character 0x", hex(uint32_t(c))));
            }
        }
    }

    throw xml_error(*this, u8"Incomplete tag");
}

std::u8string XmlReader::parseText()
{
    if (_inSelfClosingTag) {
        return std::u8string();
    }

    std::u8string text;

    auto startText = _input.pos();
    while (!_input.atEnd()) {
        const auto oldTextPos = _input.pos();

        if (_input.testAndConsume(u8"<!--")) {
            text += unescapeXmlString(std::u8string_view(startText, oldTextPos));

            if (_input.skipUntil(u8"-->") == false) {
                throw xml_error(*this, u8"Unclosed comment");
            }

            startText = _input.pos();
        }

        else if (_input.testAndConsume(u8"<![CDATA[")) {
            text += unescapeXmlString(std::u8string_view(startText, oldTextPos));

            const auto startCData = _input.pos();

            if (_input.skipUntil(u8"]]>") == false) {
                throw xml_error(*this, u8"Unclosed CDATA");
            }

            text.append(startCData, _input.pos() - 3);

            startText = _input.pos();
        }
        else if (_input.cur() == u8'<') {
            // start/end new tag.
            break;
        }
        else {
            _input.advance();
        }
    }

    text += unescapeXmlString(std::u8string_view(startText, _input.pos()));

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
        throw xml_error(*this, stringBuilder(u8"Invalid data size. Got ", data.size(), u8" bytes, expected ", expectedSize, u8"."));
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
    while (_input.cur() != u8'<' || _input.peek() != u8'/') {
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
        std::u8string msg = stringBuilder(u8"Missing close tag (expected </", expectedTagName, u8">)");
        throw xml_error(*this, msg);
    }
    _tagStack.pop();

    _input.skipWhitespace();

    if (_input.cur() != u8'>') {
        throw xml_error(*this, u8"Expected '>'");
    }
    // MUST NOT advance here

    if (!_tagStack.empty()) {
        _currentTag = _tagStack.top();
    }
    else {
        _currentTag = std::u8string();
    }
}

void XmlReader::skipText()
{
    if (_inSelfClosingTag) {
        return;
    }

    while (!_input.atEnd()) {
        if (_input.testAndConsume(u8"<!--")) {
            if (_input.skipUntil(u8"-->") == false) {
                throw xml_error(*this, u8"Unclosed comment");
            }
        }
        else if (_input.testAndConsume(u8"<![CDATA[")) {
            if (_input.skipUntil(u8"]]>") == false) {
                throw xml_error(*this, u8"Unclosed CDATA");
            }
        }
        else if (_input.cur() == u8'<') {
            // start/end new tag.
            break;
        }
        else {
            _input.advance();
        }
    }
}

inline std::u8string_view XmlReader::parseName()
{
    const auto nameStart = _input.pos();
    while (isName(_input.cur())) {
        _input.advance();
    }

    if (nameStart == _input.pos()) {
        throw xml_error(*this, u8"Missing identifier");
    }

    return { nameStart, _input.pos() };
}

inline std::u8string_view XmlReader::parseAttributeValue()
{
    const char terminator = _input.cur();
    _input.advance();

    if (terminator != u8'\'' && terminator != u8'\"') {
        throw xml_error(*this, u8"Attribute not quoted");
    }

    const auto valueStart = _input.pos();
    if (_input.skipUntil(terminator) == false) {
        throw xml_error(*this, u8"Incomplete attribute value");
    }

    return { valueStart, _input.pos() - 1 };
}

std::u8string XmlTag::generateErrorString(const std::u8string_view msg) const
{
    return stringBuilder(xml->filename(), u8":", lineNo, u8" <", name, u8">: ", msg);
}

std::u8string XmlTag::generateErrorString(const std::u8string_view aName, const std::u8string_view msg) const
{
    return stringBuilder(xml->filename(), u8":", lineNo, u8" <", name, u8" ", aName, u8">: ", msg);
}

std::u8string XmlReader::generateErrorString(const std::u8string_view message) const
{
    if (_currentTag.empty()) {
        return stringBuilder(filename(), u8":", lineNo(), u8": ", message);
    }
    else {
        return stringBuilder(filename(), u8":", lineNo(), u8" <", _currentTag, u8">: ", message);
    }
}

std::u8string XmlReader::generateErrorString(const std::u8string_view message, const std::exception& ex) const
{
    auto cast = dynamic_cast<const xml_error*>(&ex);
    if (cast && cast->filePath() == _filePath) {
        return stringBuilder(message, u8"\n  ", convert_old_string(cast->what()));
    }
    else {
        if (_currentTag.empty()) {
            return stringBuilder(message, u8"\n  ", filename(), u8":", lineNo(), u8": ", message);
        }
        else {
            return stringBuilder(message, u8"\n  ", filename(), u8":", lineNo(), u8" <", _currentTag, u8">: ", message);
        }
    }
}

}
