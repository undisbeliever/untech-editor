/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "item-index.h"
#include "windows/projectlist.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace UnTech::Project {
struct ProjectFile;
}

namespace UnTech::Gui {
class AbstractEditor;

class UnTechEditor {
private:
    static std::shared_ptr<UnTechEditor> _instance;

    // ::TODO put behind a mutex in a separate class::
    std::unique_ptr<UnTech::Project::ProjectFile> const _projectFile;
    const std::filesystem::path _filename;
    const std::string _basename;

    std::vector<std::unique_ptr<AbstractEditor>> _editors;
    AbstractEditor* _currentEditor;

    ProjectListWindow _projectListWindow;

private:
    UnTechEditor(std::unique_ptr<UnTech::Project::ProjectFile>&& pf,
                 const std::filesystem::path& fn);

public:
    // May be null
    static std::shared_ptr<UnTechEditor> instance() { return _instance; }

    // Only one project can be loaded per exectable.
    static void newProject(const std::filesystem::path& filename);
    static void loadProject(const std::filesystem::path& filename);

    std::optional<ItemIndex> selectedItemIndex() const;

    void processGui();

    // called after ImGUI render
    void updateProjectFile();

private:
    void openEditor(const ItemIndex itemIndex);
    void closeEditor();

    void saveProjectFile();
    void saveEditor(AbstractEditor* editor);
    void saveAll();

    void processMenu();
};

}
