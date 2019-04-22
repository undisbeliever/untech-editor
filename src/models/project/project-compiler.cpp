/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-compiler.h"
#include "rom-data-writer.hpp"
#include "version.h"
#include "models/common/errorlist.h"
#include "models/metasprite/compiler/compiler.h"
#include "models/metasprite/compiler/references.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/project/project.h"

namespace UnTech {
namespace Project {

template <class T>
static std::string itemNameString(const T& item)
{
    return item.name;
}
template <class T>
static std::string itemNameString(const ExternalFileItem<T>& item)
{
    return item.value->name;
}
template <class T>
static std::string itemNameString(const std::unique_ptr<T>& item)
{
    return item->name;
}

static void writeMetaSpriteData(RomDataWriter& writer,
                                const MetaSprite::Compiler::CompiledRomData& msData)
{
    auto writeData = [&](auto& d) {
        writer.addNamedData(d.label(), d.data());
    };

    // Tiles are written first so they are always aligned with
    // the start of the data
    auto tileData = msData.tileData.data();
    for (unsigned i = 0; i < tileData.size(); i++) {
        writer.addNamedData(msData.tileData.blockPrefix() + std::to_string(i),
                            tileData.at(i));
    }

    writeData(msData.frameSetData);
    writeData(msData.frameList);
    writeData(msData.frameData);
    writeData(msData.frameObjectData);
    writeData(msData.tileHitboxData);
    writeData(msData.actionPointData);
    writeData(msData.entityHitboxData);
    writeData(msData.animationList);
    writeData(msData.animationData);
    writeData(msData.paletteData);
}

static Entity::CompiledEntityRomData compileEntityRomData(const ProjectFile& input, std::ostream& errorStream)
{
    ErrorList err;
    auto out = Entity::compileEntityRomData(input.entityRomData, input, err);

    if (err.hasError()) {
        errorStream << "Entity Rom Data:\n";
        err.printIndented(errorStream);
        out.valid = false;
    }

    return out;
}

static void writeEntityRomData(RomDataWriter& writer,
                               const Entity::CompiledEntityRomData& entityData)
{
    writer.addNamedData(entityData.ENTITY_INDEXES_LABEL, entityData.entityIndexes);
    writer.addNamedData(entityData.PROJECTILE_INDEXES_LABEL, entityData.projectileIndexes);
}

std::unique_ptr<ProjectOutput>
compileProject(const ProjectFile& input, const std::string& relativeBinFilename,
               std::ostream& errorStream)
{
    const static std::vector<RomDataWriter::Constant> FORMAT_VERSIONS = {
        { "__resc__.EDITOR_VERSION", UNTECH_VERSION_INT },
        { "Resources.PALETTE_FORMAT_VERSION", Resources::PaletteData::PALETTE_FORMAT_VERSION },
        { "Resources.ANIMATED_TILESET_FORMAT_VERSION", Resources::AnimatedTilesetData::ANIMATED_TILESET_FORMAT_VERSION },
        { "MetaTiles.TILESET_FORMAT_VERSION", MetaTiles::MetaTileTilesetData::TILESET_FORMAT_VERSION },
        { "MetaSprite.Data.METASPRITE_FORMAT_VERSION", MetaSprite::Compiler::CompiledRomData::METASPRITE_FORMAT_VERSION },
        { "Entity.Data.ENTITY_FORMAT_VERSION", Entity::CompiledEntityRomData::ENTITY_FORMAT_VERSION },
    };
    enum TypeId : unsigned {
        PALETTE,
        METATILE_TILESET
    };
    const static std::vector<std::string> TYPE_NAMES = {
        "Project.PaletteList",
        "Project.MetaTileTilesetList"
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

            if (!errorList.empty()) {
                errorStream << typeName << " `" << itemNameString(item) << "`:\n";
                errorList.printIndented(errorStream);
                valid &= errorList.hasError();
            }
        }
    };

    RomDataWriter writer(input.blockSettings.size, input.blockSettings.count,
                         "__resc__", "RES_Lists", "RES_Block",
                         FORMAT_VERSIONS, TYPE_NAMES);

    // MetaSprite data MUST be first.
    auto metaSpriteData = MetaSprite::Compiler::compileMetaSprites(input, errorStream);
    if (metaSpriteData) {
        writeMetaSpriteData(writer, *metaSpriteData);
    }
    valid &= metaSpriteData != nullptr;

    const auto entityData = compileEntityRomData(input, errorStream);
    if (entityData.valid) {
        writeEntityRomData(writer, entityData);
    }
    valid &= entityData.valid;

    compileList(input.palettes, "Palette",
                [&](const Resources::PaletteInput& p, ErrorList& err) {
                    const auto palData = convertPalette(p, err);
                    if (palData) {
                        writer.addData(PALETTE, p.name, palData->exportPalette());
                    }
                });

    if (!valid) {
        // Prevents duplicate palette errors
        return nullptr;
    }

    compileList(input.metaTileTilesets, "MetaTile Tileset",
                [&](const auto& it, ErrorList& err) {
                    assert(it.value != nullptr);
                    const auto mtd = MetaTiles::convertTileset(*it.value, input, err);
                    if (mtd) {
                        const auto data = mtd->exportMetaTileTileset();
                        writer.addData(METATILE_TILESET, it.value->name, data);
                    }
                });

    if (!valid) {
        return nullptr;
    }

    auto ret = std::make_unique<ProjectOutput>();
    ret->binaryData = writer.writeBinaryData();

    writer.writeIncData(ret->incData, relativeBinFilename);
    metaSpriteData->writeToIncFile(ret->incData);
    ret->incData << entityData.entries;

    MetaSprite::Compiler::writeFrameSetReferences(input, ret->incData);
    MetaSprite::Compiler::writeExportOrderReferences(input, ret->incData);
    ret->incData << entityData.defines;

    // changes ROM BANK to code()
    MetaSprite::Compiler::writeActionPointFunctionTables(input.actionPointFunctions, ret->incData);

    ret->incData << std::endl;

    return ret;
}
}
}
