/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <vector>

namespace UnTech::Project {
struct ProjectFile;
}

namespace UnTech::Gui {

class AbstractEditorGui;
class AbstractEditorData;

bool processUndoStack(AbstractEditorData*, AbstractEditorGui*, Project::ProjectFile&);

// `UndoAction`s must not modify an editor's `UndoStack`.
class UndoAction {
public:
    // Disable copying or moving undo actions
    UndoAction(const UndoAction&) = delete;
    UndoAction(UndoAction&&) = delete;
    UndoAction& operator=(const UndoAction&) = delete;
    UndoAction& operator=(UndoAction&&) = delete;

    virtual ~UndoAction() = default;

public:
    UndoAction() = default;

    // Notify the GUI that the data has changed.
    //
    // AbstractEditorGui* may be null
    //
    // Called after `firstDo_editorData()`, `undo()` or `redo()` is called.
    virtual void notifyGui(AbstractEditorGui*) const = 0;

    // Preforms an action to the editor data.
    //
    // This function should update the selection if the list size or order is changed.
    //
    // This function is called after the GUI has been processed.
    //
    // The firstDo function is split into separate editorData and projectFile functions.
    // This allows for the editor data to be updated immediately after the GUI has been
    // processed, while also allowing for projectFile updates to be delayed until the
    // background thread is no-longer blocking projectFile writes.
    //
    virtual void firstDo_editorData() const = 0;

    // Preforms an action to the project data.
    //
    // Returns true if the action modified the project data.
    // If this function returns false then the undo action will not be added to the stack.
    //
    // This function MUST NOT modify editor data or selection.
    //
    // This function is called after `firstDo_editorData()`.
    //
    [[nodiscard]] virtual bool firstDo_projectFile(UnTech::Project::ProjectFile&) = 0;

    // Undoes an action.
    // This function MUST update both editor and project data.
    // This function should update the selection if the list size or order is changed.
    virtual void undo(UnTech::Project::ProjectFile&) const = 0;

    // Redoes an undone action.
    // This function MUST update both editor and project data.
    // This function should update the selection if the list size or order is changed.
    virtual void redo(UnTech::Project::ProjectFile&) const = 0;
};

class UndoStack {
public:
    constexpr static unsigned N_UNDO_ACTIONS = 200;

private:
    std::vector<std::unique_ptr<UndoAction>> _pendingEditorActions;
    std::vector<std::unique_ptr<UndoAction>> _pendingProjectFileActions;

    // Using vector so I can trim the stacks when they get too large
    std::vector<std::unique_ptr<UndoAction>> _undoStack;
    std::vector<std::unique_ptr<UndoAction>> _redoStack;

    bool _clean = true;
    bool _inMacro = false;

public:
    // Disable copying/moving
    UndoStack(const UndoStack&) = delete;
    UndoStack(UndoStack&&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;
    UndoStack& operator=(UndoStack&&) = delete;

    ~UndoStack() = default;

public:
    UndoStack() = default;

    void addAction(std::unique_ptr<UndoAction>&& action);

    // Macros allow multiple actions to be preformed at once
    void startMacro();
    void endMacro();

    // Called once per frame, before updateSelection().
    // AbstractEditorGui* may be null
    void processEditorActions(AbstractEditorGui*);

    [[nodiscard]] bool canUndo() const { return !_undoStack.empty(); }
    [[nodiscard]] bool canRedo() const { return !_redoStack.empty(); }

    [[nodiscard]] bool hasPendingActions() const { return (!_pendingEditorActions.empty()) || (!_pendingProjectFileActions.empty()); }

    [[nodiscard]] bool isClean() const { return _clean; }
    void markClean() { _clean = true; }

private:
    // `processPendingProjectActions()`, `undo()` and `redo()` can only be called by
    // `processUndoStack()` to ensure the `AbstractEditorData` and `ProjectFile` data will be in sync.
    friend bool processUndoStack(AbstractEditorData*, AbstractEditorGui*, Project::ProjectFile&);

    [[nodiscard]] bool processPendingProjectActions(UnTech::Project::ProjectFile&);

    // AbstractEditorGui* may be null
    // Returns true if the editor data changed.
    [[nodiscard]] bool undo(UnTech::Project::ProjectFile&, AbstractEditorGui*);
    [[nodiscard]] bool redo(UnTech::Project::ProjectFile&, AbstractEditorGui*);
};

}
