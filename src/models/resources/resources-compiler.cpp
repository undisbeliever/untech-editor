/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources.h"
#include "version.h"
#include "models/common/file.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/metatiles/metatiles-serializer.h"
#include <cassert>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace UnTech {
namespace Resources {

class ResourceWriter {
private:
    std::stringstream _incData;
    std::vector<std::vector<uint8_t>> _blocks;
    std::string _binaryFilename;
    unsigned _maxBlockSize;
    bool _finalized;

public:
    ResourceWriter(const std::string& binaryFilename, unsigned blockSize, unsigned blockCount)
        : _incData()
        , _blocks(blockCount)
        , _binaryFilename(binaryFilename)
        , _maxBlockSize(blockSize)
        , _finalized(false)
    {
        for (auto& block : _blocks) {
            block.reserve(blockSize);
        }

        _incData << "rodata(RES_Lists)\n"
                 << "constant __resc__.EDITOR_VERSION = " << UNTECH_VERSION_INT << "\n\n";
    }

    void writeConstant(const char* name, long value)
    {
        _incData << "constant " << name << " = " << value << '\n';
    }

    void startList(const char* exportListName)
    {
        _incData << '\n'
                 << exportListName << ":\n";
    }

    void addDataToList(const std::vector<uint8_t>& data)
    {
        unsigned blockId, offset;
        std::tie(blockId, offset) = storeDataBlock(data);

        _incData << "  dl __resc__.Data" << blockId << " + " << offset << '\n';
    }

    template <class T>
    void writeNamesList(const std::vector<T>& inputList, const char* exportListName)
    {
        _incData << "\nnamespace " << exportListName << " {\n"
                 << "  constant count = " << inputList.size() << "\n\n";

        for (unsigned id = 0; id < inputList.size(); id++) {
            _incData << "  constant " << inputList.at(id).name << " = " << id << '\n';
        }

        _incData << "}\n";
    }

    void finalize()
    {
        assert(_finalized == false);

        _incData << "\nnamespace __resc__ {\n";

        unsigned offset = 0;
        for (unsigned blockId = 0; blockId < _blocks.size(); blockId++) {
            unsigned bSize = _blocks.at(blockId).size();

            if (bSize > 0) {
                assert(bSize <= _maxBlockSize);

                _incData << "rodata(RES_Block" << blockId << ")\n"
                         << "  insert Data" << blockId << ", \"" << _binaryFilename << "\", "
                         << offset << ", " << bSize << '\n';

                offset += bSize;
            }
        }

        _incData << "}\n";

        _finalized = true;
    }

    std::unique_ptr<ResourcesOutput> getOutput() const
    {
        assert(_finalized);

        auto ret = std::make_unique<ResourcesOutput>();

        ret->incData = _incData.str();

        ret->binaryData.reserve(_maxBlockSize * _blocks.size());
        for (auto& block : _blocks) {
            if (block.size() > 0) {
                ret->binaryData.insert(ret->binaryData.end(), block.begin(), block.end());
            }
        }

        return ret;
    }

private:
    std::pair<unsigned, unsigned> storeDataBlock(const std::vector<uint8_t>& data)
    {
        assert(_finalized == false);

        for (unsigned blockId = 0; blockId < _blocks.size(); blockId++) {
            auto& block = _blocks.at(blockId);

            unsigned spaceLeft = _maxBlockSize - block.size();
            if (spaceLeft >= data.size()) {
                unsigned offset = block.size();
                block.insert(block.end(), data.begin(), data.end());

                return { blockId, offset };
            }
        }

        throw std::runtime_error("Unable to store " + std::to_string(data.size())
                                 + " bytes of data, increase block size/count.");
    }
};

template <>
void ResourceWriter::writeNamesList(const std::vector<std::string>& nameList, const char* exportListName)
{
    _incData << "\nnamespace " << exportListName << " {\n"
             << "  constant count = " << nameList.size() << "\n\n";

    for (unsigned id = 0; id < nameList.size(); id++) {
        _incData << "  constant " << nameList.at(id) << " = " << id << '\n';
    }

    _incData << "}\n";
}

template <class T>
static std::string itemNameString(const T& item)
{
    return item.name;
}
template <>
std::string itemNameString(const std::string& item)
{
    return File::splitFilename(item).second;
}

std::unique_ptr<ResourcesOutput>
compileResources(const ResourcesFile& input, const std::string& binaryFilename)
{
    input.validate();

    ResourceWriter writer(binaryFilename, input.blockSize, input.blockCount);

    auto compileList = [&](const auto& inputList,
                           const char* exportListName, const char* typeName,
                           auto compile_fn) {

        writer.startList(exportListName);

        for (const auto& item : inputList) {
            try {
                std::vector<uint8_t> data = compile_fn(item);
                writer.addDataToList(data);
            }
            catch (const std::exception& ex) {
                throw std::runtime_error(std::string("Unable to compile ")
                                         + typeName + ' ' + itemNameString(item) + ": " + ex.what());
            }
        }
    };

    std::vector<std::string> metaTileTilemapNames;

    writer.writeConstant("Resources.PALETTE_FORMAT_VERSION", PaletteData::PALETTE_FORMAT_VERSION);
    writer.writeConstant("Resources.ANIMATED_TILESET_FORMAT_VERSION", AnimatedTilesetData::ANIMATED_TILESET_FORMAT_VERSION);
    writer.writeConstant("MetaTiles.TILESET_FORMAT_VERSION", MetaTiles::MetaTileTilesetData::TILESET_FORMAT_VERSION);

    compileList(input.palettes, "Resources.PaletteList", "palette",
                [](const PaletteInput& p) { return convertPalette(p).exportPalette(); });

    compileList(input.metaTileTilesetFilenames, "MetaTiles.TilesetList", "metaTile tileset",
                [&](const std::string& filename) {
                    const auto mti = MetaTiles::loadMetaTileTilesetInput(filename);
                    const auto mtd = MetaTiles::convertTileset(*mti, input);

                    metaTileTilemapNames.emplace_back(mti->name);

                    return mtd.exportMetaTileTileset(input.metaTileEngineSettings);
                });

    writer.writeNamesList(input.palettes, "Resources.PaletteList");
    writer.writeNamesList(metaTileTilemapNames, "MetaTiles.TilesetList");

    writer.finalize();

    return writer.getOutput();
}
}
}
