/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/item-index.h"
#include "models/project/project.h"

namespace UnTech::Gui {
class AbstractEditor;

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
    std::vector<std::unique_ptr<AbstractEditor>> _removedEditors;

    // Selected index in the "Add Resource" menu
    unsigned _addMenuIndex;

    // Filename of the new resource to add
    std::filesystem::path _addResourceFilename;

    bool _clean = true;

public:
    const std::optional<ItemIndex>& selectedIndex() const { return _selectedIndex; }

    bool hasPendingActions() { return _state != State::SELECT_RESOURCE; }

    bool isClean() const { return _clean; }
    void markClean() { _clean = true; }

    void processMenu();
    void processGui(const UnTech::Project::ProjectFile& projectFile);

    void processPendingActions(Project::ProjectFile& projectFile, std::vector<std::unique_ptr<AbstractEditor>>& editors);

private:
    void projectListWindow(const UnTech::Project::ProjectFile& projectFile);

    void addResourceDialog();
    void confirmRemovePopup();

    void addResource(UnTech::Project::ProjectFile& projectFile);

    bool canRemoveSelectedIndex() const;
    void removeResource(Project::ProjectFile& projectFile, std::vector<std::unique_ptr<AbstractEditor>>& editors);
};

}
