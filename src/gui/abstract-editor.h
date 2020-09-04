/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "item-index.h"
#include <filesystem>
#include <memory>
#include <vector>

namespace UnTech::Project {
struct ProjectFile;
}

namespace UnTech::Gui {

class EditorUndoAction {
public:
    virtual ~EditorUndoAction() = default;

    // ::TODO add text::

    // Returns true if the action modified the project
    // If false then the undo action will not be added to the stack
    virtual bool firstDo(UnTech::Project::ProjectFile&) = 0;

    virtual void undo(UnTech::Project::ProjectFile&) const = 0;
    virtual void redo(UnTech::Project::ProjectFile&) const = 0;
};

class AbstractEditor {
public:
    constexpr static unsigned N_UNDO_ACTIONS = 200;

private:
    friend class AbstractExternalFileEditor;
    ItemIndex _itemIndex;
    std::string _basename;

    // Using vector so I can trim the stacks when they get too large
    std::vector<std::unique_ptr<EditorUndoAction>> _pendingActions;
    std::vector<std::unique_ptr<EditorUndoAction>> _undoStack;
    std::vector<std::unique_ptr<EditorUndoAction>> _redoStack;

    bool _clean = true;

public:
    AbstractEditor(const ItemIndex itemIndex);
    virtual ~AbstractEditor() = default;

    AbstractEditor(const AbstractEditor&) = delete;
    AbstractEditor(AbstractEditor&&) = delete;
    AbstractEditor& operator=(const AbstractEditor&) = delete;
    AbstractEditor& operator=(AbstractEditor&&) = delete;

    ItemIndex itemIndex() const { return _itemIndex; }

    // The returned string is empty on internal projectFile resources.
    const std::string& basename() const { return _basename; }

    // Return false if itemIndex is invalid.
    // If this editor is an `AbstractExternalFileEditor`, then you MUST call `setFilename` in this function.
    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) = 0;

    virtual void editorOpened() = 0;
    virtual void editorClosed() = 0;

    // This is fine - only one Editor is active at any given time.
    virtual void processGui(const Project::ProjectFile& projectFile) = 0;

    // Called after processGui and after an undo action has been processed.
    virtual void updateSelection() = 0;

    // ::TODO add imageFileChanged(const std::filesystem::path&); ::

    // Undo functions MUST NOT be called by an EditorUndoAction instance
    void addAction(std::unique_ptr<EditorUndoAction>&& action);
    void processPendingActions(UnTech::Project::ProjectFile&);
    void undo(UnTech::Project::ProjectFile&);
    void redo(UnTech::Project::ProjectFile&);

    bool canUndo() const { return !_undoStack.empty(); }
    bool canRedo() const { return !_redoStack.empty(); }

    bool isClean() const { return _clean; }
    void markClean() { _clean = true; }
};

class AbstractExternalFileEditor : public AbstractEditor {
private:
    std::filesystem::path _filename;

public:
    AbstractExternalFileEditor(ItemIndex itemIndex)
        : AbstractEditor(itemIndex)
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

std::unique_ptr<AbstractEditor> createEditor(ItemIndex itemIndex, const UnTech::Project::ProjectFile&);

}
