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
#include "models/common/stringbuilder.h"
#include <cassert>
#include <filesystem>
#include <sstream>
#include <vector>

namespace UnTech::Project {

template <typename T>
class DataStore;

class RomDataWriter {
public:
    struct Constant {
        const std::string name;

        // Same type that bass uses for constants
        const int64_t value;
    };

private:
    struct DataItem {
        const std::string name;
        const unsigned address;

        DataItem(const std::string& n, unsigned a)
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
    const std::string _blockName;
    const std::string _blockRodata;

private:
    [[noreturn]] void throwOutOfRomSpaceException(size_t size)
    {
        throw runtime_error("Unable to store ", size, " bytes of data, please add more banks to the Memory Map.");
    }

public:
    RomDataWriter(const MemoryMapSettings& memoryMap,
                  const std::string& blockName,
                  const std::string& blockRodata,
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
            throw runtime_error("Cannot store data in Rom Bank: incorrect address");
        }
        if (data.size() > _bankSize) {
            throw runtime_error("Cannot store data in Rom Bank: data is too large");
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

    void addNamedData(const std::string& name, const std::vector<uint8_t>& data)
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
    void addNotNullNamedData(const std::string& name, const std::vector<uint8_t>& data)
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

    void addNamedDataWithCount(const std::string& name, const std::vector<uint8_t>& data, int count)
    {
        addNamedData(name, data);
        _nameDataCounts.emplace_back(Constant{ name + ".count", count });
    }

    // Adds all data in the data store and creates a long address table pointing to the data in the store
    template <class T>
    void addDataStore(const std::string& longAddressTableName, const DataStore<T>& dataStore)
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
        _nameDataCounts.emplace_back(Constant{ longAddressTableName + ".count", unsigned(dataStore.size()) });
    }

    void writeIncData(std::stringstream& incData, const std::filesystem::path& relativeBinFilename) const
    {
        for (const Constant& c : _constants) {
            incData << "constant " << c.name << " = " << c.value << '\n';
        }

        incData << "\nnamespace " << _blockName << " {\n";
        unsigned offset = 0;
        for (auto [bankId, bank] : const_enumerate(_romBanks)) {
            const auto bSize = bank.data().size();
            if (bSize > 0U) {
                assert(bSize <= _bankSize);

                // std::filesystem::path operator<< will automatically add quotes to relativeBinFilename
                incData << "rodata(" << _blockRodata << bankId << ")\n"
                        << "assert(pc() == 0x" << std::hex << _memoryMap.bankAddress(bankId) << ")\n"
                        << std::dec
                        << "  insert Data" << bankId << ", " << relativeBinFilename << ", " << offset << ", " << bSize << '\n';

                offset += bSize;
            }
        }
        incData << "}\n";

        incData << std::dec;

        for (auto& nc : _nameDataCounts) {
            incData << "\nconstant " << nc.name << " = " << nc.value;
        }
        if (!_nameDataCounts.empty()) {
            incData << "\n\n";
        }

        incData << std::hex;

        for (const auto& nd : _namedData) {
            incData << "\nconstant " << nd.name << " = 0x" << nd.address;
        }
        incData << "\n\n";

        incData << std::dec;
    }

    std::vector<uint8_t> writeBinaryData() const
    {
        std::vector<uint8_t> binData;
        binData.reserve(_bankSize * _romBanks.size());
        for (const RomBankData& bank : _romBanks) {
            if (bank.valid() == false) {
                throw logic_error("RomBankData is too large");
            }
            if (!bank.empty()) {
                binData.insert(binData.end(), bank.data().begin(), bank.data().end());
            }
        }
        return binData;
    }
};

}
