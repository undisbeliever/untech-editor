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
#include "gui/editors/background-image-editor.h"
#include "gui/editors/entity-rom-data-editor.h"
#include "gui/editors/frameset-export-order-editor.h"
#include "gui/editors/interactive-tiles-editor.h"
#include "gui/editors/metasprite-editor.h"
#include "gui/editors/metatile-tileset-editor.h"
#include "gui/editors/palette-editor.h"
#include "gui/editors/project-settings-editor.h"
#include "gui/editors/room-editor.h"
#include "gui/editors/scenes-editor-editor.h"
#include "gui/editors/spriteimporter-editor.h"

namespace UnTech::Gui {

// ::TODO replace with a circular buffer::
void trimStack(std::vector<std::unique_ptr<EditorUndoAction>>& stack)
{
    while (stack.size() > AbstractEditor::N_UNDO_ACTIONS) {
        stack.erase(stack.begin());
    }
}

AbstractEditor::AbstractEditor(const ItemIndex itemIndex)
    : _itemIndex(itemIndex)
    , _basename()
    , _pendingActions()
    , _undoStack()
    , _redoStack()
    , _clean(true)
{
}

void AbstractEditor::addAction(std::unique_ptr<EditorUndoAction>&& action)
{
    _pendingActions.push_back(std::move(action));
}

void AbstractEditor::processPendingActions(Project::ProjectFile& projectFile)
{
    const auto pendingSize = _pendingActions.size();

    for (auto& pa : _pendingActions) {
        auto action = std::move(pa);

        const auto undoSize = _undoStack.size();
        const auto redoSize = _redoStack.size();

        const bool modifed = action->firstDo(projectFile);
        _clean = false;

        assert(_undoStack.size() == undoSize && "EditorUndoAction must not modify the undo stack");
        assert(_redoStack.size() == redoSize && "EditorUndoAction must not modify the undo stack");
        assert(_pendingActions.size() == pendingSize && "EditorUndoAction must not call addAction");

        if (modifed) {
            _undoStack.push_back(std::move(action));
            _redoStack.clear();
        }
    }

    _pendingActions.clear();

    trimStack(_undoStack);

    // action may have modified the selection
    updateSelection();
}

void AbstractEditor::undo(Project::ProjectFile& projectFile)
{
    if (_undoStack.empty()) {
        return;
    }
    if (!_pendingActions.empty()) {
        return;
    }

    // Discard any uncommitted changes
    loadDataFromProject(projectFile);

    auto a = std::move(_undoStack.back());
    _undoStack.pop_back();

    const auto undoSize = _undoStack.size();
    const auto redoSize = _redoStack.size();

    a->undo(projectFile);
    _clean = false;

    assert(_undoStack.size() == undoSize && "EditorUndoAction must not modify the undo stack");
    assert(_redoStack.size() == redoSize && "EditorUndoAction must not modify the undo stack");
    assert(_pendingActions.empty() && "EditorUndoAction must not call addAction");

    _redoStack.push_back(std::move(a));
    trimStack(_redoStack);

    // action may have modified the selection
    updateSelection();
}

void AbstractEditor::redo(Project::ProjectFile& projectFile)
{
    if (_redoStack.empty()) {
        return;
    }
    if (!_pendingActions.empty()) {
        return;
    }

    // Discard any uncommitted changes
    loadDataFromProject(projectFile);

    auto a = std::move(_redoStack.back());
    _redoStack.pop_back();

    const auto undoSize = _undoStack.size();
    const auto redoSize = _redoStack.size();

    a->redo(projectFile);
    _clean = false;

    assert(_undoStack.size() == undoSize && "EditorUndoAction must not modify the undo stack");
    assert(_redoStack.size() == redoSize && "EditorUndoAction must not modify the undo stack");
    assert(_pendingActions.empty() && "EditorUndoAction must not call addAction");

    _undoStack.push_back(std::move(a));
    trimStack(_undoStack);

    // action may have modified the selection
    updateSelection();
}

void AbstractExternalFileEditor::setFilename(const std::filesystem::path& fn)
{
    _filename = fn;
    _basename = _filename.filename();
}

std::unique_ptr<AbstractEditor> createEditor(ItemIndex itemIndex,
                                             const UnTech::Project::ProjectFile& projectFile)
{
    using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;

    switch (itemIndex.type) {
    case EditorType::ProjectSettings: {
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
    }

    case EditorType::FrameSetExportOrders:
        return std::make_unique<FrameSetExportOrderEditor>(itemIndex);

    case EditorType::FrameSets: {
        if (itemIndex.index < projectFile.frameSets.size()) {
            switch (projectFile.frameSets.at(itemIndex.index).type) {
            case FrameSetType::UNKNOWN:
                return nullptr;

            case FrameSetType::METASPRITE:
                return std::make_unique<MetaSpriteEditor>(itemIndex);

            case FrameSetType::SPRITE_IMPORTER:
                return std::make_unique<SpriteImporterEditor>(itemIndex);
            }
        }
        return nullptr;
    }

    case EditorType::Palettes:
        return std::make_unique<PaletteEditor>(itemIndex);

    case EditorType::BackgroundImages:
        return std::make_unique<BackgroundImageEditor>(itemIndex);

    case EditorType::MataTileTilesets:
        return std::make_unique<MetaTileTilesetEditor>(itemIndex);

    case EditorType::Rooms:
        return std::make_unique<RoomEditor>(itemIndex);
    }

    return nullptr;
}

}
