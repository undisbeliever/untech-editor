#ifndef _UNTECH_MODELS_METASPRITECOMPILER_ROMDATA_H
#define _UNTECH_MODELS_METASPRITECOMPILER_ROMDATA_H

#include "models/common/hash.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace UnTech {
namespace MetaSpriteCompiler {

// MEMORY: Must not exist when data class is deleted.
struct RomOffsetPtr {
    RomOffsetPtr(const std::string* label, uint32_t offset)
        : label(label)
        , offset(offset)
    {
    }

    RomOffsetPtr(uint32_t offset = 0)
        : label(nullptr)
        , offset(offset)
    {
    }

    RomOffsetPtr(const RomOffsetPtr&) = default;

    const std::string* label;
    uint32_t offset;
};
extern const RomOffsetPtr NULL_OFFSET;

class RomIncItem {
public:
    enum Type {
        NONE,
        BYTE,
        WORD,
        DWORD,
        ADDR,
        FARADDR
    };

public:
    RomIncItem()
        : _size(0)
        , _prevType(NONE)
        , _stream()
    {
    }
    RomIncItem(const RomIncItem&) = delete;

    inline uint32_t size() const { return _size; }
    inline std::string string() const { return _stream.str(); }

    void addField(Type type, unsigned value)
    {
        writeType(type);
        _stream << value;

        _size += sizeOfType(type);
    }

    void addField(Type type, const RomOffsetPtr& data)
    {
        assert(type == Type::ADDR || type == Type::FARADDR);

        writeType(type);

        if (data.label != nullptr) {
            _stream << *data.label << " + " << data.offset;
        }
        else {
            _stream << data.offset;
        }

        _size += sizeOfType(type);
    }

protected:
    unsigned sizeOfType(RomIncItem::Type type)
    {
        switch (type) {
        case BYTE:
            return 1;

        case WORD:
        case ADDR:
            return 2;

        case FARADDR:
            return 3;

        case DWORD:
            return 4;

        default:
            return 0;
        }
    }

    void writeType(Type type)
    {
        if (type == _prevType) {
            _stream << ", ";
        }
        else {
            switch (type) {
            case BYTE:
                _stream << "\n\t.byte ";
                break;

            case WORD:
                _stream << "\n\t.word ";
                break;

            case DWORD:
                _stream << "\n\t.dword ";
                break;

            case ADDR:
                _stream << "\n\t.addr ";
                break;

            case FARADDR:
                _stream << "\n\t.faraddr ";
                break;

            default:
                break;
            }
        }

        _prevType = type;
    }

private:
    uint32_t _size;
    Type _prevType;
    std::stringstream _stream;
};

class RomIncData {
public:
    RomIncData(const std::string& label, const std::string& segmentName)
        : _label(label)
        , _segmentName(segmentName)
        , _size(0)
        , _map()
        , _stream()
    {
    }

    RomIncData(const RomIncData&) = delete;

    inline const std::string& label() const { return _label; }
    inline uint32_t size() const { return _size; }

    void writeToIncFile(std::ostream& out) const;

    RomOffsetPtr addData(const RomIncItem& item)
    {
        if (item.size() == 0) {
            return RomOffsetPtr();
        }

        const std::string data = item.string();

        const auto it = _map.find(data);
        if (it != _map.end()) {
            return RomOffsetPtr(&_label, it->second);
        }
        else {
            uint32_t oldSize = _size;

            _stream << data;
            _size += item.size();

            _map.emplace(data, oldSize);

            return RomOffsetPtr(&_label, oldSize);
        }
    }

private:
    const std::string _label;
    const std::string _segmentName;
    uint32_t _size;
    std::unordered_map<std::string, uint32_t> _map;
    std::stringstream _stream;
};

class RomAddrTable {
public:
    static const unsigned ADDR_PER_LINE = 8;

public:
    RomAddrTable(const std::string& label, const std::string& segmentName, const std::string& dataLabel)
        : _label(label)
        , _segmentName(segmentName)
        , _dataLabel(dataLabel)
        , _offsets()
    {
    }

    RomAddrTable(const RomAddrTable&) = delete;

    inline const std::string& label() const { return _label; }

    void writeToIncFile(std::ostream& out) const;

    // Searches for a duplicate table before inserting.
    // a value > 0xFFFF in the table is a NULL
    RomOffsetPtr getOrInsertTable(const std::vector<uint32_t>& table)
    {
        if (table.size() == 0) {
            return RomOffsetPtr();
        }

        auto it = std::search(_offsets.begin(), _offsets.end(),
                              table.begin(), table.end());

        if (it == _offsets.end()) {
            // not found, create it
            it = _offsets.insert(_offsets.end(), table.begin(), table.end());
        }

        return RomOffsetPtr(&_label,
                            std::distance(_offsets.begin(), it) * 2);
    }

    // a value > 0xFFFF is NULL.
    RomOffsetPtr addOffset(uint32_t offset)
    {
        RomOffsetPtr ret(&_label, _offsets.size() * 2);

        _offsets.push_back(offset);

        return ret;
    }

    RomOffsetPtr addNull() { return addOffset(~0); }

private:
    const std::string _label;
    const std::string _segmentName;
    const std::string _dataLabel;
    std::vector<uint32_t> _offsets;
};

class RomBinData {
public:
    const unsigned BYTES_PER_LINE = 16;

public:
    RomBinData(const std::string& label, const std::string& segmentName)
        : _label(label)
        , _segmentName(segmentName)
        , _size(0)
        , _map()
        , _data()
    {
    }
    RomBinData(const RomBinData&) = delete;

    const std::string& label() const { return _label; }

    void writeToIncFile(std::ostream& out) const;

    RomOffsetPtr addData(const std::vector<uint8_t>& sData)
    {
        const auto it = _map.find(sData);
        if (it != _map.end()) {
            return RomOffsetPtr(&_label, it->second);
        }
        else {
            uint32_t oldSize = _size;

            _data.insert(_data.end(), sData.cbegin(), sData.cend());
            _size += sData.size();

            _map.emplace(sData, oldSize);

            return RomOffsetPtr(&_label, oldSize);
        }
    }

private:
    const std::string _label;
    const std::string _segmentName;
    uint32_t _size;
    std::unordered_map<std::vector<uint8_t>, uint32_t> _map;
    std::vector<uint8_t> _data;
};
}
}

#endif
