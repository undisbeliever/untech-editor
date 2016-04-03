#ifndef _UNTECH_MODELS_COMMON_XML_XMLTAG_H_
#define _UNTECH_MODELS_COMMON_XML_XMLTAG_H_

#include "../namedlist.h"
#include <climits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace UnTech {
namespace Xml {

struct XmlTag {
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
            throw buildError(aName, "Missing attribute");
        }
    }

    inline std::string getAttributeId(const std::string& aName) const
    {
        std::string id = getAttribute(aName);

        if (isNameValid(id)) {
            return id;
        }
        else {
            throw buildError(aName, "Invalid id");
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
            throw buildError(aName, "Not a number");
        }
        return v.first;
    }

    inline int getAttributeInteger(const std::string& aName, int min, int max) const
    {
        int i = getAttributeInteger(aName);

        if (i < min) {
            throw buildError(aName, "Number too small");
        }
        if (i > max) {
            throw buildError(aName, "Number too small");
        }
        return i;
    }

    inline unsigned getAttributeUnsigned(const std::string& aName, unsigned min = 0, unsigned max = UINT_MAX) const
    {
        auto v = String::toLong(getAttribute(aName));

        if (!v.second) {
            throw buildError(aName, "Not a number");
        }
        if (v.first < 0) {
            throw buildError(aName, "Only positive numbers allowed");
        }
        if ((unsigned long)v.first < min) {
            throw buildError(aName, "Number too small");
        }
        if ((unsigned long)v.first > max) {
            throw buildError(aName, "Number too large");
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
                throw buildError(aName, "Expected true or false");
            }
        }

        return def;
    }

    inline unsigned getAttributeUnsignedHex(const std::string& aName) const
    {
        auto v = String::hexToUnsigned(getAttribute(aName));

        if (!v.second) {
            throw buildError(aName, "Not a hexadecimal number");
        }
        return v.first;
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
            throw std::logic_error("Container too small");
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

    std::runtime_error buildError(const char* msg) const
    {
        std::stringstream stream;

        auto fp = xml->filepart();
        if (fp.empty()) {
            fp = "XML";
        }

        stream << fp << ":" << xml->lineNo() << " <" << name << ">: " << msg;
        return std::runtime_error(stream.str());
    }

    std::runtime_error buildError(const std::string& aName, const char* msg) const
    {
        std::stringstream stream;

        auto fp = xml->filepart();
        if (fp.empty()) {
            fp = "XML";
        }

        stream << fp << ":" << lineNo << " <" << name << " " << aName << ">: " << msg;
        return std::runtime_error(stream.str());
    }

    std::runtime_error buildUnknownTagError() const
    {
        std::stringstream stream;

        auto fp = xml->filepart();
        if (fp.empty()) {
            fp = "XML";
        }

        stream << fp << ":" << xml->lineNo() << ": Unknown tag '" << name << "'";
        return std::runtime_error(stream.str());
    }

    std::string name;
    std::unordered_map<std::string, std::string> attributes;
    const XmlReader* xml;
    const unsigned lineNo;
};
}
}

#endif
