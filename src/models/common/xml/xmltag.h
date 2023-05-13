/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "xmlreader.h"
#include "../aabb.h"
#include "../enummap.h"
#include "../file.h"
#include "../idstring.h"
#include "../int_ms8_t.h"
#include "../iterators.h"
#include "../ms8aabb.h"
#include <climits>
#include <memory>
#include <string>
#include <string_view>

namespace UnTech::Xml {

class unknown_tag_error : public xml_error {
public:
    unknown_tag_error(const XmlTag& tag)
        : xml_error(tag, u8"Unknown tag")
    {
    }
};

struct XmlTag {
    constexpr static unsigned MAX_ATTRIBUTES = 8;

    const std::u8string_view name;
    const XmlReader* xml;
    const unsigned lineNo;

    unsigned nAttributes;
    std::array<std::u8string_view, MAX_ATTRIBUTES> attrNames;
    std::array<std::u8string_view, MAX_ATTRIBUTES> attrRawValues;

public:
    XmlTag() = delete;
    XmlTag(const XmlTag&) = delete;
    XmlTag& operator=(const XmlTag&) = delete;
    XmlTag& operator=(XmlTag&&) = delete;

    ~XmlTag() = default;

    // Allow return from function
    XmlTag(XmlTag&&) = default;

    XmlTag(const XmlReader* xml, std::u8string_view tagName, unsigned lineNo)
        : name(tagName)
        , xml(xml)
        , lineNo(lineNo)
        , nAttributes(0)
        , attrNames()
        , attrRawValues()
    {
    }

private:
    friend class UnTech::Xml::XmlReader;
    void addAttribute(const std::u8string_view aName, const std::u8string_view rawValue)
    {
        if (nAttributes >= MAX_ATTRIBUTES) {
            throw xml_error(*this, u8"Too many attributes");
        }

        attrNames.at(nAttributes) = aName;
        attrRawValues.at(nAttributes) = rawValue;

        nAttributes++;
    }

    // NOTE: Does not unescape any `&` characters in the attribute value.
    [[nodiscard]] inline std::u8string_view getAttribute_rawValue(const std::u8string_view aName) const
    {
        assert(nAttributes <= MAX_ATTRIBUTES);

        for (const unsigned i : range(nAttributes)) {
            if (attrNames.at(i) == aName) {
                return attrRawValues.at(i);
            }
        }

        throw xml_error(*this, aName, u8"Missing attribute");
    }

    // NOTE: Does not unescape any `&` characters in the attribute value.
    [[nodiscard]] inline std::optional<std::u8string_view> getOptionalAttribute_rawValue(const std::u8string_view aName) const
    {
        assert(nAttributes <= MAX_ATTRIBUTES);

        for (const unsigned i : range(nAttributes)) {
            if (attrNames.at(i) == aName) {
                return attrRawValues.at(i);
            }
        }
        return std::nullopt;
    }

public:
    operator bool() const { return !name.empty(); }

    [[nodiscard]] bool hasAttribute(const std::u8string_view aName) const
    {
        assert(nAttributes <= MAX_ATTRIBUTES);

        for (const unsigned i : range(nAttributes)) {
            if (attrNames.at(i) == aName) {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] inline std::u8string getAttribute(const std::u8string_view aName) const
    {
        return unescapeXmlString(getAttribute_rawValue(aName));
    }

    [[nodiscard]] inline std::u8string getAttributeOrEmpty(const std::u8string_view aName) const
    {
        if (const auto r = getOptionalAttribute_rawValue(aName)) {
            return unescapeXmlString(*r);
        }
        else {
            return {};
        }
    }

    [[nodiscard]] inline std::optional<std::u8string> getOptionalAttribute(const std::u8string_view aName) const
    {
        if (auto r = getOptionalAttribute_rawValue(aName)) {
            return unescapeXmlString(*r);
        }
        else {
            return std::nullopt;
        }
    }

    [[nodiscard]] inline idstring getAttributeId(const std::u8string_view aName) const
    {
        // No need to escape value - only alnum and underscore characters are valid
        auto id = idstring::fromString(getAttribute_rawValue(aName));

        if (id.isValid()) {
            return id;
        }
        else {
            throw xml_error(*this, aName, u8"Invalid id");
        }
    }

    [[nodiscard]] inline idstring getAttributeOptionalId(const std::u8string_view aName) const
    {
        // No need to escape value - only alnum and underscore characters are valid
        if (const auto r = getOptionalAttribute_rawValue(aName)) {
            auto id = idstring::fromString(*r);
            if (id.isValid()) {
                return id;
            }
            else {
                throw xml_error(*this, aName, u8"Invalid idstring");
            }
        }
        else {
            return {};
        }
    }

    [[nodiscard]] inline int getAttributeInteger(const std::u8string_view aName) const
    {
        static_assert(std::is_same_v<int, int32_t>);

        // No need to escape value - only digit characters and '-' are valid
        const auto v = String::toInt32(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, u8"Cannot convert value to int32_t");
        }
        return v.value();
    }

    [[nodiscard]] inline int getAttributeInteger(const std::u8string_view aName, int min, int max) const
    {
        const int i = getAttributeInteger(aName);

        if (i < min) {
            throw xml_error(*this, aName, u8"Number too small");
        }
        if (i > max) {
            throw xml_error(*this, aName, u8"Number too small");
        }
        return i;
    }

    [[nodiscard]] inline unsigned getAttributeUnsigned(const std::u8string_view aName) const
    {
        static_assert(std::is_same_v<unsigned, uint32_t>);

        const auto v = String::toUint32(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, u8"Cannot convert value to uint32_t");
        }

        return v.value();
    }

    [[nodiscard]] inline unsigned getAttributeUnsigned(const std::u8string_view aName, unsigned min, unsigned max = UINT_MAX) const
    {
        const unsigned v = getAttributeUnsigned(aName);

        if (v < min) {
            throw xml_error(*this, aName, u8"Number too small");
        }
        if (v > max) {
            throw xml_error(*this, aName, u8"Number too large");
        }
        return v;
    }

    [[nodiscard]] inline uint8_t getAttributeUint8(const std::u8string_view aName) const
    {
        const std::optional<uint8_t> v = String::toUint8(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, u8"Cannot convert value to uint8_t");
        }
        return *v;
    }

    [[nodiscard]] inline uint16_t getAttributeUint16(const std::u8string_view aName) const
    {
        const std::optional<uint16_t> v = String::toUint16(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, u8"Cannot convert value to uint16_t");
        }
        return *v;
    }

    [[nodiscard]] inline int_ms8_t getAttributeIntMs8(const std::u8string_view aName) const
    {
        return getAttributeInteger(aName, int_ms8_t::MIN, int_ms8_t::MAX);
    }

    [[nodiscard]] inline bool getAttributeBoolean(const std::u8string_view aName, bool def = false) const
    {
        // No need to escape value - "true" or "false" allowed here
        if (auto r = getOptionalAttribute_rawValue(aName)) {
            if (r == u8"true") {
                return true;
            }
            else if (r == u8"false") {
                return false;
            }
            else {
                throw xml_error(*this, aName, u8"Expected true or false");
            }
        }
        else {
            return def;
        }
    }

    [[nodiscard]] inline std::filesystem::path getAttributeFilename(const std::u8string_view aName) const
    {
        const auto xmlPath = xml->filePath();

        if (xmlPath.empty()) {
            throw xml_error(*this, aName, u8"XML file has no path");
        }

        // Must escape attribute value
        const auto a = getAttribute(aName);

        // ::ANNOY std::filesystem::u8path() is deprecated in C++20::
        // ::TODO confirm this does what I want in Windows:;
        static_assert(__cplusplus >= 202002L);
        static_assert(std::is_same_v<decltype(a), const std::u8string>);
        auto path = std::filesystem::path(a);

        if (path.empty()) {
            throw xml_error(*this, aName, u8"Expected filename");
        }

        return xmlPath.parent_path() / path.make_preferred();
    }

    template <typename T>
    inline T getAttributeEnum(const std::u8string_view aName, const EnumMap<T>& enumMap) const
    {
        // No need to escape value - only alnum, dash and underscore characters are valid
        auto it = enumMap.find(getAttribute_rawValue(aName));
        if (it != enumMap.end()) {
            return it->second;
        }
        else {
            throw xml_error(*this, aName, u8"Invalid value");
        }
    }

    template <typename T>
    inline T getAttributeOptionalEnum(const std::u8string_view aName, const EnumMap<T>& enumMap, const T default_value) const
    {
        // No need to escape value - only alnum, dash and underscore characters are valid
        if (const auto rawValue = getOptionalAttribute_rawValue(aName)) {
            if (!rawValue->empty()) {
                auto eIt = enumMap.find(*rawValue);
                if (eIt != enumMap.end()) {
                    return eIt->second;
                }
                else {
                    throw xml_error(*this, aName, u8"Invalid value");
                }
            }
        }
        return default_value;
    }

    [[nodiscard]] inline unsigned getAttributeUnsignedHex(const std::u8string_view aName) const
    {
        // No need to escape value, only 0-9 and a-f characters are valid
        auto v = String::hexToUint32(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, u8"Not a hexadecimal number");
        }
        return v.value();
    }

    [[nodiscard]] inline point getAttributePoint(const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y") const
    {
        int x = getAttributeInteger(xName);
        int y = getAttributeInteger(yName);

        return { x, y };
    }

    [[nodiscard]] inline upoint getAttributeUpoint(const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        return { x, y };
    }

    [[nodiscard]] inline usize getAttributeUsize(const std::u8string_view widthName = u8"width", const std::u8string_view heightName = u8"height") const
    {
        unsigned width = getAttributeUnsigned(widthName);
        unsigned height = getAttributeUnsigned(heightName);

        return { width, height };
    }

    [[nodiscard]] inline urect getAttributeUrect(const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y", const std::u8string_view widthName = u8"width", const std::u8string_view heightName = u8"height") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        unsigned width = getAttributeUnsigned(widthName, 1, UINT_MAX);
        unsigned height = getAttributeUnsigned(heightName, 1, UINT_MAX);

        return { x, y, width, height };
    }

    [[nodiscard]] inline urect getAttributeUrect(const usize& minimumSize, const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y", const std::u8string_view widthName = u8"width", const std::u8string_view heightName = u8"height") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        unsigned width = getAttributeUnsigned(widthName, minimumSize.width, UINT_MAX);
        unsigned height = getAttributeUnsigned(heightName, minimumSize.height, UINT_MAX);

        return { x, y, width, height };
    }

    [[nodiscard]] inline ms8point getAttributeMs8point(const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y") const
    {
        int_ms8_t x = getAttributeIntMs8(xName);
        int_ms8_t y = getAttributeIntMs8(yName);

        return { x, y };
    }

    [[nodiscard]] inline ms8rect getAttributeMs8rect(const std::u8string_view xName = u8"x", const std::u8string_view yName = u8"y", const std::u8string_view widthName = u8"width", const std::u8string_view heightName = u8"height") const
    {
        int_ms8_t x = getAttributeIntMs8(xName);
        int_ms8_t y = getAttributeIntMs8(yName);
        uint8_t width = getAttributeUint8(widthName);
        uint8_t height = getAttributeUint8(heightName);

        return { x, y, width, height };
    }

    [[nodiscard]] std::u8string generateErrorString(const std::u8string_view msg) const;
    [[nodiscard]] std::u8string generateErrorString(const std::u8string_view aName, const std::u8string_view msg) const;
};

}
