/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace UnTech::Project {
struct ProjectFile;
}

namespace UnTech::Gui {
enum class EditorType : unsigned;

struct ItemIndex {
    EditorType type;
    unsigned index;

    ItemIndex(EditorType t, unsigned i)
        : type(t)
        , index(i)
    {
    }

    bool operator==(const ItemIndex& o) const
    {
        return this->type == o.type
               && this->index == o.index;
    }
};

class UnTechEditor {
private:
    static std::shared_ptr<UnTechEditor> _instance;

    // ::TODO put behind a mutex in a separate class::
    std::unique_ptr<UnTech::Project::ProjectFile> const _projectFile;

    std::optional<ItemIndex> _selectedItem;

private:
    UnTechEditor();
    UnTechEditor(std::unique_ptr<UnTech::Project::ProjectFile>&& pf);

public:
    // May be null
    static std::shared_ptr<UnTechEditor> instance() { return _instance; }

    // Only one project can be loaded per exectable.
    static void newProject();
    static void loadProject(std::filesystem::path filename);

    // ::TODO put behind a mutex::
    const UnTech::Project::ProjectFile& projectFile() const { return *_projectFile; }

    std::optional<ItemIndex> selectedItem() const { return _selectedItem; }

    void openEditor(EditorType type, unsigned item);
    void closeEditor();

    void processGui();

    // called after ImGUI render
    void updateProjectFile();
};

}
