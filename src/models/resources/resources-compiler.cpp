/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources.h"
#include "rom-data-writer.hpp"
#include "version.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/metatiles/metatiles-serializer.h"

namespace UnTech {
namespace Resources {

template <class T>
static std::string itemNameString(const T& item)
{
    return item.name;
}
template <class T>
static std::string itemNameString(const std::shared_ptr<T>& item)
{
    return item->name;
}
template <>
std::string itemNameString(const std::string& item)
{
    return File::splitFilename(item).second;
}

std::unique_ptr<ResourcesOutput>
compileResources(const ResourcesFile& input, const std::string& relativeBinFilename,
                 std::ostream& errorStream)
{
    const static std::vector<RomDataWriter::Constant> FORMAT_VERSIONS = {
        { "__resc__.EDITOR_VERSION", UNTECH_VERSION_INT },
        { "Resources.PALETTE_FORMAT_VERSION", PaletteData::PALETTE_FORMAT_VERSION },
        { "Resources.ANIMATED_TILESET_FORMAT_VERSION", AnimatedTilesetData::ANIMATED_TILESET_FORMAT_VERSION },
        { "MetaTiles.TILESET_FORMAT_VERSION", MetaTiles::MetaTileTilesetData::TILESET_FORMAT_VERSION }
    };
    enum TypeId : unsigned {
        PALETTE,
        METATILE_TILESET
    };
    const static std::vector<std::string> TYPE_NAMES = {
        "Resources.PaletteList",
        "MetaTiles.TilesetList"
    };

    bool valid = true;
    {
        ErrorList errorList;
        valid = input.validate(errorList);
        if (!valid) {
            errorStream << "Unable to compile resources:\n";
            errorList.printIndented(errorStream);
            return nullptr;
        }
    }

    auto compileList = [&](const auto& inputList, const char* typeName, auto compile_fn) {
        for (const auto& item : inputList) {
            ErrorList errorList;

            compile_fn(item, errorList);

            if (errorList.hasError()) {
                errorStream << "ERROR compiling " << typeName << " `" << itemNameString(item) << "`:\n";
                errorList.printIndented(errorStream);
                valid = false;
            }
        }
    };

    RomDataWriter writer(input.blockSize, input.blockCount,
                         "__resc__", "RES_Lists", "RES_Block",
                         FORMAT_VERSIONS, TYPE_NAMES);

    compileList(input.palettes, "Palette",
                [&](const std::shared_ptr<PaletteInput>& p, ErrorList& err) {
                    assert(p != nullptr);
                    const auto palData = convertPalette(*p, err);
                    if (palData) {
                        writer.addData(PALETTE, p->name, palData->exportPalette());
                    }
                });

    if (!valid) {
        // Prevents duplicate palette errors
        return nullptr;
    }

    compileList(input.metaTileTilesetFilenames, "MetaTile Tileset",
                [&](const std::string& filename, ErrorList& err) {
                    const auto mti = MetaTiles::loadMetaTileTilesetInput(filename, err);
                    if (!mti) {
                        return;
                    }
                    const auto mtd = MetaTiles::convertTileset(*mti, input, err);
                    if (mtd) {
                        const auto data = mtd->exportMetaTileTileset(input.metaTileEngineSettings);
                        writer.addData(METATILE_TILESET, mti->name, data);
                    }
                });

    if (!valid) {
        return nullptr;
    }

    auto ret = std::make_unique<ResourcesOutput>();
    ret->incData = writer.writeIncData(relativeBinFilename);
    ret->binaryData = writer.writeBinaryData();
    return ret;
}
}
}
