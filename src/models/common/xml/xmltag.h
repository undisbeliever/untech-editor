#ifndef _UNTECH_MODELS_COMMON_XML_XMLTAG_H_
#define _UNTECH_MODELS_COMMON_XML_XMLTAG_H_

// ::TODO clean up exceptions, include tag name and line no::

#include <climits>
#include <memory>
#include <string>
#include <unordered_map>
#include "../namedlist.h"

namespace UnTech {
namespace Xml {

struct XmlTag {
    XmlTag(std::string tagName, unsigned lineNo)
        : name(tagName)
        , attributes()
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
            throw("Missing attribute");
        }
    }

    inline std::string getAttributeId(const std::string& aName) const
    {
        std::string id = getAttribute(aName);

        if (isNameListNameValid(id)) {
            return id;
        }
        else {
            throw("Invalid id");
        }
    }

    inline std::pair<std::string, bool> getOptionalAttribute(const std::string& aName) const
    {
        auto it = attributes.find(aName);
        if (it != attributes.end()) {
            return { it->second, true };
        }
        else {
            return { std::string(), false };
        }
    }

    inline int getAttributeInteger(const std::string& aName) const
    {
        auto v = String::toInt(getAttribute(aName));

        if (!v.second) {
            throw("Not a number");
        }
        return v.first;
    }

    inline int getAttributeInteger(const std::string& aName, int min, int max) const
    {
        int i = getAttributeInteger(aName);

        if (i < min) {
            throw("Number too small");
        }
        if (i > max) {
            throw("Number too small");
        }
        return i;
    }

    inline unsigned getAttributeUnsigned(const std::string& aName, unsigned min = 0, unsigned max = UINT_MAX) const
    {
        auto v = String::toLong(getAttribute(aName));

        if (!v.second) {
            throw("Not a number");
        }
        if (v.first < 0) {
            throw("Only positive numbers allowed");
        }
        if ((unsigned long)v.first < min) {
            throw("Number too small");
        }
        if ((unsigned long)v.first > max) {
            throw("Number too large");
        }
        return (unsigned)v.first;
    }

    inline int8_t getAttributeInt8(const std::string& aName) const
    {
        return (int8_t)getAttributeInteger(aName, INT8_MIN, INT8_MAX);
    }

    inline uint8_t getAttributeUint8(const std::string& aName) const
    {
        return (uint8_t)getAttributeUnsigned(aName, 0, UINT8_MAX);
    }

    inline bool getAttributeBoolean(const std::string& aName, bool def = false) const
    {
        auto v = getOptionalAttribute(aName);

        if (v.second) {
            if (v.first == "true") {
                return true;
            }
            else if (v.first == "false") {
                return false;
            }
            else {
                throw("Expected true or false");
            }
        }

        return def;
    }

    inline upoint getAttributeUpoint(const std::string& xName = "x", const std::string& yName = "y") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        return { x, y };
    }

    inline usize getAttributeUsize(const std::string& widthName = "width", const std::string& heightName = "height") const
    {
        unsigned width = getAttributeUnsigned(widthName);
        unsigned height = getAttributeUnsigned(heightName);

        return { width, height };
    }

    inline upoint getAttributeUpointInside(const urect& container, const std::string& xName = "x", const std::string& yName = "y") const
    {
        unsigned x = getAttributeUnsigned(xName, 0, container.width);
        unsigned y = getAttributeUnsigned(yName, 0, container.height);

        return { x, y };
    }

    inline upoint getAttributeUpointInside(const urect& container, unsigned squareSize, const std::string& xName = "x", const std::string& yName = "y") const
    {
        if (container.width < squareSize || container.height < squareSize) {
            throw("Container too small");
        }
        unsigned x = getAttributeUnsigned(xName, 0, container.width - squareSize);
        unsigned y = getAttributeUnsigned(yName, 0, container.height - squareSize);

        return { x, y };
    }

    inline urect getAttributeUrect(const std::string& xName = "x", const std::string yName = "y", const std::string& widthName = "width", const std::string& heightName = "height") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        unsigned width = getAttributeUnsigned(widthName, 1, UINT_MAX);
        unsigned height = getAttributeUnsigned(heightName, 1, UINT_MAX);

        return { x, y, width, height };
    }

    inline urect getAttributeUrect(const usize& maximumSize, const std::string& xName = "x", const std::string yName = "y", const std::string& widthName = "width", const std::string& heightName = "height") const
    {
        unsigned x = getAttributeUnsigned(xName);
        unsigned y = getAttributeUnsigned(yName);

        unsigned width = getAttributeUnsigned(widthName, 1, maximumSize.width);
        unsigned height = getAttributeUnsigned(heightName, 1, maximumSize.height);

        return { x, y, width, height };
    }

    inline urect getAttributeUrectInside(const urect& container, const std::string& xName = "x", const std::string& yName = "y", const std::string& widthName = "width", const std::string& heightName = "height") const
    {
        unsigned x = getAttributeUnsigned(xName, 0, container.width);
        unsigned y = getAttributeUnsigned(yName, 0, container.height);

        unsigned width = getAttributeUnsigned(widthName, 1, container.width - x);
        unsigned height = getAttributeUnsigned(heightName, 1, container.height - y);

        return { x, y, width, height };
    }

    std::string name;
    std::unordered_map<std::string, std::string> attributes;
    unsigned lineNo;
};
}
}

#endif
