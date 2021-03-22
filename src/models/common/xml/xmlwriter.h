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
#include <cstdint>
#include <filesystem>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

namespace UnTech {
namespace Xml {

/**
 * The `XmlWriter` class simplifies the creation of XML documents.
 */
class XmlWriter {

private:
    std::ostream& _file;
    const std::filesystem::path _filePath;

    // Cannot use std::string_view here (stack-use-after-scope AddressSanitizer error)
    std::stack<std::string> _tagStack;

    bool _inTag;
    bool _useRelativePaths;

public:
    XmlWriter() = delete;
    XmlWriter(const XmlWriter&) = delete;
    XmlWriter(XmlWriter&&) = delete;
    XmlWriter& operator=(const XmlWriter&) = delete;
    XmlWriter& operator=(XmlWriter&&) = delete;

    XmlWriter(std::ostream& output, const std::string_view doctype)
        : XmlWriter(output, std::filesystem::path(), doctype)
    {
    }
    XmlWriter(std::ostream& output, const std::filesystem::path& filePath, const std::string_view doctype);
    ~XmlWriter();

    const std::filesystem::path& filePath() const { return _filePath; }

    void forceFullFilePaths() { _useRelativePaths = false; }

    void writeTag(const std::string_view name);

    void writeTagAttribute(const std::string_view name, const std::string_view value);
    void writeTagAttribute(const std::string_view name, const int value);
    void writeTagAttribute(const std::string_view name, const unsigned value);
    void writeTagAttributeHex(const std::string_view name, const unsigned value, unsigned width);

    void writeTagAttribute(const std::string_view name, const char* value)
    {
        writeTagAttribute(name, std::string_view(value));
    }

    void writeTagAttributeOptional(const std::string_view name, const std::string_view value);

    void writeText(const std::string_view text);

    void writeBase64(const uint8_t* data, const size_t size);
    void writeBase64(const std::vector<uint8_t>& data);

    template <size_t N>
    void writeBase64(const std::array<uint8_t, N>& data)
    {
        writeBase64(data.data(), data.size());
    }

    void writeCloseTag();

    void writeTagAttributeFilename(const std::string_view name, const std::filesystem::path& path);

    inline void writeTagAttribute(const std::string_view name, const idstring& value)
    {
        writeTagAttribute(name, value.str());
    }

    inline void writeTagAttributeOptional(const std::string_view name, const idstring& value)
    {
        writeTagAttributeOptional(name, value.str());
    }

    inline void writeTagAttribute(const std::string_view name, bool v)
    {
        if (v) {
            writeTagAttribute(name, "true");
        }
    }

    template <typename T, T MIN, T MAX>
    inline void writeTagAttribute(const std::string_view name,
                                  const ClampedType<T, MIN, MAX>& v)
    {
        writeTagAttribute(name, T(v));
    }

    template <class T>
    inline void writeTagAttributeEnum(const std::string_view name, const T& value)
    {
        const auto& enumMap = T::enumMap;
        writeTagAttribute_noEscape(name, enumMap.nameOf(value.value()));
    }

    template <typename T>
    inline void writeTagAttributeEnum(const std::string_view name, const T& value, const EnumMap<T>& enumMap)
    {
        writeTagAttribute_noEscape(name, enumMap.nameOf(value));
    }

    inline void writeTagAttributePoint(const point& p, const std::string_view xName = "x", const std::string_view yName = "y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeUpoint(const upoint& p, const std::string_view xName = "x", const std::string_view yName = "y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeUsize(const usize& s, const std::string_view widthName = "width", const std::string_view heightName = "height")
    {
        writeTagAttribute(widthName, s.width);
        writeTagAttribute(heightName, s.height);
    }

    inline void writeTagAttributeUrect(const urect& r, const std::string_view xName = "x", const std::string_view yName = "y", const std::string_view widthName = "width", const std::string_view heightName = "height")
    {
        writeTagAttribute(xName, r.x);
        writeTagAttribute(yName, r.y);
        writeTagAttribute(widthName, r.width);
        writeTagAttribute(heightName, r.height);
    }

    inline void writeTagAttributeMs8point(const ms8point& p, const std::string_view xName = "x", const std::string_view yName = "y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeMs8rect(const ms8rect& r, const std::string_view xName = "x", const std::string_view yName = "y", const std::string_view widthName = "width", const std::string_view heightName = "height")
    {
        writeTagAttribute(xName, r.x);
        writeTagAttribute(yName, r.y);
        writeTagAttribute(widthName, r.width);
        writeTagAttribute(heightName, r.height);
    }

private:
    void writeCloseTagHead();
    void escapeAndWrite(const std::string_view text);

    void writeTagAttribute_noEscape(const std::string_view name, const std::string_view value);
};
}
}
