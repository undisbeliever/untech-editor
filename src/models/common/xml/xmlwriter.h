/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "xml.h"
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

public:
    XmlWriter() = delete;
    XmlWriter(const XmlWriter&) = delete;
    XmlWriter(XmlWriter&&) = delete;
    XmlWriter& operator=(const XmlWriter&) = delete;
    XmlWriter& operator=(XmlWriter&&) = delete;

    XmlWriter(std::ostream& output, const std::string& doctype)
        : XmlWriter(output, std::filesystem::path(), doctype)
    {
    }
    XmlWriter(std::ostream& output, const std::filesystem::path& filePath, const std::string& doctype);
    ~XmlWriter();

    const std::filesystem::path& filePath() const { return _filePath; }

    void forceFullFilePaths() { _useRelativePaths = false; }

    void writeTag(const std::string& name);

    void writeTagAttribute(const std::string& name, const std::string& value);
    void writeTagAttribute(const std::string& name, const char* value);
    void writeTagAttribute(const std::string& name, const int value);
    void writeTagAttribute(const std::string& name, const unsigned value);
    void writeTagAttributeHex(const std::string& name, const unsigned value, unsigned width);

    void writeTagAttributeOptional(const std::string& name, const std::string& value);

    void writeText(const std::string& text);

    void writeBase64(const uint8_t* data, const size_t size);
    void writeBase64(const std::vector<uint8_t>& data);

    template <size_t N>
    void writeBase64(const std::array<uint8_t, N>& data)
    {
        writeBase64(data.data(), data.size());
    }

    void writeCloseTag();

    void writeTagAttributeFilename(const std::string& name, const std::filesystem::path& path);

    inline void writeTagAttribute(const std::string& name, bool v)
    {
        if (v) {
            writeTagAttribute(name, "true");
        }
    }

    template <typename T, T MIN, T MAX>
    inline void writeTagAttribute(const std::string& name,
                                  const ClampedType<T, MIN, MAX>& v)
    {
        writeTagAttribute(name, T(v));
    }

    template <class T>
    inline void writeTagAttributeEnum(const std::string& name, const T& value)
    {
        const auto& enumMap = T::enumMap;
        writeTagAttribute(name, enumMap.nameOf(value.value()));
    }

    template <typename T>
    inline void writeTagAttributeEnum(const std::string& name, const T& value, const EnumMap<T>& enumMap)
    {
        writeTagAttribute(name, enumMap.nameOf(value));
    }

    inline void writeTagAttributePoint(const point& p, const std::string& xName = "x", const std::string& yName = "y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeUpoint(const upoint& p, const std::string& xName = "x", const std::string& yName = "y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeUsize(const usize& s, const std::string& widthName = "width", const std::string& heightName = "height")
    {
        writeTagAttribute(widthName, s.width);
        writeTagAttribute(heightName, s.height);
    }

    inline void writeTagAttributeUrect(const urect& r, const std::string& xName = "x", const std::string& yName = "y", const std::string& widthName = "width", const std::string& heightName = "height")
    {
        writeTagAttribute(xName, r.x);
        writeTagAttribute(yName, r.y);
        writeTagAttribute(widthName, r.width);
        writeTagAttribute(heightName, r.height);
    }

    inline void writeTagAttributeMs8point(const ms8point& p, const std::string& xName = "x", const std::string& yName = "y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeMs8rect(const ms8rect& r, const std::string& xName = "x", const std::string& yName = "y", const std::string& widthName = "width", const std::string& heightName = "height")
    {
        writeTagAttribute(xName, r.x);
        writeTagAttribute(yName, r.y);
        writeTagAttribute(widthName, r.width);
        writeTagAttribute(heightName, r.height);
    }

private:
    void writeCloseTagHead();
    void writeEscapeAttribute(const std::string& text);
    void writeEscapeAttribute(const char* text);

private:
    std::ostream& _file;
    const std::filesystem::path _filePath;
    std::stack<std::string> _tagStack;
    bool _inTag;
    bool _useRelativePaths;
};
}
}
