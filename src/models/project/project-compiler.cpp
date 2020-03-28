/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-compiler.h"
#include "project-data.h"
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

static const idstring BLANK_IDSTRING{};

template <class T>
static const idstring& itemNameString(const T& item)
{
    return item.name;
}
template <class T>
static const idstring& itemNameString(const T* item)
{
    return item ? item->name : BLANK_IDSTRING;
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
        writer.addNamedData(stringBuilder(msData.tileData.blockPrefix(), i),
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

static Resources::SceneSettingsData compileSceneSettingsData(const ProjectFile& input, std::ostream& errorStream)
{
    ErrorList err;
    auto out = Resources::compileSceneSettingsData(input.resourceScenes.settings, err);

    if (err.hasError()) {
        errorStream << "Scene Settings Data:\n";
        err.printIndented(errorStream);
        out.valid = false;
    }

    return out;
}

static Resources::CompiledScenesData compileSceneData(const ProjectFile& input, const Resources::SceneSettingsData& settingsData, const ProjectData& projectData,
                                                      std::ostream& errorStream)
{
    ErrorList err;
    auto out = Resources::compileScenesData(input.resourceScenes, settingsData, projectData, err);

    if (err.hasError()) {
        errorStream << "Scene Data:\n";
        err.printIndented(errorStream);
        out.valid = false;
    }

    return out;
}

static void writeSceneData(RomDataWriter& writer,
                           const Resources::SceneSettingsData& settings, const Resources::CompiledScenesData& scenes)
{
    writer.addNamedDataWithCount(settings.DATA_LABEL, settings.sceneSettings, settings.nSceneSettings);
    writer.addNamedDataWithCount(scenes.sceneLayouts.DATA_LABEL, scenes.sceneLayouts.sceneLayoutData(), scenes.sceneLayouts.nLayouts());
    writer.addNamedDataWithCount(scenes.DATA_LABEL, scenes.sceneSnesData, scenes.scenes.size());
}

std::unique_ptr<ProjectOutput>
compileProject(const ProjectFile& input, const std::filesystem::path& relativeBinFilename,
               std::ostream& errorStream)
{
    const static std::vector<RomDataWriter::Constant> FORMAT_VERSIONS = {
        { "__resc__.EDITOR_VERSION", UNTECH_VERSION_INT },
        { "Resources.PALETTE_FORMAT_VERSION", Resources::PaletteData::PALETTE_FORMAT_VERSION },
        { "Resources.ANIMATED_TILESET_FORMAT_VERSION", Resources::AnimatedTilesetData::ANIMATED_TILESET_FORMAT_VERSION },
        { "Resources.BACKGROUND_IMAGE_FORMAT_VERSION", Resources::BackgroundImageData::BACKGROUND_IMAGE_FORMAT_VERSION },
        { "Resources.SCENE_FORMAT_VERSION", Resources::CompiledScenesData::SCENE_FORMAT_VERSION },
        { "MetaTiles.TILESET_FORMAT_VERSION", MetaTiles::MetaTileTilesetData::TILESET_FORMAT_VERSION },
        { "MetaSprite.Data.METASPRITE_FORMAT_VERSION", MetaSprite::Compiler::CompiledRomData::METASPRITE_FORMAT_VERSION },
        { "Entity.Data.ENTITY_FORMAT_VERSION", Entity::CompiledEntityRomData::ENTITY_FORMAT_VERSION },
    };
    enum TypeId : unsigned {
        PALETTE,
        BACKGROUND_IMAGE,
        METATILE_TILESET
    };
    const static std::vector<std::string> TYPE_NAMES = {
        "Project.PaletteList",
        "Project.BackgroundImageList",
        "Project.MetaTileTilesetList",
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

    ProjectData projectData(input);

    auto compileList = [&](const auto& sourceList, auto compile_fn, const char* typeName) {
        for (unsigned i = 0; i < sourceList.size(); i++) {
            ErrorList errorList;

            valid &= std::invoke(compile_fn, projectData, i, errorList);

            if (!errorList.empty()) {
                errorStream << typeName << " `" << itemNameString(sourceList.at(i)) << "`:\n";
                errorList.printIndented(errorStream);
                valid &= errorList.hasError();
            }
        }
    };

    // MetaSprite data MUST be first.
    auto metaSpriteData = MetaSprite::Compiler::compileMetaSprites(input, errorStream);
    if (metaSpriteData == nullptr) {
        return nullptr;
    }

    const auto entityData = compileEntityRomData(input, errorStream);
    if (entityData.valid == false) {
        return nullptr;
    }

    compileList(input.palettes, &ProjectData::compilePalette, "Palette");
    if (!valid) {
        return nullptr;
    }

    compileList(input.backgroundImages, &ProjectData::compileBackgroundImage, "Background Image");
    // no !valid test needed, metaTileTilesets and backgroundImages are unrelated

    compileList(input.metaTileTilesets, &ProjectData::compileMetaTiles, "MetaTile Tileset");
    if (!valid) {
        return nullptr;
    }

    const auto sceneSettingsData = compileSceneSettingsData(input, errorStream);
    if (entityData.valid == false) {
        return nullptr;
    }
    const auto sceneData = compileSceneData(input, sceneSettingsData, projectData, errorStream);
    if (entityData.valid == false) {
        return nullptr;
    }

    RomDataWriter writer(input.blockSettings.size, input.blockSettings.count,
                         "__resc__", "RES_Lists", "RES_Block",
                         FORMAT_VERSIONS, TYPE_NAMES);

    auto writeData = [&](const TypeId typeId, const auto& list) {
        for (unsigned i = 0; i < list.size(); i++) {
            auto data = list.at(i);
            assert(data);
            writer.addData(typeId, data->name, data->exportSnesData());
        }
    };

    // must write meta sprite data first
    writeMetaSpriteData(writer, *metaSpriteData);
    writeEntityRomData(writer, entityData);
    writeSceneData(writer, sceneSettingsData, sceneData);
    writeData(PALETTE, projectData.palettes());
    writeData(BACKGROUND_IMAGE, projectData.backgroundImages());
    writeData(METATILE_TILESET, projectData.metaTileTilesets());

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
    Resources::writeSceneIncData(input.resourceScenes, ret->incData);

    ret->incData << std::endl;

    return ret;
}
}
}
