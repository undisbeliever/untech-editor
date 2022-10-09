/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../aabb.h"
#include "../enummap.h"
#include "../ms8aabb.h"
#include "../string.h"
#include "../stringstream.h"
#include <cstdint>
#include <filesystem>
#include <span>
#include <stack>
#include <string>

namespace UnTech::Xml {

/**
 * The `XmlWriter` class simplifies the creation of XML documents.
 */
class XmlWriter {

private:
    StringStream _out;
    const std::filesystem::path _filePath;

    // Cannot use std::u8string_view here (stack-use-after-scope AddressSanitizer error)
    std::stack<std::u8string> _tagStack;

    bool _inTag;
    bool _useRelativePaths;

public:
    XmlWriter() = delete;
    XmlWriter(const XmlWriter&) = delete;
    XmlWriter(XmlWriter&&) = delete;
    XmlWriter& operator=(const XmlWriter&) = delete;
    XmlWriter& operator=(XmlWriter&&) = delete;

    explicit XmlWriter(const std::u8string_view doctype, size_t bufferSize = StringStream::default_initial_size)
        : XmlWriter(std::filesystem::path(), doctype, bufferSize)
    {
    }
    explicit XmlWriter(const std::filesystem::path& filePath, const std::u8string_view doctype, size_t bufferSize = StringStream::default_initial_size);
    ~XmlWriter();

    // returned string_view is only valid until the next write call.
    [[nodiscard]] std::u8string_view string_view() const { return _out.string_view(); }

    const std::filesystem::path& filePath() const { return _filePath; }

    void forceFullFilePaths() { _useRelativePaths = false; }

    void writeTag(const std::u8string_view name);

    void writeTagAttribute(const std::u8string_view name, const std::u8string_view value);
    void writeTagAttribute(const std::u8string_view name, const int value);
    void writeTagAttribute(const std::u8string_view name, const unsigned value);

    // no padding
    void writeTagAttributeHex(const std::u8string_view name, const unsigned value);
    // 6 digits of padding
    void writeTagAttributeHex6(const std::u8string_view name, const unsigned value);

    void writeTagAttribute(const std::u8string_view name, const char8_t* value)
    {
        writeTagAttribute(name, std::u8string_view(value));
    }

    void writeTagAttributeOptional(const std::u8string_view name, const std::u8string_view value);

    void writeText(const std::u8string_view text);

    void writeBase64(std::span<const uint8_t> data);

    void writeCloseTag();

    void writeTagAttributeFilename(const std::u8string_view name, const std::filesystem::path& path);

    inline void writeTagAttribute(const std::u8string_view name, const idstring& value)
    {
        writeTagAttribute(name, value.str());
    }

    inline void writeTagAttributeOptional(const std::u8string_view name, const idstring& value)
    {
        writeTagAttributeOptional(name, value.str());
    }

    inline void writeTagAttribute(const std::u8string_view name, bool v)
    {
        if (v) {
            writeTagAttribute(name, u8"true");
        }
    }

    inline void writeTagAttribute(const std::u8string_view name,
                                  const int_ms8_t& v)
    {
        writeTagAttribute(name, int(v));
    }

    template <typename T>
    inline void writeTagAttributeEnum(const std::u8string_view name, const T& value, const EnumMap<T>& enumMap)
    {
        writeTagAttribute_noEscape(name, enumMap.nameOf(value));
    }

    inline void writeTagAttributePoint(const point& p, const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeUpoint(const upoint& p, const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeUsize(const usize& s, const std::u8string_view widthName = u8"width", const std::u8string_view heightName = u8"height")
    {
        writeTagAttribute(widthName, s.width);
        writeTagAttribute(heightName, s.height);
    }

    inline void writeTagAttributeUrect(const urect& r, const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y", const std::u8string_view widthName = u8"width", const std::u8string_view heightName = u8"height")
    {
        writeTagAttribute(xName, r.x);
        writeTagAttribute(yName, r.y);
        writeTagAttribute(widthName, r.width);
        writeTagAttribute(heightName, r.height);
    }

    inline void writeTagAttributeMs8point(const ms8point& p, const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeMs8rect(const ms8rect& r, const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y", const std::u8string_view widthName = u8"width", const std::u8string_view heightName = u8"height")
    {
        writeTagAttribute(xName, r.x);
        writeTagAttribute(yName, r.y);
        writeTagAttribute(widthName, r.width);
        writeTagAttribute(heightName, r.height);
    }

private:
    void writeCloseTagHead();
    void escapeAndWrite(const std::u8string_view text);

    void writeTagAttribute_noEscape(const std::u8string_view name, const std::u8string_view value);
};

}
