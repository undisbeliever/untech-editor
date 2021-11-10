/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "xmlwriter.h"
#include "xml-is-name.h"
#include "../base64.h"
#include "../file.h"
#include "../string.h"
#include <algorithm>
#include <cassert>
#include <iomanip>

namespace UnTech::Xml {

static inline std::string_view toStringView(const std::string_view::const_iterator begin, const std::string_view::const_iterator end)
{
    return std::string_view(&*begin, std::distance(begin, end));
}

XmlWriter::XmlWriter(const std::filesystem::path& filePath, const std::string_view doctype, size_t bufferSize)
    : _out(bufferSize)
    , _filePath(filePath)
    , _tagStack()
    , _inTag(false)
    , _useRelativePaths(!filePath.empty())
{
    _out.write("<?xml version=\"1.0\" encoding=\"UTF_8\"?>\n");

    if (!doctype.empty()) {
        assert(isName(doctype));
        _out.write("<!DOCTYPE ", doctype, ">\n");
    }

    _inTag = false;
}

XmlWriter::~XmlWriter()
{
    assert(_tagStack.size() == 0);
}

void XmlWriter::writeTag(const std::string_view name)
{
    assert(isName(name));

    if (_inTag) {
        writeCloseTagHead();
    }

    // Indent
    for (size_t i = 0; i < _tagStack.size(); i++) {
        _out.write("  ");
    }

    _out.write("<", name);

    _tagStack.emplace(name);
    _inTag = true;
}

void XmlWriter::writeTagAttribute_noEscape(const std::string_view name, const std::string_view value)
{
    assert(_inTag);
    assert(isName(name));

    _out.write(" ", name, "=\"", value, "\"");
}

void XmlWriter::writeTagAttribute(const std::string_view name, const std::string_view value)
{
    assert(_inTag);
    assert(isName(name));

    _out.write(" ", name, "=\"");
    escapeAndWrite(value);
    _out.write("\"");
}

void XmlWriter::writeTagAttribute(const std::string_view name, const int value)
{
    assert(_inTag);
    assert(isName(name));

    _out.write(" ", name, "=\"", value, "\"");
}

void XmlWriter::writeTagAttribute(const std::string_view name, const unsigned value)
{
    assert(_inTag);
    assert(isName(name));

    _out.write(" ", name, "=\"", value, "\"");
}

void XmlWriter::writeTagAttributeFilename(const std::string_view name, const std::filesystem::path& path)
{
    std::filesystem::path p = _useRelativePaths ? path.lexically_relative(_filePath.parent_path())
                                                : std::filesystem::absolute(path);

    writeTagAttribute(name, p.generic_string());
}

void XmlWriter::writeTagAttributeHex(const std::string_view name, const unsigned value)
{
    writeTagAttribute_noEscape(name, stringBuilder(hex(value)));
}

void XmlWriter::writeTagAttributeHex6(const std::string_view name, const unsigned value)
{
    writeTagAttribute_noEscape(name, stringBuilder(hex_6(value)));
}

void XmlWriter::writeTagAttributeOptional(const std::string_view name, const std::string_view value)
{
    if (!value.empty()) {
        writeTagAttribute(name, value);
    }
}

void XmlWriter::writeText(const std::string_view text)
{
    if (_inTag) {
        writeCloseTagHead();
    }

    escapeAndWrite(text);
}

void XmlWriter::writeBase64(const uint8_t* data, const size_t size)
{
    if (_inTag) {
        writeCloseTagHead();
    }

    Base64::encode(data, size, _out, _tagStack.size() * 2);
}

void XmlWriter::writeBase64(const std::vector<uint8_t>& data)
{
    writeBase64(data.data(), data.size());
}

void XmlWriter::writeCloseTag()
{
    if (_inTag) {
        _out.write("/>\n");
    }
    else {
        assert(_tagStack.size() > 0);

        for (size_t i = 1; i < _tagStack.size(); i++) {
            _out.write("  ");
        }

        _out.write("</", _tagStack.top(), ">\n");
    }

    _inTag = false;
    _tagStack.pop();
}

inline void XmlWriter::writeCloseTagHead()
{
    assert(_inTag);

    _out.write(">\n");

    _inTag = false;
}

void XmlWriter::escapeAndWrite(const std::string_view text)
{
    using namespace std::string_view_literals;

    constexpr std::string_view toMatch = "&<>\"\'"sv;

    // Using `std::find_first_of` instead of `std::string_view::find_first_of`
    // as the compiler is able to inline it better.
    // (`std::string_view::find_first_of` is calling `memchr`)
    // (Profiling has confirmed std::find_first_of is the faster option)

    auto start = text.begin();
    auto it = std::find_first_of(start, text.end(),
                                 toMatch.begin(), toMatch.end());

    while (it != text.end()) {
        _out.write(toStringView(start, it));

        const char c = *it;
        switch (c) {
        case '&':
            _out.write("&amp;");
            break;

        case '<':
            _out.write("&lt;");
            break;

        case '>':
            _out.write("&gt;");
            break;

        case '"':
            _out.write("&quot;");
            break;

        case '\'':
            _out.write("&apos;");
            break;

        default:
            abort();
        }

        start = it + 1;
        it = std::find_first_of(start, text.end(),
                                toMatch.begin(), toMatch.end());
    }

    _out.write(toStringView(start, text.end()));
}

}
