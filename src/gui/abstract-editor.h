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

namespace UnTech::Project {
struct ProjectFile;
class ProjectData;
}

namespace UnTech::Gui {

class EditorUndoAction {
public:
    virtual ~EditorUndoAction() = default;

    // Returns true if the action modified the project
    // If false then the undo action will not be added to the stack
    virtual bool firstDo(UnTech::Project::ProjectFile&) = 0;

    virtual void undo(UnTech::Project::ProjectFile&) const = 0;
    virtual void redo(UnTech::Project::ProjectFile&) const = 0;
};

class AbstractEditorData {
public:
    constexpr static unsigned N_UNDO_ACTIONS = 200;

private:
    friend class AbstractExternalFileEditorData;
    ItemIndex _itemIndex;
    std::string _basename;

    // Using vector so I can trim the stacks when they get too large
    std::vector<std::unique_ptr<EditorUndoAction>> _pendingActions;
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
    const std::string& basename() const { return _basename; }

    // Return false if itemIndex is invalid.
    // If this editor is an `AbstractExternalFileEditor`, then you MUST call `setFilename` in this function.
    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) = 0;

    // Called after processGui and after an undo action has been processed.
    virtual void updateSelection() = 0;

    // Undo functions MUST NOT be called by an EditorUndoAction instance
    void addAction(std::unique_ptr<EditorUndoAction>&& action);

    // Macros allow multiple actions to be preformed at once
    void startMacro();
    void endMacro();

    // Returns true if the editor data changed.
    bool processPendingActions(UnTech::Project::ProjectFile&);
    bool undo(UnTech::Project::ProjectFile&);
    bool redo(UnTech::Project::ProjectFile&);

    bool canUndo() const { return !_undoStack.empty(); }
    bool canRedo() const { return !_redoStack.empty(); }

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

    // Called when the editor data is changed outside this GUI class.
    // (Note: also called after `setEditorData`)
    // Called after undo, redo, ImageCache invalidation or setEditorData
    virtual void editorDataChanged() = 0;

    virtual void editorOpened() = 0;
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