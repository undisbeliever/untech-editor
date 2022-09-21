/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "item-index.h"
#include <filesystem>
#include <memory>
#include <vector>

namespace UnTech {
class AbstractError;
}

namespace UnTech::Project {
struct ProjectFile;
class ProjectData;
}

namespace UnTech::Gui {

class AbstractEditorGui;

class EditorUndoAction {
public:
    virtual ~EditorUndoAction() = default;

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
    virtual bool firstDo_projectFile(UnTech::Project::ProjectFile&) = 0;

    // Undoes an action.
    // This function MUST update both editor and project data.
    // This function should update the selection if the list size or order is changed.
    virtual void undo(UnTech::Project::ProjectFile&) const = 0;

    // Redoes an undone action.
    // This function MUST update both editor and project data.
    // This function should update the selection if the list size or order is changed.
    virtual void redo(UnTech::Project::ProjectFile&) const = 0;
};

class AbstractEditorData {
public:
    constexpr static unsigned N_UNDO_ACTIONS = 200;

private:
    friend class AbstractExternalFileEditorData;
    ItemIndex _itemIndex;
    std::u8string _basename;

    std::vector<std::unique_ptr<EditorUndoAction>> _pendingEditorActions;
    std::vector<std::unique_ptr<EditorUndoAction>> _pendingProjectFileActions;

    // Using vector so I can trim the stacks when they get too large
    std::vector<std::unique_ptr<EditorUndoAction>> _undoStack;
    std::vector<std::unique_ptr<EditorUndoAction>> _redoStack;

    bool _clean = true;
    bool _inMacro;

private:
    AbstractEditorData(const AbstractEditorData&) = delete;
    AbstractEditorData(AbstractEditorData&&) = delete;
    AbstractEditorData& operator=(const AbstractEditorData&) = delete;
    AbstractEditorData& operator=(AbstractEditorData&&) = delete;

public:
    AbstractEditorData(const ItemIndex itemIndex);
    virtual ~AbstractEditorData() = default;

    ItemIndex itemIndex() const { return _itemIndex; }

    // The returned string is empty on internal projectFile resources.
    const std::u8string& basename() const { return _basename; }

    // Return false if itemIndex is invalid.
    // If this editor is an `AbstractExternalFileEditor`, then you MUST call `setFilename` in this function.
    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) = 0;

    // Called when a an error list item is double clicked
    virtual void errorDoubleClicked(const AbstractError*) = 0;

    // Called after processGui and after an undo action has been processed.
    virtual void updateSelection() = 0;

    // Undo functions MUST NOT be called by an EditorUndoAction instance
    void addAction(std::unique_ptr<EditorUndoAction>&& action);

    // Macros allow multiple actions to be preformed at once
    void startMacro();
    void endMacro();

    // Called once per frame, before updateSelection().
    // AbstractEditorGui* may be null
    void processEditorActions(AbstractEditorGui*);

    // AbstractEditorGui* may be null
    // Returns true if the editor data changed.
    bool processPendingProjectActions(UnTech::Project::ProjectFile&);
    bool undo(UnTech::Project::ProjectFile&, AbstractEditorGui*);
    bool redo(UnTech::Project::ProjectFile&, AbstractEditorGui*);

    bool canUndo() const { return !_undoStack.empty(); }
    bool canRedo() const { return !_redoStack.empty(); }

    bool hasPendingActions() const { return (!_pendingEditorActions.empty()) || (!_pendingProjectFileActions.empty()); }

    bool isClean() const { return _clean; }
    void markClean() { _clean = true; }

private:
    friend class ProjectListWindow;
    void setItemIndex(const ItemIndex i) { _itemIndex = i; }
};

class AbstractExternalFileEditorData : public AbstractEditorData {
private:
    std::filesystem::path _filename;

public:
    AbstractExternalFileEditorData(ItemIndex itemIndex)
        : AbstractEditorData(itemIndex)
        , _filename()
    {
    }

    const std::filesystem::path& filename() const { return _filename; }

    // May throw an exception
    virtual void saveFile() const = 0;

protected:
    // To be called in `loadDataFromProject` if this editor an AbstractExternalFileEditor
    // Must never be called on a
    void setFilename(const std::filesystem::path& fn);
};

class AbstractEditorGui {
private:
    AbstractEditorGui(const AbstractEditorGui&) = delete;
    AbstractEditorGui(AbstractEditorGui&&) = delete;
    AbstractEditorGui& operator=(const AbstractEditorGui&) = delete;
    AbstractEditorGui& operator=(AbstractEditorGui&&) = delete;

public:
    bool undoClicked = false;
    bool redoClicked = false;

public:
    AbstractEditorGui() = default;
    virtual ~AbstractEditorGui() = default;

    virtual bool setEditorData(AbstractEditorData* data) = 0;

    // Called after setEditorData or ImageCache invalidation
    virtual void resetState() = 0;

    virtual void editorClosed() = 0;

    virtual void processGui(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData) = 0;

    virtual void viewMenu();

protected:
    void undoStackButtons();
};

// Returns true if the editor data was changed.
bool processUndoStack(AbstractEditorGui* gui, AbstractEditorData* editor, UnTech::Project::ProjectFile&);

std::unique_ptr<AbstractEditorData> createEditor(ItemIndex itemIndex, const UnTech::Project::ProjectFile&);

std::vector<std::unique_ptr<AbstractEditorGui>> createEditorGuis();

}
