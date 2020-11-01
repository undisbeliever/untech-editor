/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstract-editor.h"
#include "imgui.h"
#include "models/enums.h"
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

class MacroAction final : public EditorUndoAction {
private:
    std::vector<std::unique_ptr<EditorUndoAction>> actions;

public:
    MacroAction()
        : EditorUndoAction()
    {
    }
    virtual ~MacroAction() = default;

    void addAction(std::unique_ptr<EditorUndoAction>&& a)
    {
        assert(a != nullptr);
        actions.push_back(std::move(a));
    }

    virtual bool firstDo(Project::ProjectFile& projectFile) final
    {
        bool changed = false;
        for (auto& a : actions) {
            assert(a != nullptr);

            const bool c = a->firstDo(projectFile);
            changed |= c;
            if (!c) {
                a = nullptr;
            }
        }
        return changed;
    }

    virtual void undo(Project::ProjectFile& projectFile) const final
    {
        for (auto it = actions.rbegin(); it != actions.rend(); it++) {
            auto& a = *it;
            if (a) {
                a->undo(projectFile);
            }
        }
    }

    virtual void redo(Project::ProjectFile& projectFile) const final
    {
        for (auto& a : actions) {
            if (a) {
                a->redo(projectFile);
            }
        }
    }
};

void trimStack(std::vector<std::unique_ptr<EditorUndoAction>>& stack)
{
    while (stack.size() > AbstractEditorData::N_UNDO_ACTIONS) {
        stack.erase(stack.begin());
    }
}

AbstractEditorData::AbstractEditorData(const ItemIndex itemIndex)
    : _itemIndex(itemIndex)
    , _basename()
    , _pendingActions()
    , _undoStack()
    , _redoStack()
    , _clean(true)
    , _inMacro(false)
{
}

void AbstractEditorData::addAction(std::unique_ptr<EditorUndoAction>&& action)
{
    if (_inMacro == false) {
        _pendingActions.push_back(std::move(action));
    }
    else {
        assert(!_pendingActions.empty());
        if (!_pendingActions.empty()) {
            auto* m = dynamic_cast<MacroAction*>(_pendingActions.back().get());
            assert(m);
            if (m) {
                m->addAction(std::move(action));
            }
        }
    }
}

void AbstractEditorData::startMacro()
{
    assert(_inMacro == false);
    if (_inMacro == false) {
        _pendingActions.push_back(std::make_unique<MacroAction>());
        _inMacro = true;
    }
}

void AbstractEditorData::endMacro()
{
    _inMacro = false;
}

bool AbstractEditorData::processPendingActions(Project::ProjectFile& projectFile)
{
    const auto pendingSize = _pendingActions.size();

    bool edited = false;

    assert(_inMacro == false);
    _inMacro = false;

    for (auto& pa : _pendingActions) {
        auto action = std::move(pa);

        const auto undoSize = _undoStack.size();
        const auto redoSize = _redoStack.size();

        const bool modifed = action->firstDo(projectFile);
        _clean = false;
        edited = true;

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

    return edited;
}

bool AbstractEditorData::undo(Project::ProjectFile& projectFile)
{
    if (_undoStack.empty()) {
        return false;
    }
    if (!_pendingActions.empty()) {
        return false;
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

    return true;
}

bool AbstractEditorData::redo(Project::ProjectFile& projectFile)
{
    if (_redoStack.empty()) {
        return false;
    }
    if (!_pendingActions.empty()) {
        return false;
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

    return true;
}

void AbstractExternalFileEditorData::setFilename(const std::filesystem::path& fn)
{
    _filename = fn;
    _basename = _filename.filename();
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

bool processUndoStack(AbstractEditorGui* gui, AbstractEditorData* editor, Project::ProjectFile& pf)
{
    bool edited = false;

    if (gui->undoClicked) {
        edited = editor->undo(pf);
        gui->editorDataChanged();
    }
    else if (gui->redoClicked) {
        edited = editor->redo(pf);
        gui->editorDataChanged();
    }

    gui->undoClicked = false;
    gui->redoClicked = false;

    return edited;
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
    constexpr unsigned N_ELEMENTS = 12;

    std::vector<std::unique_ptr<AbstractEditorGui>> ret;
    ret.reserve(N_ELEMENTS);

    ret.push_back(std::make_unique<ProjectSettingsEditorGui>());
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
