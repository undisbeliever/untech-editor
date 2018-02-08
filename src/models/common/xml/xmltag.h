/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../aabb.h"
#include "../clampedinteger.h"
#include "../file.h"
#include "../idstring.h"
#include "../int_ms8_t.h"
#include "../ms8aabb.h"
#include "../optional.h"
#include <climits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace UnTech {
namespace Xml {

class unknown_tag_error : public xml_error {
public:
    unknown_tag_error(XmlTag& tag)
        : xml_error(tag, "Unknown tag")
    {
    }
};

struct XmlTag {
    XmlTag() = delete;
    XmlTag(const XmlTag&) = delete;
    XmlTag(XmlTag&&) = delete;
    XmlTag& operator=(const XmlTag&) = delete;
    XmlTag& operator=(XmlTag&&) = delete;

    XmlTag(const XmlReader* xml, std::string tagName, unsigned lineNo)
        : name(tagName)
        , attributes()
        , xml(xml)
        , lineNo(lineNo)
    {
    }

    bool hasAttribute(const std::string& aName) const
    {
        return attributes.find(aName) != attributes.end();
    }

    inline std::string getAttribute(const std::string& aName) const
    {
        auto it = attributes.find(aName);
        if (it != attributes.end()) {
            return it->second;
        }
        else {
            throw xml_error(*this, aName, "Missing attribute");
        }
    }

    inline idstring getAttributeId(const std::string& aName) const
    {
        idstring id = getAttribute(aName);

        if (id.isValid()) {
            return id;
        }
        else {
            throw xml_error(*this, aName, "Invalid id");
        }
    }

    template <class MapT>
    inline idstring getAttributeUniqueId(const std::string& aName,
                                         const MapT& map) const
    {
        std::string id = getAttributeId(aName);

        if (map.contains(id)) {
            throw xml_error(*this, aName, "id already exists");
        }
        return id;
    }

    inline optional<std::string> getOptionalAttribute(const std::string& aName) const
    {
        auto it = attributes.find(aName);
        if (it != attributes.end()) {
            return it->second;
        }
        else {
            return optional<std::string>();
        }
    }

    inline int getAttributeInteger(const std::string& aName) const
    {
        auto v = String::toInt(getAttribute(aName));

        if (!v) {
            throw xml_error(*this, aName, "Not a number");
        }
        return v.value();
    }

    inline int getAttributeInteger(const std::string& aName, int min, int max) const
    {
        int i = getAttributeInteger(aName);

        if (i < min) {
            throw xml_error(*this, aName, "Number too small");
        }
        if (i > max) {
            throw xml_error(*this, aName, "Number too small");
        }
        return i;
    }

    template <class T>
    inline T getAttributeClamped(const std::string& aName)
    {
        static_assert(std::is_integral<typename T::TYPE>::value, "not integral");

        if (std::is_signed<typename T::TYPE>::value) {
            return getAttributeInteger(aName, T::MIN, T::MAX);
        }
        else if (std::is_unsigned<typename T::TYPE>::value) {
            return getAttributeUnsigned(aName, T::MIN, T::MAX);
        }
    }

    inline unsigned getAttributeUnsigned(const std::string& aName, unsigned min = 0, unsigned max = UINT_MAX) const
    {
        auto v = String::toLong(getAttribute(aName));

        if (!v) {
            throw xml_error(*this, aName, "Not a number");
        }
        if (v.value() < 0) {
            throw xml_error(*this, aName, "Only positive numbers allowed");
        }
        if ((unsigned long)v.value() < min) {
            throw xml_error(*this, aName, "Number too small");
        }
        if ((unsigned long)v.value() > max) {
            throw xml_error(*this, aName, "Number too large");
        }
        return (unsigned)v.value();
    }

    inline int8_t getAttributeInt8(const std::string& aName) const
    {
        return (int8_t)getAttributeInteger(aName, INT8_MIN, INT8_MAX);
    }

    inline uint8_t getAttributeUint8(const std::string& aName) const
    {
        return (uint8_t)getAttributeUnsigned(aName, 0, UINT8_MAX);
    }

    inline uint8_t getAttributeUint8NotZero(const std::string& aName) const
    {
        return (uint8_t)getAttributeUnsigned(aName, 1, UINT8_MAX);
    }

    inline int_ms8_t getAttributeIntMs8(const std::string& aName) const
    {
        return int_ms8_t(getAttributeInteger(aName, int_ms8_t::MIN, int_ms8_t::MAX));
    }

    inline bool getAttributeBoolean(const std::string& aName, bool def = false) const
    {
        auto v = getOptionalAttribute(aName);

        if (v) {
            if (v.value() == "true") {
                return true;
            }
            else if (v.value() == "false") {
                return false;
            }
            else {
                throw xml_error(*this, aName, "Expected true or false");
            }
        }

        return def;
    }

    inline std::string getAttributeFilename(const std::string& aName) const
    {
        auto v = getAttribute(aName);

        return File::joinPath(xml->dirname(), v);
    }

    template <class T>
    inline T getAttributeSimpleClass(const std::string& name) const
    {
        try {
            return T(getAttribute(name));
        }
        catch (const xml_error&) {
            throw;
        }
        catch (const std::out_of_range& ex) {
            throw xml_error(*this, name, "Invalid value");
        }
        catch (const std::exception& ex) {
            throw xml_error(*this, name, ex.what());
        }
    }

    inline unsigned getAttributeUnsignedHex(const std::string& aName) const
    {
        auto v = String::hexToUnsigned(getAttribute(aName));

        if (!v) {
            throw xml_error(*this, aName, "Not a hexadecimal number");
        }
        return v.value();
    }

    inline upoint getAttributeUpoint(const std::string& xName = "x", const std::string& yName = "y") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        return upoint(x, y);
    }

    inline usize getAttributeUsize(const std::string& widthName = "width", const std::string& heightName = "height") const
    {
        unsigned width = getAttributeUnsigned(widthName);
        unsigned height = getAttributeUnsigned(heightName);

        return usize(width, height);
    }

    inline upoint getAttributeUpointInside(const urect& container, const std::string& xName = "x", const std::string& yName = "y") const
    {
        unsigned x = getAttributeUnsigned(xName, 0, container.width);
        unsigned y = getAttributeUnsigned(yName, 0, container.height);

        return upoint(x, y);
    }

    inline upoint getAttributeUpointInside(const urect& container, unsigned squareSize, const std::string& xName = "x", const std::string& yName = "y") const
    {
        if (container.width < squareSize || container.height < squareSize) {
            throw xml_error(*this, "upoint outside urect");
        }
        unsigned x = getAttributeUnsigned(xName, 0, container.width - squareSize);
        unsigned y = getAttributeUnsigned(yName, 0, container.height - squareSize);

        return upoint(x, y);
    }

    inline urect getAttributeUrect(const std::string& xName = "x", const std::string yName = "y", const std::string& widthName = "width", const std::string& heightName = "height") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        unsigned width = getAttributeUnsigned(widthName, 1, UINT_MAX);
        unsigned height = getAttributeUnsigned(heightName, 1, UINT_MAX);

        return urect(x, y, width, height);
    }

    inline urect getAttributeUrect(const usize& minimumSize, const std::string& xName = "x", const std::string yName = "y", const std::string& widthName = "width", const std::string& heightName = "height") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        unsigned width = getAttributeUnsigned(widthName, minimumSize.width, UINT_MAX);
        unsigned height = getAttributeUnsigned(heightName, minimumSize.height, UINT_MAX);

        return urect(x, y, width, height);
    }

    inline urect getAttributeUrectInside(const urect& container, const std::string& xName = "x", const std::string& yName = "y", const std::string& widthName = "width", const std::string& heightName = "height") const
    {
        unsigned x = getAttributeUnsigned(xName, 0, container.width);
        unsigned y = getAttributeUnsigned(yName, 0, container.height);

        unsigned width = getAttributeUnsigned(widthName, 1, container.width - x);
        unsigned height = getAttributeUnsigned(heightName, 1, container.height - y);

        return urect(x, y, width, height);
    }

    inline ms8point getAttributeMs8point(const std::string& xName = "x", const std::string& yName = "y") const
    {
        int_ms8_t x = getAttributeIntMs8(xName);
        int_ms8_t y = getAttributeIntMs8(yName);

        return ms8point(x, y);
    }

    inline ms8rect getAttributeMs8rect(const std::string& xName = "x", const std::string yName = "y", const std::string& widthName = "width", const std::string& heightName = "height") const
    {
        int_ms8_t x = getAttributeIntMs8(xName);
        int_ms8_t y = getAttributeIntMs8(yName);
        unsigned width = getAttributeUint8(widthName);
        unsigned height = getAttributeUint8(heightName);

        return ms8rect(x, y, width, height);
    }

    std::string generateErrorString(const char* msg) const;
    std::string generateErrorString(const std::string& aName, const char* msg) const;

    std::string name;
    std::unordered_map<std::string, std::string> attributes;
    const XmlReader* xml;
    const unsigned lineNo;
};
}
}
