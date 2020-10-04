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
#include "models/metasprite/compiler/framesetcompiler.h"
#include "models/metasprite/compiler/references.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/project/project.h"

namespace UnTech {
namespace Project {

static const idstring BLANK_IDSTRING{};

static void writeMetaSpriteData(RomDataWriter& writer,
                                const Project::MemoryMapSettings& memoryMap,
                                const DataStore<UnTech::MetaSprite::Compiler::FrameSetData>& fsData)
{
    auto writeData = [&](auto& d) {
        writer.addNamedData(d.label(), d.data());
    };
    auto writeNotNullData = [&](auto& d) {
        writer.addNotNullNamedData(d.label(), d.data());
    };

    MetaSprite::Compiler::CompiledRomData msData(memoryMap);

    for (unsigned i = 0; i < fsData.size(); i++) {
        auto fs = fsData.at(i);
        assert(fs);
        msData.addFrameSetData(*fs);
    }

    // Tiles are written first so they are always aligned with
    // the start of the data
    for (const auto& tileBank : msData.tileData.tileBanks()) {
        writer.addBankData(tileBank.bankId, tileBank.startingAddress, tileBank.tiles.snesData());
    }

    writeData(msData.frameSetData);
    writeData(msData.frameList);
    writeData(msData.frameData);
    writeData(msData.animationList);
    writeData(msData.animationData);
    writeData(msData.paletteList);
    writeData(msData.paletteData);

    writeNotNullData(msData.dmaTile16Data);
    writeNotNullData(msData.frameObjectData);
    writeNotNullData(msData.entityHitboxData);
    writeNotNullData(msData.actionPointData);
}

static void writeEntityRomData(RomDataWriter& writer,
                               const Entity::CompiledEntityRomData& entityData)
{
    writer.addNamedData(entityData.ROM_DATA_LIST_LABEL, entityData.romDataIndexes);
    writer.addNamedData(entityData.ROM_DATA_LABEL, entityData.romData);
}

static void writeSceneData(RomDataWriter& writer,
                           const Resources::SceneSettingsData& settings, const Resources::CompiledScenesData& scenes)
{
    writer.addNamedDataWithCount(settings.DATA_LABEL, settings.sceneSettings, settings.nSceneSettings);
    writer.addNamedDataWithCount(scenes.sceneLayouts.DATA_LABEL, scenes.sceneLayouts.sceneLayoutData(), scenes.sceneLayouts.nLayouts());
    writer.addNamedDataWithCount(scenes.DATA_LABEL, scenes.sceneSnesData, scenes.scenes.size());
}

template <typename T>
static void writeIncList(std::stringstream& incData, const std::string& typeName, const DataStore<T>& dataStore)
{
    incData << "\nnamespace " << typeName << " {\n";

    for (unsigned id = 0; id < dataStore.size(); id++) {
        const auto item = dataStore.at(id);
        incData << "  constant " << item->name << " = " << id << '\n';
    }
    incData << "}\n"
               "\n";
}

static void printErrors(const ProjectData& projectData, std::ostream& errorStream)
{
    auto print = [&](const ResourceListStatus& listStatus) {
        for (const ResourceStatus& status : listStatus.resources) {
            if (!status.errorList.empty()) {
                errorStream << listStatus.typeNameSingle << " `" << status.name << "`:\n";
                status.errorList.printIndented(errorStream);
            }
        }
    };

    print(projectData.projectSettingsStatus());
    print(projectData.frameSetExportOrderStatus());
    print(projectData.frameSets().listStatus());
    print(projectData.palettes().listStatus());
    print(projectData.backgroundImages().listStatus());
    print(projectData.metaTileTilesets().listStatus());
    print(projectData.rooms().listStatus());
};

std::unique_ptr<ProjectOutput>
compileProject(const ProjectFile& input, const std::filesystem::path& relativeBinFilename,
               std::ostream& errorStream)
{
    ProjectData projectData(input);

    const bool valid = projectData.compileAll();

    printErrors(projectData, errorStream);

    if (!valid) {
        return nullptr;
    }

    const std::vector<RomDataWriter::Constant> constants = {
        { "__resc__.EDITOR_VERSION", UNTECH_VERSION_INT },
        { "Resources.PALETTE_FORMAT_VERSION", Resources::PaletteData::PALETTE_FORMAT_VERSION },
        { "Resources.ANIMATED_TILESET_FORMAT_VERSION", Resources::AnimatedTilesetData::ANIMATED_TILESET_FORMAT_VERSION },
        { "Resources.BACKGROUND_IMAGE_FORMAT_VERSION", Resources::BackgroundImageData::BACKGROUND_IMAGE_FORMAT_VERSION },
        { "Resources.SCENE_FORMAT_VERSION", Resources::CompiledScenesData::SCENE_FORMAT_VERSION },
        { "MetaTiles.TILESET_FORMAT_VERSION", MetaTiles::MetaTileTilesetData::TILESET_FORMAT_VERSION },
        { "MetaTiles.INTERACTIVE_TILES_FORMAT_VERSION", MetaTiles::INTERACTIVE_TILES_FORMAT_VERSION },
        { "MetaSprite.Data.METASPRITE_FORMAT_VERSION", MetaSprite::Compiler::CompiledRomData::METASPRITE_FORMAT_VERSION },
        { "Entity.Data.ENTITY_FORMAT_VERSION", Entity::CompiledEntityRomData::ENTITY_FORMAT_VERSION },
        { "Room.ROOM_FORMAT_VERSION", Rooms::RoomData::ROOM_FORMAT_VERSION },
        { "Project.ROOM_DATA_SIZE", input.roomSettings.roomDataSize },
        { "Project.MS_FrameSetListCount", unsigned(input.frameSets.size()) },
    };

    RomDataWriter writer(input.memoryMap,
                         "__resc__", "RES_Block",
                         constants);

    // must write meta sprite data first
    writeMetaSpriteData(writer, input.memoryMap, projectData.frameSets());
    writeEntityRomData(writer, *projectData.entityRomData());
    writeSceneData(writer, *projectData.sceneSettings(), *projectData.scenes());

    writer.addDataStore("Project.PaletteList", projectData.palettes());
    writer.addDataStore("Project.BackgroundImageList", projectData.backgroundImages());
    writer.addDataStore("Project.MetaTileTilesetList", projectData.metaTileTilesets());
    writer.addDataStore("Project.RoomList", projectData.rooms());

    auto ret = std::make_unique<ProjectOutput>();
    ret->binaryData = writer.writeBinaryData();

    writer.writeIncData(ret->incData, relativeBinFilename);
    writeIncList(ret->incData, "Project.RoomList", projectData.rooms());

    MetaSprite::Compiler::writeFrameSetReferences(input, ret->incData);
    MetaSprite::Compiler::writeExportOrderReferences(input, ret->incData);
    ret->incData << projectData.entityRomData()->defines;

    // changes ROM BANK to code()
    MetaSprite::Compiler::writeActionPointFunctionTables(input.actionPointFunctions, ret->incData);
    Resources::writeSceneIncData(input.resourceScenes, ret->incData);
    ret->incData << projectData.entityRomData()->functionTableData;

    MetaTiles::writeFunctionTables(ret->incData, input.interactiveTiles);

    ret->incData << std::endl;

    return ret;
}
}
}
