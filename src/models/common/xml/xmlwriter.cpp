/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "xmlwriter.h"
#include "../base64.h"
#include "../file.h"
#include "../string.h"
#include <algorithm>
#include <cassert>
#include <iomanip>
#include <sstream>

namespace UnTech::Xml {

XmlWriter::XmlWriter(std::ostream& output, const std::filesystem::path& filePath, const std::string_view doctype)
    : _file(output)
    , _filePath(filePath)
    , _tagStack()
    , _inTag(false)
    , _useRelativePaths(!filePath.empty())
{
    _file << "<?xml version=\"1.0\" encoding=\"UTF_8\"?>\n";

    if (!doctype.empty()) {
        assert(isName(doctype));
        _file << "<!DOCTYPE " << doctype << ">\n";
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
        _file << "  ";
    }

    _file << "<" << name;

    _tagStack.emplace(name);
    _inTag = true;
}

void XmlWriter::writeTagAttribute_noEscape(const std::string_view name, const std::string_view value)
{
    assert(_inTag);
    assert(isName(name));

    _file << ' ' << name << "=\"" << value << '"';
}

void XmlWriter::writeTagAttribute(const std::string_view name, const std::string_view value)
{
    assert(_inTag);
    assert(isName(name));

    _file << ' ' << name << "=\"";
    writeEscapeAttribute(value);
    _file << '"';
}

void XmlWriter::writeTagAttribute(const std::string_view name, const char* value)
{
    assert(_inTag);
    assert(isName(name));

    _file << ' ' << name << "=\"";
    writeEscapeAttribute(value);
    _file << '"';
}

void XmlWriter::writeTagAttribute(const std::string_view name, const int value)
{
    assert(_inTag);
    assert(isName(name));
    assert(_file.width() == 0);
    assert((_file.flags() & std::ios_base::basefield) == std::ios_base::dec);

    _file << ' ' << name << "=\"" << value << '"';
}

void XmlWriter::writeTagAttribute(const std::string_view name, const unsigned value)
{
    assert(_inTag);
    assert(isName(name));
    assert(_file.width() == 0);
    assert((_file.flags() & std::ios_base::basefield) == std::ios_base::dec);

    _file << ' ' << name << "=\"" << value << '"';
}

void XmlWriter::writeTagAttributeFilename(const std::string_view name, const std::filesystem::path& path)
{
    std::filesystem::path p = _useRelativePaths ? path.lexically_relative(_filePath.parent_path())
                                                : std::filesystem::absolute(path);

    writeTagAttribute(name, p.generic_u8string());
}

void XmlWriter::writeTagAttributeHex(const std::string_view name, const unsigned value, unsigned width)
{
    assert(_inTag);
    assert(isName(name));

    _file << ' ' << name << "=\""
          << std::hex << std::setw(width) << std::setfill('0')
          << value
          << std::dec << std::setw(0)
          << '"';
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

    for (char c : text) {
        switch (c) {

        case '&':
            _file << "&amp;";
            break;

        case '<':
            _file << "&lt;";
            break;

        case '>':
            _file << "&gt;";
            break;

        default:
            _file << c;
        }
    }
}

void XmlWriter::writeBase64(const uint8_t* data, const size_t size)
{
    if (_inTag) {
        writeCloseTagHead();
    }

    Base64::encode(data, size, _file, _tagStack.size() * 2);
}

void XmlWriter::writeBase64(const std::vector<uint8_t>& data)
{
    writeBase64(data.data(), data.size());
}

void XmlWriter::writeCloseTag()
{
    if (_inTag) {
        _file << "/>\n";
    }
    else {
        assert(_tagStack.size() > 0);

        for (size_t i = 1; i < _tagStack.size(); i++) {
            _file << "  ";
        }

        _file << "</" << _tagStack.top() << ">\n";
    }

    _inTag = false;
    _tagStack.pop();
}

inline void XmlWriter::writeCloseTagHead()
{
    assert(_inTag);

    _file << ">\n";

    _inTag = false;
}

inline void XmlWriter::writeEscapeAttribute(const std::string_view text)
{
    for (char c : text) {
        switch (c) {

        case '&':
            _file << "&amp;";
            break;

        case '<':
            _file << "&lt;";
            break;

        case '>':
            _file << "&gt;";
            break;

        case '"':
            _file << "&quot;";
            break;

        default:
            _file << c;
        }
    }
}

inline void XmlWriter::writeEscapeAttribute(const char* text)
{
    const char* pos = text;
    while (*pos) {
        switch (*pos) {

        case '&':
            _file << "&amp;";
            break;

        case '<':
            _file << "&lt;";
            break;

        case '>':
            _file << "&gt;";
            break;

        case '"':
            _file << "&quot;";
            break;

        default:
            _file << *pos;
        }

        pos++;
    }
}

}
