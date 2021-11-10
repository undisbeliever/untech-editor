/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "memorymap.h"
#include "rom-bank-data.h"
#include "models/common/exceptions.h"
#include "models/common/iterators.h"
#include "models/common/string.h"
#include "models/common/stringbuilder.h"
#include "models/common/stringstream.h"
#include <cassert>
#include <filesystem>

#include <vector>

namespace UnTech::Project {

template <typename T>
class DataStore;

class RomDataWriter {
public:
    struct Constant {
        const std::u8string name;

        // Same type that bass uses for constants
        const int64_t value;
    };

private:
    struct DataItem {
        const std::u8string name;
        const unsigned address;

        DataItem(const std::u8string& n, unsigned a)
            : name(n)
            , address(a)
        {
        }
    };

private:
    const MemoryMapSettings _memoryMap;
    const unsigned _bankSize;

    const std::vector<Constant>& _constants;
    std::vector<RomBankData> _romBanks;
    std::vector<DataItem> _namedData;
    std::vector<Constant> _nameDataCounts;
    const std::u8string _blockName;
    const std::u8string _blockRodata;

private:
    [[noreturn]] void throwOutOfRomSpaceException(size_t size)
    {
        throw runtime_error(u8"Unable to store ", size, u8" bytes of data, please add more banks to the Memory Map.");
    }

public:
    RomDataWriter(const MemoryMapSettings& memoryMap,
                  const std::u8string& blockName,
                  const std::u8string& blockRodata,
                  const std::vector<Constant>& constants)
        : _memoryMap(memoryMap)
        , _bankSize(memoryMap.bankSize())
        , _constants(constants)
        , _romBanks()
        , _namedData()
        , _blockName(blockName)
        , _blockRodata(blockRodata)
    {
        _romBanks.reserve(memoryMap.nBanks);

        for (const auto i : range(memoryMap.nBanks)) {
            _romBanks.emplace_back(memoryMap.bankAddress(i), memoryMap.bankSize());
        }
    }

    void addBankData(unsigned bankId, const unsigned addr, const std::vector<uint8_t> data)
    {
        auto& bank = _romBanks.at(bankId);
        if (!bank.empty() || bank.currentAddress() != addr) {
            throw runtime_error(u8"Cannot store data in Rom Bank: incorrect address");
        }
        if (data.size() > _bankSize) {
            throw runtime_error(u8"Cannot store data in Rom Bank: data is too large");
        }

        _romBanks.at(bankId).addData(data);
    }

    unsigned addData(const std::vector<uint8_t>& data)
    {
        for (auto& bank : _romBanks) {
            if (auto addr = bank.tryToAddData(data)) {
                return *addr;
            }
        }

        throwOutOfRomSpaceException(data.size());
    }

    void addNamedData(const std::u8string& name, const std::vector<uint8_t>& data)
    {
        for (auto& bank : _romBanks) {
            if (auto addr = bank.tryToAddData(data)) {
                _namedData.emplace_back(name, *addr);
                return;
            }
        }

        throwOutOfRomSpaceException(data.size());
    }

    // data will never be stored at word address 0
    void addNotNullNamedData(const std::u8string& name, const std::vector<uint8_t>& data)
    {
        for (auto& bank : _romBanks) {
            if (auto addr = bank.tryToAddNotNullData(data)) {
                _namedData.emplace_back(name, *addr);
                return;
            }
        }
        for (auto& bank : _romBanks) {
            if (auto addr = bank.tryToAddDataWidthPaddingByte(data)) {
                _namedData.emplace_back(name, *addr);
                return;
            }
        }

        throwOutOfRomSpaceException(data.size());
    }

    void addNamedDataWithCount(const std::u8string& name, const std::vector<uint8_t>& data, int count)
    {
        addNamedData(name, data);
        _nameDataCounts.emplace_back(Constant{ name + u8".count", count });
    }

    // Adds all data in the data store and creates a long address table pointing to the data in the store
    template <class T>
    void addDataStore(const std::u8string& longAddressTableName, const DataStore<T>& dataStore)
    {
        std::vector<uint8_t> longAddressTable(dataStore.size() * 3);
        auto it = longAddressTable.begin();

        for (const auto i : range(dataStore.size())) {
            auto data = dataStore.at(i);
            assert(data);
            const auto addr = addData(data->exportSnesData());

            *it++ = addr & 0xff;
            *it++ = (addr >> 8) & 0xff;
            *it++ = (addr >> 16) & 0xff;
        }
        assert(it == longAddressTable.end());

        assert(dataStore.size() < INT_MAX);

        addNamedData(longAddressTableName, longAddressTable);
        _nameDataCounts.emplace_back(Constant{ longAddressTableName + u8".count", unsigned(dataStore.size()) });
    }

    void writeIncData(StringStream& incData, const std::filesystem::path& relativeBinFilename) const
    {
        const std::u8string rbfString = relativeBinFilename.u8string();

        for (const Constant& c : _constants) {
            incData.write(u8"constant ", c.name, u8" = ", c.value, u8"\n");
        }

        incData.write(u8"\nnamespace ", _blockName, u8" {\n");
        unsigned offset = 0;
        for (auto [bankId, bank] : const_enumerate(_romBanks)) {
            const auto bSize = bank.data().size();
            if (bSize > 0U) {
                assert(bSize <= _bankSize);

                incData.write(u8"rodata(", _blockRodata, bankId, u8")\n",
                              u8"assert(pc() == 0x", hex_6(_memoryMap.bankAddress(bankId)), u8")\n",
                              u8"  insert Data", bankId, u8", \"", rbfString, u8"\", ", offset, u8", ", bSize, u8"\n");
                offset += bSize;
            }
        }
        incData.write(u8"}\n");

        for (auto& nc : _nameDataCounts) {
            incData.write(u8"\nconstant ", nc.name, u8" = ", nc.value);
        }
        if (!_nameDataCounts.empty()) {
            incData.write(u8"\n\n");
        }

        for (const auto& nd : _namedData) {
            incData.write(u8"\nconstant ", nd.name, u8" = 0x", hex_6(nd.address));
        }
        incData.write(u8"\n\n");
    }

    std::vector<uint8_t> writeBinaryData() const
    {
        std::vector<uint8_t> binData;
        binData.reserve(_bankSize * _romBanks.size());
        for (const RomBankData& bank : _romBanks) {
            if (bank.valid() == false) {
                throw logic_error(u8"RomBankData is too large");
            }
            if (!bank.empty()) {
                binData.insert(binData.end(), bank.data().begin(), bank.data().end());
            }
        }
        return binData;
    }
};

}
