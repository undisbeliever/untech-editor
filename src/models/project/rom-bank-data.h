/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "memorymap.h"
#include <climits>
#include <cstdint>
#include <optional>
#include <vector>

namespace UnTech::Project {

class RomBankData {
private:
    unsigned _startingAddress;
    size_t _bankSize;
    std::vector<uint8_t> _data;

public:
    RomBankData(const unsigned address, const unsigned size)
        : _startingAddress(address)
        , _bankSize(size)
        , _data()
    {
        assert(_bankSize < INT_MAX);

        _data.reserve(_bankSize);
    }

    [[nodiscard]] bool valid() const { return _data.size() <= _bankSize; }

    [[nodiscard]] const std::vector<uint8_t>& data() const { return _data; }
    [[nodiscard]] bool empty() const { return _data.empty(); }

    [[nodiscard]] unsigned startingAddress() const { return _startingAddress; }
    [[nodiscard]] unsigned currentAddress() const { return _data.size() + _startingAddress; }

    [[nodiscard]] bool canAddData(const size_t s) const
    {
        return _data.size() + s <= _bankSize;
    }
    [[nodiscard]] bool canAddData(const std::vector<uint8_t>& d) const
    {
        return canAddData(d.size());
    }

    unsigned addData(const std::vector<uint8_t>& d)
    {
        const unsigned addr = currentAddress();
        _data.insert(_data.end(), d.begin(), d.end());
        return addr;
    }

    std::optional<unsigned> tryToAddData(const std::vector<uint8_t>& d)
    {
        if (canAddData(d) == false) {
            return std::nullopt;
        }
        return addData(d);
    }

    // data will never be stored at word address 0
    std::optional<unsigned> tryToAddNotNullData(const std::vector<uint8_t>& d)
    {
        if ((currentAddress() & 0xffff) == 0) {
            return std::nullopt;
        }
        return tryToAddData(d);
    }

    std::optional<unsigned> tryToAddDataWidthPaddingByte(const std::vector<uint8_t>& d)
    {
        if (canAddData(d.size() + 1) == false) {
            return std::nullopt;
        }

        _data.push_back(0); // padding byte
        return addData(d);
    }
};

}
