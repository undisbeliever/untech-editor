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
#include "../optional.h"
#include <climits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace UnTech::Xml {

class unknown_tag_error : public xml_error {
public:
    unknown_tag_error(const XmlTag& tag)
        : xml_error(tag, "Unknown tag")
    {
    }
};

struct XmlTag {
    constexpr static unsigned MAX_ATTRIBUTES = 8;

    const std::string_view name;
    const XmlReader* xml;
    const unsigned lineNo;

    unsigned nAttributes;
    std::array<std::string_view, MAX_ATTRIBUTES> attrNames;
    std::array<std::string_view, MAX_ATTRIBUTES> attrRawValues;

public:
    XmlTag() = delete;
    XmlTag(const XmlTag&) = delete;
    XmlTag& operator=(const XmlTag&) = delete;
    XmlTag& operator=(XmlTag&&) = delete;

    // Allow return from function
    XmlTag(XmlTag&&) = default;

    XmlTag(const XmlReader* xml, std::string_view tagName, unsigned lineNo)
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
    void addAttribute(const std::string_view aName, const std::string_view rawValue)
    {
        if (nAttributes >= MAX_ATTRIBUTES) {
            throw xml_error(*this, "Too many attributes");
        }

        attrNames.at(nAttributes) = aName;
        attrRawValues.at(nAttributes) = rawValue;

        nAttributes++;
    }

    // Returns a value > MAX_ATTRUBUTES if aName not found
    inline unsigned findAttributeIndex(const std::string_view aName) const
    {
        assert(nAttributes <= MAX_ATTRIBUTES);

        for (const unsigned i : range(nAttributes)) {
            if (attrNames.at(i) == aName) {
                return i;
            }
        }

        static_assert(INT_MAX > MAX_ATTRIBUTES);
        return INT_MAX;
    }

    // NOTE: Does not unescape any `&` characters in the attribute value.
    inline std::string_view getAttribute_rawValue(const std::string_view aName) const
    {
        assert(nAttributes <= MAX_ATTRIBUTES);

        for (const unsigned i : range(nAttributes)) {
            if (attrNames.at(i) == aName) {
                return attrRawValues.at(i);
            }
        }

        throw xml_error(*this, aName, "Missing attribute");
    }

public:
    operator bool() const { return !name.empty(); }

    bool hasAttribute(const std::string_view aName) const
    {
        assert(nAttributes <= MAX_ATTRIBUTES);

        for (const unsigned i : range(nAttributes)) {
            if (attrNames.at(i) == aName) {
                return true;
            }
        }

        return false;
    }

    inline std::string getAttribute(const std::string_view aName) const
    {
        return unescapeXmlString(getAttribute_rawValue(aName));
    }

    inline std::string getAttributeOrEmpty(const std::string_view aName) const
    {
        const auto i = findAttributeIndex(aName);

        if (i < MAX_ATTRIBUTES) {
            return unescapeXmlString(attrRawValues.at(i));
        }
        else {
            return std::string();
        }
    }

    inline idstring getAttributeId(const std::string_view aName) const
    {
        // No need to escape value - only alnum and underscore characters are valid
        const auto id = idstring::fromString(getAttribute_rawValue(aName));

        if (id.isValid()) {
            return id;
        }
        else {
            throw xml_error(*this, aName, "Invalid id");
        }
    }

    inline idstring getAttributeOptionalId(const std::string_view aName) const
    {
        const auto i = findAttributeIndex(aName);

        if (i < MAX_ATTRIBUTES) {
            // No need to escape value - only alnum and underscore characters are valid
            const auto id = idstring::fromString(attrRawValues.at(i));

            if (id.isValid()) {
                return id;
            }
            else {
                throw xml_error(*this, aName, "Invalid idstring");
            }
        }
        else {
            return idstring();
        }
    }

    template <class MapT>
    inline idstring getAttributeUniqueId(const std::string_view aName,
                                         const MapT& map) const
    {
        // No need to escape value - only alnum and underscore characters are valid
        const auto id = idstring::fromString(getAttribute_rawValue(aName));

        if (map.contains(id)) {
            throw xml_error(*this, aName, "id already exists");
        }
        return id;
    }

    inline optional<std::string> getOptionalAttribute(const std::string_view aName) const
    {
        const auto i = findAttributeIndex(aName);

        if (i < MAX_ATTRIBUTES) {
            return unescapeXmlString(attrRawValues.at(i));
        }
        else {
            return optional<std::string>();
        }
    }

    inline int getAttributeInteger(const std::string_view aName) const
    {
        static_assert(std::is_same_v<int, int32_t>);

        // No need to escape value - only digit characters and '-' are valid
        const auto v = String::toInt32(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, "Cannot convert value to int32_t");
        }
        return v.value();
    }

    inline int getAttributeInteger(const std::string_view aName, int min, int max) const
    {
        const int i = getAttributeInteger(aName);

        if (i < min) {
            throw xml_error(*this, aName, "Number too small");
        }
        if (i > max) {
            throw xml_error(*this, aName, "Number too small");
        }
        return i;
    }

    inline unsigned getAttributeUnsigned(const std::string_view aName) const
    {
        static_assert(std::is_same_v<unsigned, uint32_t>);

        const auto v = String::toUint32(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, "Cannot convert value to uint32_t");
        }

        return v.value();
    }

    inline unsigned getAttributeUnsigned(const std::string_view aName, unsigned min, unsigned max = UINT_MAX) const
    {
        const unsigned v = getAttributeUnsigned(aName);

        if (v < min) {
            throw xml_error(*this, aName, "Number too small");
        }
        if (v > max) {
            throw xml_error(*this, aName, "Number too large");
        }
        return v;
    }

    inline uint8_t getAttributeUint8(const std::string_view aName) const
    {
        const std::optional<uint8_t> v = String::toUint8(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, "Cannot convert value to uint8_t");
        }
        return *v;
    }

    inline uint16_t getAttributeUint16(const std::string_view aName) const
    {
        const std::optional<uint16_t> v = String::toUint16(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, "Cannot convert value to uint16_t");
        }
        return *v;
    }

    inline int_ms8_t getAttributeIntMs8(const std::string_view aName) const
    {
        return int_ms8_t(getAttributeInteger(aName, int_ms8_t::MIN, int_ms8_t::MAX));
    }

    inline bool getAttributeBoolean(const std::string_view aName, bool def = false) const
    {
        const auto i = findAttributeIndex(aName);

        if (i < MAX_ATTRIBUTES) {
            // No need to escape value - "true" or "false" allowed here
            const auto& value = attrRawValues.at(i);

            if (value == "true") {
                return true;
            }
            else if (value == "false") {
                return false;
            }
            else {
                throw xml_error(*this, aName, "Expected true or false");
            }
        }
        else {
            return def;
        }
    }

    inline std::filesystem::path getAttributeFilename(const std::string_view aName) const
    {
        const auto xmlPath = xml->filePath();

        if (xmlPath.empty()) {
            throw xml_error(*this, aName, "XML file has no path");
        }

        // Must escape attribute value
        auto path = std::filesystem::u8path(getAttribute(aName));
        if (path.empty()) {
            throw xml_error(*this, aName, "Expected filename");
        }

        return xmlPath.parent_path() / path.make_preferred();
    }

    template <typename T>
    inline T getAttributeEnum(const std::string_view aName, const EnumMap<T>& enumMap) const
    {
        // No need to escape value - only alnum, dash and underscore characters are valid
        auto it = enumMap.find(getAttribute_rawValue(aName));
        if (it != enumMap.end()) {
            return it->second;
        }
        else {
            throw xml_error(*this, aName, "Invalid value");
        }
    }

    template <typename T>
    inline T getAttributeOptionalEnum(const std::string_view aName, const EnumMap<T>& enumMap, const T default_value) const
    {
        const auto i = findAttributeIndex(aName);

        if (i < MAX_ATTRIBUTES) {
            // No need to escape value - "true" or "false" allowed here
            const auto& value = attrRawValues.at(i);

            auto eIt = enumMap.find(value);
            if (eIt != enumMap.end()) {
                return eIt->second;
            }
        }
        return default_value;
    }

    inline unsigned getAttributeUnsignedHex(const std::string_view aName) const
    {
        // No need to escape value, only 0-9 and a-f characters are valid
        auto v = String::hexToUint32(getAttribute_rawValue(aName));

        if (!v) {
            throw xml_error(*this, aName, "Not a hexadecimal number");
        }
        return v.value();
    }

    inline point getAttributePoint(const std::string_view xName = "x", const std::string_view yName = "y") const
    {
        unsigned x = getAttributeInteger(xName);
        unsigned y = getAttributeInteger(yName);

        return point(x, y);
    }

    inline upoint getAttributeUpoint(const std::string_view xName = "x", const std::string_view yName = "y") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        return upoint(x, y);
    }

    inline usize getAttributeUsize(const std::string_view widthName = "width", const std::string_view heightName = "height") const
    {
        unsigned width = getAttributeUnsigned(widthName);
        unsigned height = getAttributeUnsigned(heightName);

        return usize(width, height);
    }

    inline urect getAttributeUrect(const std::string_view xName = "x", const std::string_view yName = "y", const std::string_view widthName = "width", const std::string_view heightName = "height") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        unsigned width = getAttributeUnsigned(widthName, 1, UINT_MAX);
        unsigned height = getAttributeUnsigned(heightName, 1, UINT_MAX);

        return urect(x, y, width, height);
    }

    inline urect getAttributeUrect(const usize& minimumSize, const std::string_view xName = "x", const std::string_view yName = "y", const std::string_view widthName = "width", const std::string_view heightName = "height") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        unsigned width = getAttributeUnsigned(widthName, minimumSize.width, UINT_MAX);
        unsigned height = getAttributeUnsigned(heightName, minimumSize.height, UINT_MAX);

        return urect(x, y, width, height);
    }

    inline ms8point getAttributeMs8point(const std::string_view xName = "x", const std::string_view yName = "y") const
    {
        int_ms8_t x = getAttributeIntMs8(xName);
        int_ms8_t y = getAttributeIntMs8(yName);

        return ms8point(x, y);
    }

    inline ms8rect getAttributeMs8rect(const std::string_view xName = "x", const std::string_view yName = "y", const std::string_view widthName = "width", const std::string_view heightName = "height") const
    {
        int_ms8_t x = getAttributeIntMs8(xName);
        int_ms8_t y = getAttributeIntMs8(yName);
        unsigned width = getAttributeUint8(widthName);
        unsigned height = getAttributeUint8(heightName);

        return ms8rect(x, y, width, height);
    }

    std::string generateErrorString(const std::string_view msg) const;
    std::string generateErrorString(const std::string_view aName, const std::string_view msg) const;
};

}
