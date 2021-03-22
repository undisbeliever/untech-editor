/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../aabb.h"
#include "../base64.h"
#include "../string.h"
#include "../stringparser.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

namespace UnTech::Xml {

class XmlReader;
struct XmlTag;

// NOTE: Will only unescape sequences created by `XmlWriter`
std::string unescapeXmlString(const std::string_view xmlString);

class xml_error : public std::runtime_error {
public:
    explicit xml_error(const XmlTag& tag, const std::string_view message);
    explicit xml_error(const XmlTag& tag, const std::string_view attr, const std::string_view message);
    explicit xml_error(const XmlReader& xml, const std::string_view message);
    explicit xml_error(const XmlReader& tag, const std::string_view message, const std::exception& error);

    inline const std::filesystem::path& filePath() const { return _filePath; }

private:
    const std::filesystem::path _filePath;
};

/**
 * The `XmlReader` class allows for the parsing of a subset of the XML spec.
 *
 * It only supports:
 *      * UTF8 text encoding
 *      * tags/attributes (without namespaces)
 *      * comments
 *      * cdata
 *      * &lt;, &gt;, &amp;, &apos; and &quot; escape sequences
 *
 * It is inspired by Qt's QXmlStreamReader, but is simpler and will cause an
 * exception if it tries to process malformed XML.
 *
 * The parser functions can raise a std::runtime_error with a human readable
 * description of the parsing error and its location.
 *
 * CHANGELOG:
 *   * Now uses std::string_view
 *   * XmlReader does not lowercases names.  Names are now case-sensitive.

 */
class XmlReader {

private:
    const std::filesystem::path _filePath;
    StringParser _input;

    std::stack<std::string_view> _tagStack;
    std::string_view _currentTag;
    bool _inSelfClosingTag;

public:
    XmlReader() = delete;
    XmlReader(const XmlReader&) = delete;
    XmlReader(XmlReader&&) = delete;
    XmlReader& operator=(const XmlReader&) = delete;
    XmlReader& operator=(XmlReader&&) = delete;

    XmlReader(std::string&& xml, const std::filesystem::path& filePath = std::filesystem::path());

    static std::unique_ptr<XmlReader> fromFile(const std::filesystem::path& filePath);

    /** The filesystem path of the XML file, may be empty */
    inline const std::filesystem::path& filePath() const { return _filePath; }

    std::string filename() const;

    /** restart processing from the beginning */
    void parseDocument();

    /**
     * This method skips text/whitespace before the next tag.
     * returns a XmlTag with an empty `name` if there are no tags in the current level.
     *
     * MEMORY SAFETY: The XmlTag relies on the XmlReader.
     * It must not exist when this XmlReader is reclaimed.
     */
    XmlTag parseTag();

    /** returns the text at the current cursor */
    std::string parseText();

    /** returns the base64 data at the current cursor */
    std::vector<uint8_t> parseBase64OfUnknownSize();

    /** returns the base64 data at the current cursor, throws xml_error if size of data != size */
    std::vector<uint8_t> parseBase64OfKnownSize(const size_t expectedSize);

    /** reads the base64 data into a byte array, throws xml_error if size of data != size of array. */
    template <size_t N>
    void parseBase64ToByteArray(std::array<uint8_t, N>& data)
    {
        const size_t bytesDecoded = Base64::decodeToBuffer(data.data(), data.size(), parseText());

        if (bytesDecoded != data.size()) {
            throw xml_error(*this, stringBuilder("Invalid data size. Got ", bytesDecoded, " bytes, expected ", data.size(), "."));
        }
    }

    /** This method will skip over any child/sibling text/tags in order to close the current tag */
    void parseCloseTag();

    /** the current line number of the cursor */
    inline unsigned lineNo() const { return _input.lineNo(); }

    std::string generateErrorString(const std::string_view message) const;
    std::string generateErrorString(const std::string_view message, const std::exception& ex) const;

private:
    void skipText();
    std::string_view parseName();
    std::string_view parseAttributeValue();
    std::string_view parseTagStart();
};
}

#include "xmltag.h"
