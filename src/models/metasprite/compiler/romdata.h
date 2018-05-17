/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/hash.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// uses std::string instead of idstring as
// the assembly data labels can contain dots (.)

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

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

    const std::string* label;
    uint32_t offset;
};

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

    void addAddr(const RomOffsetPtr& data)
    {
        writeType(RomIncItem::ADDR);

        if (data.label != nullptr) {
            _stream << *data.label << " + " << data.offset;
        }
        else {
            _stream << data.offset;
        }

        _size += 2;
    }

    void addIndex(const RomOffsetPtr& data)
    {
        writeType(RomIncItem::ADDR);
        _stream << data.offset;
        _size += 2;
    }

    void addTilePtr(const RomOffsetPtr& data)
    {
        writeType(RomIncItem::ADDR);
        _stream << "(" << *data.label << " + " << data.offset << ") >> 7";
        _size += 2;
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
                _stream << "\n\tdb\t";
                break;

            case WORD:
                _stream << "\n\tdw\t";
                break;

            case DWORD:
                _stream << "\n\tdd\t";
                break;

            case ADDR:
                _stream << "\n\tdw\t";
                break;

            case FARADDR:
                _stream << "\n\tdl\t";
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
    RomIncData(const std::string& label, const std::string& segmentName,
               bool nullableType = false)
        : _label(label)
        , _segmentName(segmentName)
        , _size(0)
        , _map()
        , _stream()
        , _nullableType(nullableType)
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
    bool _nullableType;
};

class RomAddrTable {
public:
    static const unsigned ADDR_PER_LINE = 8;

public:
    RomAddrTable(const std::string& label, const std::string& segmentName,
                 const std::string& dataLabel, bool nullableType = false)
        : _label(label)
        , _segmentName(segmentName)
        , _dataLabel(dataLabel)
        , _offsets()
        , _nullableType(nullableType)
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
    bool _nullableType;
};

class RomBinData {
public:
    const unsigned BYTES_PER_LINE = 16;

public:
    RomBinData(const std::string& label, const std::string& segmentName,
               bool nullableType = false)
        : _label(label)
        , _segmentName(segmentName)
        , _size(0)
        , _map()
        , _data()
        , _nullableType(nullableType)
    {
    }
    RomBinData(const RomBinData&) = delete;

    const std::string& label() const { return _label; }

    void writeToIncFile(std::ostream& out) const;

    RomOffsetPtr addData(const std::vector<uint8_t>& sData)
    {
        if (sData.size() == 0) {
            return RomOffsetPtr();
        }

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
    bool _nullableType;
};
}
}
}
