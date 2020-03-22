/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

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
        const int value;
    };

private:
    struct DataItem {
        const std::string name;
        const unsigned blockId;
        const unsigned offset;
    };

private:
    const std::vector<Constant>& _constants;
    const std::vector<std::string>& _typeNames;
    std::vector<std::vector<DataItem>> _dataItems; // [type][id]
    std::vector<std::vector<uint8_t>> _blocks;     // [blockId] => data
    std::vector<DataItem> _namedData;
    std::vector<Constant> _nameDataCounts;
    const std::string _blockName;
    const std::string _listRodata;
    const std::string _blockRodata;
    const unsigned _maxBlockSize;

public:
    RomDataWriter(unsigned blockSize, unsigned blockCount,
                  const std::string& blockName,
                  const std::string& listRodata,
                  const std::string& blockRodata,
                  const std::vector<Constant>& constants,
                  const std::vector<std::string>& typeNames)
        : _constants(constants)
        , _typeNames(typeNames)
        , _dataItems(_typeNames.size())
        , _blocks(blockCount)
        , _namedData()
        , _blockName(blockName)
        , _listRodata(listRodata)
        , _blockRodata(blockRodata)
        , _maxBlockSize(blockSize)
    {
        if (_typeNames.size() == 0) {
            throw std::invalid_argument("Expected at least one type name");
        }
        for (auto& block : _blocks) {
            block.reserve(blockSize);
        }
    }

    void addData(unsigned typeId, const std::string& name, const std::vector<uint8_t>& data)
    {
        for (unsigned blockId = 0; blockId < _blocks.size(); blockId++) {
            auto& block = _blocks.at(blockId);

            unsigned spaceLeft = _maxBlockSize - block.size();
            if (spaceLeft >= data.size()) {
                unsigned offset = block.size();
                block.insert(block.end(), data.begin(), data.end());

                _dataItems.at(typeId).push_back({ name, blockId, offset });

                return;
            }
        }

        throw std::runtime_error(stringBuilder("Unable to store ", data.size(), " bytes of data, increase block size/count."));
    }

    void addNamedData(const std::string& name, const std::vector<uint8_t>& data)
    {
        for (unsigned blockId = 0; blockId < _blocks.size(); blockId++) {
            auto& block = _blocks.at(blockId);

            unsigned spaceLeft = _maxBlockSize - block.size();
            if (spaceLeft >= data.size()) {
                unsigned offset = block.size();
                block.insert(block.end(), data.begin(), data.end());

                _namedData.push_back({
                    name,
                    blockId,
                    offset,
                });
                return;
            }
        }

        throw std::runtime_error(stringBuilder("Unable to store ", data.size(), " bytes of data, increase block size/count."));
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
        for (unsigned blockId = 0; blockId < _blocks.size(); blockId++) {
            unsigned bSize = _blocks.at(blockId).size();

            if (bSize > 0) {
                assert(bSize <= _maxBlockSize);

                // std::filesystem::path operator<< will automatically add quotes to relativeBinFilename
                incData << "rodata(" << _blockRodata << blockId << ")\n"
                        << "  insert Data" << blockId << ", " << relativeBinFilename << ", "
                        << offset << ", " << bSize << '\n';

                offset += bSize;
            }
        }
        incData << "}\n";

        for (const auto& nd : _namedData) {
            incData << "\nconstant " << nd.name << " = "
                    << _blockName << ".Data" << nd.blockId << " + " << nd.offset;
        }
        incData << "\n\n";

        for (auto& nc : _nameDataCounts) {
            incData << "\nconstant " << nc.name << " = " << nc.value;
        }
        if (!_nameDataCounts.empty()) {
            incData << "\n\n";
        }

        incData << "\nrodata(" << _listRodata << ")\n";
        for (unsigned typeId = 0; typeId < _typeNames.size(); typeId++) {
            incData << '\n'
                    << _typeNames.at(typeId) << ":\n";

            for (const DataItem& di : _dataItems.at(typeId)) {
                incData << "  dl " << _blockName << ".Data" << di.blockId << " + " << di.offset << '\n';
            }
        }
        incData << "\n";
    }

    std::vector<uint8_t> writeBinaryData() const
    {
        std::vector<uint8_t> binData;
        binData.reserve(_maxBlockSize * _blocks.size());
        for (const std::vector<uint8_t>& block : _blocks) {
            if (block.size() > 0) {
                binData.insert(binData.end(), block.begin(), block.end());
            }
        }
        return binData;
    }
};
}
}
