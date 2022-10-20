/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstract-editor.h"
#include "models/common/iterators.h"
#include "models/enums.h"
#include "models/project/project.h"

#include "gui/editors/action-points-editor.h"
#include "gui/editors/background-image-editor.h"
#include "gui/editors/bytecode-editor.h"
#include "gui/editors/entity-rom-data-editor.h"
#include "gui/editors/frameset-export-order-editor.h"
#include "gui/editors/game-state-editor.h"
#include "gui/editors/interactive-tiles-editor.h"
#include "gui/editors/metasprite-editor.h"
#include "gui/editors/metatile-tileset-editor.h"
#include "gui/editors/palette-editor.h"
#include "gui/editors/project-settings-editor.h"
#include "gui/editors/room-editor.h"
#include "gui/editors/scenes-editor-editor.h"
#include "gui/editors/spriteimporter-editor.h"

namespace UnTech::Gui {

void AbstractExternalFileEditorData::setFilename(const std::filesystem::path& fn)
{
    _filename = fn;
    _basename = _filename.filename().u8string();
}

void AbstractEditorGui::processExtraWindows(const Project::ProjectFile&, const Project::ProjectData&)
{
}

void AbstractEditorGui::viewMenu()
{
}

void AbstractEditorGui::undoStackButtons()
{
    if (ImGui::Button("Undo")) {
        undoClicked = true;
    }
    ImGui::SameLine();

    if (ImGui::Button("Redo")) {
        redoClicked = true;
    }
}

std::unique_ptr<AbstractEditorData> createEditor(ItemIndex itemIndex,
                                                 const UnTech::Project::ProjectFile& projectFile)
{
    using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;

    switch (itemIndex.type) {
    case ResourceType::ProjectSettings: {
        switch (ProjectSettingsIndex(itemIndex.index)) {
        case ProjectSettingsIndex::ProjectSettings:
            return std::make_unique<ProjectSettingsEditorData>(itemIndex);

        case ProjectSettingsIndex::GameState:
            return std::make_unique<GameStateEditorData>(itemIndex);

        case ProjectSettingsIndex::Bytecode:
            return std::make_unique<BytecodeEditorData>(itemIndex);

        case ProjectSettingsIndex::InteractiveTiles:
            return std::make_unique<InteractiveTilesEditorData>(itemIndex);

        case ProjectSettingsIndex::ActionPoints:
            return std::make_unique<ActionPointsEditorData>(itemIndex);

        case ProjectSettingsIndex::EntityRomData:
            return std::make_unique<EntityRomDataEditorData>(itemIndex);

        case ProjectSettingsIndex::Scenes:
            return std::make_unique<ScenesEditorData>(itemIndex);
        }
        return nullptr;
    }

    case ResourceType::FrameSetExportOrders:
        return std::make_unique<FrameSetExportOrderEditorData>(itemIndex);

    case ResourceType::FrameSets: {
        if (itemIndex.index < projectFile.frameSets.size()) {
            switch (projectFile.frameSets.at(itemIndex.index).type) {
            case FrameSetType::UNKNOWN:
                return nullptr;

            case FrameSetType::METASPRITE:
                return std::make_unique<MetaSpriteEditorData>(itemIndex);

            case FrameSetType::SPRITE_IMPORTER:
                return std::make_unique<SpriteImporterEditorData>(itemIndex);
            }
        }
        return nullptr;
    }

    case ResourceType::Palettes:
        return std::make_unique<PaletteEditorData>(itemIndex);

    case ResourceType::BackgroundImages:
        return std::make_unique<BackgroundImageEditorData>(itemIndex);

    case ResourceType::MataTileTilesets:
        return std::make_unique<MetaTileTilesetEditorData>(itemIndex);

    case ResourceType::Rooms:
        return std::make_unique<RoomEditorData>(itemIndex);
    }

    return nullptr;
}

std::vector<std::unique_ptr<AbstractEditorGui>> createEditorGuis()
{
    constexpr unsigned N_ELEMENTS = 14;

    std::vector<std::unique_ptr<AbstractEditorGui>> ret;
    ret.reserve(N_ELEMENTS);

    ret.push_back(std::make_unique<ProjectSettingsEditorGui>());
    ret.push_back(std::make_unique<GameStateEditorGui>());
    ret.push_back(std::make_unique<BytecodeEditorGui>());
    ret.push_back(std::make_unique<InteractiveTilesEditorGui>());
    ret.push_back(std::make_unique<ActionPointsEditorGui>());
    ret.push_back(std::make_unique<EntityRomDataEditorGui>());
    ret.push_back(std::make_unique<ScenesEditorGui>());
    ret.push_back(std::make_unique<FrameSetExportOrderEditorGui>());
    ret.push_back(std::make_unique<MetaSpriteEditorGui>());
    ret.push_back(std::make_unique<SpriteImporterEditorGui>());
    ret.push_back(std::make_unique<PaletteEditorGui>());
    ret.push_back(std::make_unique<BackgroundImageEditorGui>());
    ret.push_back(std::make_unique<MetaTileTilesetEditorGui>());
    ret.push_back(std::make_unique<RoomEditorGui>());

    assert(ret.size() == N_ELEMENTS);

    return ret;
}

}
