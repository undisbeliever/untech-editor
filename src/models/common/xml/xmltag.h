#pragma once

#include "../aabb.h"
#include "../file.h"
#include "../int_ms8_t.h"
#include "../ms8aabb.h"
#include "../namedlist.h"
#include "../optional.h"
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
            throw buildError(aName, "Not a number");
        }
        return v.value();
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

        if (!v) {
            throw buildError(aName, "Not a number");
        }
        if (v.value() < 0) {
            throw buildError(aName, "Only positive numbers allowed");
        }
        if ((unsigned long)v.value() < min) {
            throw buildError(aName, "Number too small");
        }
        if ((unsigned long)v.value() > max) {
            throw buildError(aName, "Number too large");
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
                throw buildError(aName, "Expected true or false");
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
        catch (const std::out_of_range& ex) {
            throw buildError(name, "Invalid");
        }
        catch (const std::exception& ex) {
            throw buildError(name, ex.what());
        }
    }

    inline unsigned getAttributeUnsignedHex(const std::string& aName) const
    {
        auto v = String::hexToUnsigned(getAttribute(aName));

        if (!v) {
            throw buildError(aName, "Not a hexadecimal number");
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
            throw std::logic_error("Container too small");
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

    std::runtime_error buildError(const std::string& message, const std::exception& ex) const
    {
        std::stringstream stream;

        auto fp = xml->filepart();
        if (fp.empty()) {
            fp = "XML";
        }

        stream << fp << ":" << lineNo << ' ' << message << "\n\t"
               << ex.what();
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
