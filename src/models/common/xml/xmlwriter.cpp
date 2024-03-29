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

XmlWriter::XmlWriter(const std::filesystem::path& filePath, const std::u8string_view doctype, size_t bufferSize)
    : _out(bufferSize)
    , _filePath(filePath)
    , _tagStack()
    , _inTag(false)
    , _useRelativePaths(!filePath.empty())
{
    _out.write(u8"<?xml version=\"1.0\" encoding=\"UTF_8\"?>\n");

    if (!doctype.empty()) {
        assert(isName(doctype));
        _out.write(u8"<!DOCTYPE ", doctype, u8">\n");
    }

    _inTag = false;
}

XmlWriter::~XmlWriter()
{
    assert(_tagStack.size() == 0);
}

void XmlWriter::writeTag(const std::u8string_view name)
{
    assert(isName(name));

    if (_inTag) {
        writeCloseTagHead();
    }

    // Indent
    for (size_t i = 0; i < _tagStack.size(); i++) {
        _out.write(u8"  ");
    }

    _out.write(u8"<", name);

    _tagStack.emplace(name);
    _inTag = true;
}

void XmlWriter::writeTagAttribute_noEscape(const std::u8string_view name, const std::u8string_view value)
{
    assert(_inTag);
    assert(isName(name));

    _out.write(u8" ", name, u8"=\"", value, u8"\"");
}

void XmlWriter::writeTagAttribute(const std::u8string_view name, const std::u8string_view value)
{
    assert(_inTag);
    assert(isName(name));

    _out.write(u8" ", name, u8"=\"");
    escapeAndWrite(value);
    _out.write(u8"\"");
}

void XmlWriter::writeTagAttribute(const std::u8string_view name, const int value)
{
    assert(_inTag);
    assert(isName(name));

    _out.write(u8" ", name, u8"=\"", value, u8"\"");
}

void XmlWriter::writeTagAttribute(const std::u8string_view name, const unsigned value)
{
    assert(_inTag);
    assert(isName(name));

    _out.write(u8" ", name, u8"=\"", value, u8"\"");
}

void XmlWriter::writeTagAttributeFilename(const std::u8string_view name, const std::filesystem::path& path)
{
    std::filesystem::path p = _useRelativePaths ? path.lexically_relative(_filePath.parent_path())
                                                : std::filesystem::absolute(path);

    writeTagAttribute(name, p.generic_u8string());
}

void XmlWriter::writeTagAttributeHex(const std::u8string_view name, const unsigned value)
{
    writeTagAttribute_noEscape(name, stringBuilder(hex(value)));
}

void XmlWriter::writeTagAttributeHex6(const std::u8string_view name, const unsigned value)
{
    writeTagAttribute_noEscape(name, stringBuilder(hex_6(value)));
}

void XmlWriter::writeTagAttributeOptional(const std::u8string_view name, const std::u8string_view value)
{
    if (!value.empty()) {
        writeTagAttribute(name, value);
    }
}

void XmlWriter::writeText(const std::u8string_view text)
{
    if (_inTag) {
        writeCloseTagHead();
    }

    escapeAndWrite(text);
}

void XmlWriter::writeBase64(std::span<const uint8_t> data)
{
    if (_inTag) {
        writeCloseTagHead();
    }

    Base64::encode(data, _out, _tagStack.size() * 2);
}

void XmlWriter::writeCloseTag()
{
    if (_inTag) {
        _out.write(u8"/>\n");
    }
    else {
        assert(_tagStack.size() > 0);

        for (size_t i = 1; i < _tagStack.size(); i++) {
            _out.write(u8"  ");
        }

        _out.write(u8"</", _tagStack.top(), u8">\n");
    }

    _inTag = false;
    _tagStack.pop();
}

inline void XmlWriter::writeCloseTagHead()
{
    assert(_inTag);

    _out.write(u8">\n");

    _inTag = false;
}

void XmlWriter::escapeAndWrite(const std::u8string_view text)
{
    using namespace std::string_view_literals;

    constexpr std::u8string_view toMatch = u8"&<>\"\'"sv;

    // Using `std::find_first_of` instead of `std::u8string_view::find_first_of`
    // as the compiler is able to inline it better.
    // (`std::u8string_view::find_first_of` is calling `memchr`)
    // (Profiling has confirmed std::find_first_of is the faster option)

    auto it = text.begin();

    while (it != text.end()) {
        {
            const auto end = std::find_first_of(it, text.end(),
                                                toMatch.begin(), toMatch.end());
            _out.write(std::u8string_view(it, end));
            it = end;
        }

        if (it != text.end()) {
            const char8_t c = *it;
            switch (c) {
            case u8'&':
                _out.write(u8"&amp;");
                break;

            case u8'<':
                _out.write(u8"&lt;");
                break;

            case u8'>':
                _out.write(u8"&gt;");
                break;

            case u8'"':
                _out.write(u8"&quot;");
                break;

            case u8'\'':
                _out.write(u8"&apos;");
                break;

            default:
                abort();
            }
            it++;
        }
    }
}

}
