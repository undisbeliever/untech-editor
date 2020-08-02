/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstract-editor.h"
#include "enums.h"
#include "imgui.h"
#include "models/project/project.h"

#include "gui/editors/action-points-editor.h"
#include "gui/editors/entity-rom-data-editor.h"
#include "gui/editors/interactive-tiles-editor.h"
#include "gui/editors/project-settings-editor.h"
#include "gui/editors/scenes-editor-editor.h"

namespace UnTech::Gui {

AbstractEditor::AbstractEditor(const ItemIndex itemIndex)
    : _itemIndex(itemIndex)
    , pendingChanges(false)
{
}

std::unique_ptr<AbstractEditor> createEditor(ItemIndex itemIndex)
{
    switch (itemIndex.type) {
    case EditorType::ProjectSettings:
        switch (ProjectSettingsIndex(itemIndex.index)) {
        case ProjectSettingsIndex::ProjectSettings:
            return std::make_unique<ProjectSettingsEditor>(itemIndex);

        case ProjectSettingsIndex::InteractiveTiles:
            return std::make_unique<InteractiveTilesEditor>(itemIndex);

        case ProjectSettingsIndex::ActionPoints:
            return std::make_unique<ActionPointsEditor>(itemIndex);

        case ProjectSettingsIndex::EntityRomData:
            return std::make_unique<EntityRomDataEditor>(itemIndex);

        case ProjectSettingsIndex::Scenes:
            return std::make_unique<ScenesEditor>(itemIndex);
        }
        return nullptr;

    case EditorType::FrameSetExportOrders:
    case EditorType::FrameSets:
    case EditorType::Palettes:
    case EditorType::BackgroundImages:
    case EditorType::MataTileTilesets:
    case EditorType::Rooms:
        return nullptr;
    }

    return nullptr;
}

}
