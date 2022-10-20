/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "item-index.h"
#include "undostack.h"
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

class AbstractEditorData {
public:
    constexpr static unsigned N_UNDO_ACTIONS = 200;

private:
    friend class AbstractExternalFileEditorData;
    ItemIndex _itemIndex;
    std::u8string _basename;

    UndoStack _undoStack;

public:
    AbstractEditorData(const AbstractEditorData&) = delete;
    AbstractEditorData(AbstractEditorData&&) = delete;
    AbstractEditorData& operator=(const AbstractEditorData&) = delete;
    AbstractEditorData& operator=(AbstractEditorData&&) = delete;

public:
    AbstractEditorData(const ItemIndex itemIndex)
        : _itemIndex{ itemIndex }
        , _basename{}
        , _undoStack{}
    {
    }

    virtual ~AbstractEditorData() = default;

    [[nodiscard]] ItemIndex itemIndex() const { return _itemIndex; }

    // The returned string is empty on internal projectFile resources.
    [[nodiscard]] const std::u8string& basename() const { return _basename; }

    [[nodiscard]] UndoStack& undoStack() { return _undoStack; }

    // Return false if itemIndex is invalid.
    // If this editor is an `AbstractExternalFileEditor`, then you MUST call `setFilename` in this function.
    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) = 0;

    // Called when an error list item is double clicked
    virtual void errorDoubleClicked(const AbstractError*) = 0;

    // Called at the end of `UnTechEditor::processGui()` and `processUndoStack()`
    virtual void updateSelection() = 0;

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

    [[nodiscard]] const std::filesystem::path& filename() const { return _filename; }

    // May throw an exception
    virtual void saveFile() const = 0;

protected:
    // To be called in `loadDataFromProject` if this editor an AbstractExternalFileEditor
    // Must never be called on a
    void setFilename(const std::filesystem::path& fn);
};

class AbstractEditorGui {
public:
    // str_id of the child window inside the BG window
    const char* const childWindowStrId;

    bool undoClicked = false;
    bool redoClicked = false;

public:
    AbstractEditorGui(const AbstractEditorGui&) = delete;
    AbstractEditorGui(AbstractEditorGui&&) = delete;
    AbstractEditorGui& operator=(const AbstractEditorGui&) = delete;
    AbstractEditorGui& operator=(AbstractEditorGui&&) = delete;

public:
    AbstractEditorGui(const char* strId)
        : childWindowStrId(strId)

    {
    }
    virtual ~AbstractEditorGui() = default;

    virtual bool setEditorData(AbstractEditorData* data) = 0;

    // Called after setEditorData or ImageCache invalidation
    virtual void resetState() = 0;

    virtual void editorClosed() = 0;

    virtual void processGui(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData) = 0;
    virtual void processExtraWindows(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData);

    virtual void viewMenu();

protected:
    void undoStackButtons();
};

std::unique_ptr<AbstractEditorData> createEditor(ItemIndex itemIndex, const UnTech::Project::ProjectFile&);

std::vector<std::unique_ptr<AbstractEditorGui>> createEditorGuis();

}
