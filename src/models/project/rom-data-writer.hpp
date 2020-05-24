/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "memorymap.h"
#include "rom-bank-data.h"
#include "models/common/stringbuilder.h"
#include <cassert>
#include <filesystem>
#include <sstream>
#include <vector>

namespace UnTech {
namespace Project {

class RomDataWriter {
public:
    struct Constant {
        const std::string name;
        const long value;
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
    const std::vector<std::string>& _typeNames;
    std::vector<std::vector<DataItem>> _dataItems; // [type][id]
    std::vector<RomBankData> _romBanks;
    std::vector<DataItem> _namedData;
    std::vector<Constant> _nameDataCounts;
    const std::string _blockName;
    const std::string _listRodata;
    const std::string _blockRodata;

    void throwOutOfRomSpaceException(size_t size)
    {
        throw std::runtime_error(stringBuilder("Unable to store ", size, " bytes of data, please add more banks to the Memory Map."));
    }

public:
    RomDataWriter(const MemoryMapSettings& memoryMap,
                  const std::string& blockName,
                  const std::string& listRodata,
                  const std::string& blockRodata,
                  const std::vector<Constant>& constants,
                  const std::vector<std::string>& typeNames)
        : _memoryMap(memoryMap)
        , _bankSize(memoryMap.bankSize())
        , _constants(constants)
        , _typeNames(typeNames)
        , _dataItems(_typeNames.size())
        , _romBanks()
        , _namedData()
        , _blockName(blockName)
        , _listRodata(listRodata)
        , _blockRodata(blockRodata)
    {
        if (_typeNames.size() == 0) {
            throw std::invalid_argument("Expected at least one type name");
        }

        _romBanks.reserve(memoryMap.nBanks);
        for (unsigned i = 0; i < memoryMap.nBanks; i++) {
            _romBanks.emplace_back(memoryMap.bankAddress(i), memoryMap.bankSize());
        }
    }

    void addBankData(unsigned bankId, const unsigned addr, const std::vector<uint8_t> data)
    {
        auto& bank = _romBanks.at(bankId);
        if (!bank.empty() || bank.currentAddress() != addr) {
            throw std::runtime_error("Cannot store data in Rom Bank: incorrect address");
        }
        if (data.size() > _bankSize) {
            throw std::overflow_error("Cannot store data in Rom Bank: data is too large");
        }

        _romBanks.at(bankId).addData(data);
    }

    void addData(unsigned typeId, const std::string& name, const std::vector<uint8_t>& data)
    {
        for (auto& bank : _romBanks) {
            if (auto addr = bank.tryToAddData(data)) {
                _dataItems.at(typeId).emplace_back(name, *addr);

                return;
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

    void writeIncData(std::stringstream& incData, const std::filesystem::path& relativeBinFilename) const
    {
        for (const Constant& c : _constants) {
            incData << "constant " << c.name << " = " << c.value << '\n';
        }

        for (unsigned typeId = 0; typeId < _typeNames.size(); typeId++) {
            const std::string& typeName = _typeNames.at(typeId);
            const std::vector<DataItem>& items = _dataItems.at(typeId);

            incData << "\nnamespace " << typeName << " {\n"
                    << "  constant count = " << items.size() << "\n\n";

            for (unsigned id = 0; id < items.size(); id++) {
                incData << "  constant " << items.at(id).name << " = " << id << '\n';
            }
            incData << "}\n";
        }

        incData << "\nnamespace " << _blockName << " {\n";
        unsigned offset = 0;
        for (unsigned bankId = 0; bankId < _romBanks.size(); bankId++) {
            unsigned bSize = _romBanks.at(bankId).data().size();

            if (bSize > 0) {
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

        incData << "\nrodata(" << _listRodata << ")";
        for (unsigned typeId = 0; typeId < _typeNames.size(); typeId++) {
            incData << "\n\n"
                    << _typeNames.at(typeId) << ":";

            for (const DataItem& di : _dataItems.at(typeId)) {
                incData << "\n  dl 0x" << di.address;
            }
        }
        incData << "\n";

        incData << std::dec;
    }

    std::vector<uint8_t> writeBinaryData() const
    {
        std::vector<uint8_t> binData;
        binData.reserve(_bankSize * _romBanks.size());
        for (const RomBankData& bank : _romBanks) {
            if (bank.valid() == false) {
                throw std::logic_error("RomBankData is too large");
            }
            if (!bank.empty()) {
                binData.insert(binData.end(), bank.data().begin(), bank.data().end());
            }
        }
        return binData;
    }
};
}
}
