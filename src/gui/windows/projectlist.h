/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/item-index.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

namespace UnTech::Project {
class ProjectData;
struct ProjectFile;
}

namespace UnTech::Gui {
class AbstractEditorData;

class ProjectListWindow {
    static const char* const windowTitle;
    static const char* const confirmRemovePopupTitle;

    enum class State {
        SELECT_RESOURCE,

        ADD_RESOURCE_DIALOG,
        ADD_RESOURCE_CONFIRMED,

        REMOVE_RESOURCE_INIT,
        REMOVE_RESOURCE_POPUP_OPEN,
        REMOVE_RESOURCE_CONFIRMED,
    };

private:
    State _state;
    std::optional<ItemIndex> _selectedIndex;
    std::vector<std::unique_ptr<AbstractEditorData>> _removedEditors;

    // Selected index in the "Add Resource" menu
    unsigned _addMenuIndex;

    // Filename of the new resource to add
    std::filesystem::path _addResourceFilename;

    bool _clean = true;

public:
    const std::optional<ItemIndex>& selectedIndex() const { return _selectedIndex; }

    bool isClean() const { return _clean; }
    void markClean() { _clean = true; }

    void processMenu();
    void processGui(const Project::ProjectData& projectData);

    bool hasPendingActions() const
    {
        return _state == State::ADD_RESOURCE_CONFIRMED || _state == State::REMOVE_RESOURCE_CONFIRMED;
    }
    void processPendingActions(Project::ProjectFile& projectFile, std::vector<std::unique_ptr<AbstractEditorData>>& editors);

private:
    void projectListWindow(const Project::ProjectData& projectData);

    void addResourceDialog();
    void confirmRemovePopup();

    void addResource(UnTech::Project::ProjectFile& projectFile);

    bool canRemoveSelectedIndex() const;
    void removeResource(Project::ProjectFile& projectFile, std::vector<std::unique_ptr<AbstractEditorData>>& editors);
};

}
