/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/exceptions.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cstdint>
#include <string>
#include <vector>

// uses std::u8string instead of idstring as
// the assembly data labels can contain dots (.)

namespace UnTech::MetaSprite::Compiler {

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

private:
    const std::u8string _label;
    const std::u8string _dataLabel;
    std::vector<uint32_t> _offsets;

public:
    RomAddrTable(const std::u8string& label, const std::u8string& dataLabel)
        : _label(label)
        , _dataLabel(dataLabel)
        , _offsets()
    {
    }

    RomAddrTable(const RomAddrTable&) = delete;

    inline const std::u8string& label() const { return _label; }

    // Searches for a duplicate table before inserting.
    // a value > 0xFFFF in the table is a NULL
    unsigned getOrInsertTable(const std::vector<uint32_t>& table)
    {
        if (table.size() == 0) {
            throw invalid_argument(u8"Cannot add an empty table");
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
};

class DataBlock {

private:
    std::vector<uint8_t> _data;
    unsigned _pos;

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
};

class RomBinData {

private:
    const std::u8string _label;
    std::vector<uint8_t> _data;
    bool _nullableType;

public:
    RomBinData(const std::u8string& label, bool nullableType = false)
        : _label(label)
        , _data()
        , _nullableType(nullableType)
    {
    }
    RomBinData(const RomBinData&) = delete;

    const std::u8string& label() const { return _label; }
    bool nullableType() const { return _nullableType; }
    const std::vector<uint8_t>& data() const { return _data; }

    // Does not check for duplicates
    void addData_NoIndex(const std::vector<uint8_t>& sData)
    {
        _data.insert(_data.end(), sData.cbegin(), sData.cend());
    }

    uint32_t addData_Index(const std::vector<uint8_t>& sData)
    {
        if (sData.size() == 0) {
            throw invalid_argument(u8"Cannot add empty data");
        }

        auto it = std::search(_data.begin(), _data.end(),
                              sData.begin(), sData.end());

        if (it != _data.end()) {
            return std::distance(_data.begin(), it);
        }
        else {
            uint32_t oldSize = _data.size();

            _data.insert(_data.end(), sData.cbegin(), sData.cend());

            return oldSize;
        }
    }

    template <size_t N>
    uint32_t addData_Index(const std::array<uint8_t, N>& sData)
    {
        auto it = std::search(_data.begin(), _data.end(),
                              sData.begin(), sData.end());

        if (it != _data.end()) {
            return std::distance(_data.begin(), it);
        }
        else {
            uint32_t oldSize = _data.size();

            _data.insert(_data.end(), sData.cbegin(), sData.cend());

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
};

}
