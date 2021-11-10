/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-compiler.h"
#include "project-data.h"
#include "rom-data-writer.hpp"
#include "version.h"
#include "models/common/errorlist.h"
#include "models/common/iterators.h"
#include "models/metasprite/compiler/compiler.h"
#include "models/metasprite/compiler/framesetcompiler.h"
#include "models/metasprite/compiler/references.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/project/project.h"
#include "models/snes/tile-data.h"

namespace UnTech::Project {

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

    for (const auto i : range(fsData.size())) {
        const auto fs = fsData.at(i);
        assert(fs);
        msData.addFrameSetData(*fs);
    }

    // Tiles are written first so they are always aligned with
    // the start of the data
    for (const auto& tileBank : msData.tileData.tileBanks()) {
        writer.addBankData(tileBank.bankId, tileBank.startingAddress,
                           Snes::snesTileData4bppTile16(tileBank.tiles));
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
    writeNotNullData(msData.actionPointData);
    writeNotNullData(msData.collisionBoxData);
}

static void writeEntityRomData(RomDataWriter& writer,
                               const Entity::CompiledEntityRomData& entityData)
{
    writer.addNamedData(entityData.ROM_DATA_LIST_LABEL, entityData.romDataIndexes);
    writer.addNamedData(entityData.ROM_DATA_LABEL, entityData.romData);
}

static void writeSceneData(RomDataWriter& writer,
                           const Resources::CompiledScenesData& scenes)
{
    writer.addNamedDataWithCount(scenes.sceneSettings.DATA_LABEL, scenes.sceneSettings.sceneSettings, scenes.sceneSettings.nSceneSettings);
    writer.addNamedDataWithCount(scenes.sceneLayouts.DATA_LABEL, scenes.sceneLayouts.sceneLayoutData(), scenes.sceneLayouts.nLayouts());
    writer.addNamedDataWithCount(scenes.DATA_LABEL, scenes.sceneSnesData, scenes.scenes.size());
}

template <typename T>
static void writeIncList(StringStream& incData, const std::string& typeName, const DataStore<T>& dataStore)
{
    incData.write("\nnamespace ", typeName, " {\n");

    dataStore.readResourceListState([&](auto state, const auto& resources) {
        static_assert(std::is_const_v<std::remove_reference_t<decltype(resources)>>);

        assert(state == ResourceState::Valid);

        for (auto [id, s] : const_enumerate(resources)) {
            assert(s.state == ResourceState::Valid);
            incData.write("  constant ", s.name, " = ", id, "\n");
        }
    });
    incData.write("}\n"
                  "\n");
}

static void printErrors(const ProjectData& projectData, StringStream& errorStream)
{
    auto print = [&](const ResourceListStatus& listStatus) {
        listStatus.readResourceListState([&](auto& state, auto& resources) {
            static_assert(std::is_const_v<std::remove_reference_t<decltype(state)>>);
            static_assert(std::is_const_v<std::remove_reference_t<decltype(resources)>>);

            for (const ResourceStatus& status : resources) {
                if (!status.errorList.empty()) {
                    errorStream.write(listStatus.typeNameSingle(), " `", status.name, "`:\n");
                    status.errorList.printIndented(errorStream);
                }
            }
        });
    };

    print(projectData.projectSettingsStatus());
    print(projectData.frameSetExportOrderStatus());
    print(projectData.frameSets());
    print(projectData.palettes());
    print(projectData.backgroundImages());
    print(projectData.metaTileTilesets());
    print(projectData.rooms());
};

std::unique_ptr<ProjectOutput>
compileProject(const ProjectFile& input, const std::filesystem::path& relativeBinFilename,
               StringStream& errorStream)
{
    ProjectData projectData;

    const bool valid = projectData.compileAll_EarlyExit(input);

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
        { "Scripting.GAME_STATE_FORMAT_VERSION", Scripting::GameStateData::GAME_STATE_FORMAT_VERSION },
        { "Project.ROOM_DATA_SIZE", input.projectSettings.roomSettings.roomDataSize },
        { "Project.MS_FrameSetListCount", unsigned(input.frameSets.size()) },
    };

    RomDataWriter writer(input.projectSettings.memoryMap,
                         "__resc__", "RES_Block",
                         constants);

    const auto gameStateData = projectData.gameState();
    assert(gameStateData);

    // must write meta sprite data first
    writeMetaSpriteData(writer, input.projectSettings.memoryMap, projectData.frameSets());
    writeEntityRomData(writer, *projectData.entityRomData());
    writeSceneData(writer, *projectData.scenes());

    writer.addNamedData("Project.InitialGameState", gameStateData->exportSnesData());

    writer.addDataStore("Project.PaletteList", projectData.palettes());
    writer.addDataStore("Project.BackgroundImageList", projectData.backgroundImages());
    writer.addDataStore("Project.MetaTileTilesetList", projectData.metaTileTilesets());
    writer.addDataStore("Project.RoomList", projectData.rooms());

    // The inc file is large: increase StringStream buffer size.
    StringStream incData(64 * 1024);

    writer.writeIncData(incData, relativeBinFilename);
    writeIncList(incData, "Project.RoomList", projectData.rooms());

    Scripting::writeGameStateConstants(input.gameState, *gameStateData, incData);
    MetaSprite::Compiler::writeFrameSetReferences(input, incData);
    MetaSprite::Compiler::writeExportOrderReferences(input, incData);

    incData.write(projectData.entityRomData()->defines);

    // changes ROM BANK to code()
    Scripting::writeBytecodeFunctionTable(input.bytecode, incData);
    MetaSprite::Compiler::writeActionPointFunctionTables(input.actionPointFunctions, incData);
    Resources::writeSceneIncData(input.resourceScenes, incData);

    incData.write(projectData.entityRomData()->functionTableData);

    MetaTiles::writeFunctionTables(incData, input.interactiveTiles);

    incData.write("\n");

    auto ret = std::make_unique<ProjectOutput>();
    ret->incData = incData.takeString();
    ret->binaryData = writer.writeBinaryData();

    return ret;
}

}
