/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/hash.h"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// uses std::string instead of idstring as
// the assembly data labels can contain dots (.)

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct IndexPlusOne {
    uint16_t index;

    explicit IndexPlusOne(unsigned i = 0)
        : index(i)
    {
        assert(i <= UINT16_MAX);
    }
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
    unsigned getOrInsertTable(const std::vector<uint32_t>& table)
    {
        if (table.size() == 0) {
            throw std::invalid_argument("Cannot add an empty table");
        }

        auto it = std::search(_offsets.begin(), _offsets.end(),
                              table.begin(), table.end());

        if (it == _offsets.end()) {
            // not found, create it
            it = _offsets.insert(_offsets.end(), table.begin(), table.end());
        }

        return std::distance(_offsets.begin(), it) * 2;
    }

    // a value > 0xFFFF is NULL.
    unsigned addOffset(uint32_t offset)
    {
        unsigned ret = _offsets.size() * 2;

        _offsets.push_back(offset);

        return ret;
    }

    unsigned addNull() { return addOffset(~0U); }

private:
    const std::string _label;
    const std::string _segmentName;
    const std::string _dataLabel;
    std::vector<uint32_t> _offsets;
    bool _nullableType;
};

class DataBlock {
public:
    DataBlock(size_t size)
        : _data(size)
        , _pos(0)
    {
    }

    void addByte(uint8_t data)
    {
        _data.at(_pos) = data;
        _pos += 1;
    }

    void addWord(uint16_t data)
    {
        _data.at(_pos) = data & 0xff;
        _data.at(_pos + 1) = (data >> 8) & 0xff;
        _pos += 2;
    }

    void addWord(IndexPlusOne data)
    {
        addWord(data.index);
    }

    bool atEnd() const { return _pos == _data.size(); }

    const std::vector<uint8_t>& data() const { return _data; }

private:
    std::vector<uint8_t> _data;
    unsigned _pos;
};

class RomBinData {
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

    // Does not check for duplicates
    void addData_NoIndex(const std::vector<uint8_t>& sData)
    {
        uint32_t oldSize = _size;

        _data.insert(_data.end(), sData.cbegin(), sData.cend());
        _size += sData.size();

        _map.emplace(sData, oldSize);
    }

    uint32_t addData_Index(const std::vector<uint8_t>& sData)
    {
        if (sData.size() == 0) {
            throw std::invalid_argument("Cannot add empty data");
        }

        const auto it = _map.find(sData);
        if (it != _map.end()) {
            return it->second;
        }
        else {
            uint32_t oldSize = _size;

            _data.insert(_data.end(), sData.cbegin(), sData.cend());
            _size += sData.size();

            _map.emplace(sData, oldSize);

            return oldSize;
        }
    }

    IndexPlusOne addData_IndexPlusOne(const std::vector<uint8_t>& sData)
    {
        if (sData.size() == 0) {
            return IndexPlusOne{ 0 };
        }

        return IndexPlusOne{ addData_Index(sData) + 1U };
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
