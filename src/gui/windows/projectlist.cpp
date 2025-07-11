/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "projectlist.h"
#include "message-box.h"
#include "gui/abstract-editor.h"
#include "gui/imgui-filebrowser.h"
#include "gui/imgui.h"
#include "models/common/iterators.h"
#include "models/enums.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/project/compiler-status.h"
#include "models/project/project.h"
#include "models/rooms/rooms-serializer.h"
#include <functional>

namespace UnTech::Gui {

const char* const ProjectListWindow::windowTitle = "Project";
const char* const ProjectListWindow::confirmRemovePopupTitle = "Remove Project Resource?";

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

static idstring nameFromPath(const std::filesystem::path& fn)
{
    return idstring::fixup(fn.stem().u8string());
}

static unsigned addPalette(NamedList<UnTech::Resources::PaletteInput>& list, const std::filesystem::path& fn)
{
    list.emplace_back();
    auto& pal = list.back();

    pal.name = nameFromPath(fn);
    pal.paletteImageFilename = fn;

    return list.size() - 1;
}

static unsigned addBackgroundImage(NamedList<UnTech::Resources::BackgroundImageInput>& list, const std::filesystem::path& fn)
{
    list.emplace_back();
    auto& bi = list.back();

    bi.name = nameFromPath(fn);
    bi.imageFilename = fn;

    return list.size() - 1;
}

template <class T, typename SaveFunction>
static unsigned addExternalFile(ExternalFileList<T>& list, const std::filesystem::path& fn,
                                SaveFunction saveFunction)
{
    assert(!fn.empty());

    // Do not add resource if the file is used by the project
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const auto& i) { return i.filename == fn; });
    if (it != list.end()) {
        return std::distance(list.begin(), it);
    }

    if (std::filesystem::exists(fn) == false) {
        T item;
        item.name = nameFromPath(fn);
        saveFunction(item, fn);
    }

    const unsigned index = list.size();

    list.insert_back(fn);
    list.item(index).loadFile();

    return index;
}

template <class T, auto saveFunction>
static unsigned addFrameSet(std::vector<UnTech::MetaSprite::FrameSetFile>& list, const std::filesystem::path& fn)
{
    using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;

    constexpr bool isMetaSprite = std::is_same_v<T, MS::FrameSet>;

    assert(!fn.empty());

    // Do not add resource if the file is used by the project
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const auto& i) { return i.filename == fn; });
    if (it != list.end()) {
        return std::distance(list.begin(), it);
    }

    if (std::filesystem::exists(fn) == false) {
        T fs;
        fs.name = nameFromPath(fn);
        saveFunction(fs, fn);
    }

    const unsigned index = list.size();

    auto& item = list.emplace_back();
    item.type = isMetaSprite ? FrameSetType::METASPRITE : FrameSetType::SPRITE_IMPORTER;
    item.filename = fn;
    item.loadFile();

    return index;
}

struct AddResourceSettings {
    // cppcheck-suppress unusedStructMember
    const char8_t* menuTitle;
    const char8_t* dialogTitle;
    const char8_t* extension;
    bool createFile;
    ResourceType editorType;

    // Return value is the index of the newly created item
    // Allowed to throw an exception
    std::function<unsigned(Project::ProjectFile&, const std::filesystem::path&)> addResource;
};

static const std::array<AddResourceSettings, 7> addResourceSettings{
    {
        { u8"Add FrameSet Export Order", u8"New FrameSet Export Order", u8".utfseo", true,
          ResourceType::FrameSetExportOrders,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addExternalFile(pf.frameSetExportOrders, fn, &UnTech::MetaSprite::saveFrameSetExportOrder);
          } },
        { u8"Add MetaSprite FrameSet", u8"New MetaSprite FrameSet", u8".utms", true,
          ResourceType::FrameSets,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addFrameSet<MS::FrameSet, &MS::saveFrameSet>(pf.frameSets, fn);
          } },
        { u8"Add Sprite Importer FrameSet", u8"New Sprite Importer FrameSet", u8".utsi", true,
          ResourceType::FrameSets,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addFrameSet<SI::FrameSet, &SI::saveFrameSet>(pf.frameSets, fn);
          } },
        { u8"Add Palette", u8"Select Palette Image", u8".png", false,
          ResourceType::Palettes,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addPalette(pf.palettes, fn);
          } },
        { u8"Add Background Image", u8"Select Background Image", u8".png", false,
          ResourceType::BackgroundImages,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addBackgroundImage(pf.backgroundImages, fn);
          } },
        { u8"Add MetaTile Tileset", u8"New MetaTile Tileset", u8".utmt", true,
          ResourceType::MataTileTilesets,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addExternalFile(pf.metaTileTilesets, fn, &UnTech::MetaTiles::saveMetaTileTilesetInput);
          } },
        { u8"Add Room", u8"New Room", u8".utroom", true,
          ResourceType::Rooms,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addExternalFile(pf.rooms, fn, &UnTech::Rooms::saveRoomInput);
          } },
    }
};

bool ProjectListWindow::canRemoveSelectedIndex() const
{
    return _selectedIndex && _selectedIndex->type != ResourceType::ProjectSettings;
}

static void resourceStateIcon(UnTech::Project::ResourceState state)
{
    using RS = UnTech::Project::ResourceState;

    switch (state) {
    case RS::AllUnchecked:
    case RS::Unchecked:
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::TextUnformatted(u8"·");
        break;

    case RS::Valid:
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::TextUnformatted(u8"·");
        break;

    case RS::Invalid:
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::TextUnformatted(u8"X");
        break;

    case RS::Missing:
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::TextUnformatted(u8"M");
        break;

    case RS::DependencyError:
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::TextUnformatted(u8"D");
        break;
    }

    ImGui::PopStyleColor();
}

void ProjectListWindow::projectListWindow(const UnTech::Project::CompilerStatus& status)
{
    using namespace std::string_literals;

    const ImGuiSelectableFlags leafFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;

    std::optional<ItemIndex> pendingIndex = _selectedIndex;

    status.resourceLists().read([&](const auto& resourceLists) {
        for (const auto [rtIndex, rList] : enumerate(resourceLists)) {

            assert(resourceLists.size() == N_RESOURCE_TYPES);
            const auto type = static_cast<ResourceType>(rtIndex);

            assert(rList.resources.size() < INT_MAX);

            resourceStateIcon(rList.state);
            ImGui::SameLine();
            ImGui::TextUnformatted(rList.typeNamePlural);

            ImGui::PushID(int(type));
            ImGui::Indent();

            for (auto [index, item] : enumerate(rList.resources)) {
                const ItemIndex itemIndex{ type, unsigned(index) };

                ImGui::PushID(index);

                if (ImGui::Selectable("##sel", _selectedIndex == itemIndex, leafFlags)) {
                    pendingIndex = itemIndex;
                    _state = State::SELECT_RESOURCE;
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1)) {
                    // Ensure item is selected when opening a context menu
                    pendingIndex = itemIndex;
                }

                ImGui::SameLine();

                resourceStateIcon(item.state);
                ImGui::SameLine();

                ImGui::TextUnformatted(item.name);

                ImGui::PopID();
            }

            ImGui::Unindent();
            ImGui::PopID();
        }
    });

    _selectedIndex = pendingIndex;

    if (ImGui::BeginPopupContextWindow()) {
        processMenu();
        ImGui::EndPopup();
    }
}

void ProjectListWindow::processMenu()
{
    if (ImGui::BeginMenu("Add Resource")) {
        for (auto [i, s] : enumerate(addResourceSettings)) {
            ImGui::PushID(i);
            if (ImGui::MenuItem(u8Cast(s.menuTitle))) {
                _state = State::ADD_RESOURCE_DIALOG;
                _addMenuIndex = i;
            }
            ImGui::PopID();
        }
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Remove Resource", nullptr, false, canRemoveSelectedIndex())) {
        _state = State::REMOVE_RESOURCE_INIT;
    }

    // ::TODO add Restore removed resource to menu::
}

void ProjectListWindow::addResourceDialog()
{
    if (_state == State::ADD_RESOURCE_DIALOG) {
        if (_addMenuIndex >= addResourceSettings.size()) {
            _state = State::SELECT_RESOURCE;
            return;
        }

        const auto& settings = addResourceSettings.at(_addMenuIndex);

        const auto d = settings.createFile ? ImGui::SaveFileDialog("AddResource", settings.dialogTitle, settings.extension)
                                           : ImGui::OpenFileDialog("AddResource", settings.dialogTitle, settings.extension);
        const auto& closed = d.first;
        const auto& fn = d.second;
        if (closed) {
            if (fn) {
                _addResourceFilename = fn.value();
                _state = State::ADD_RESOURCE_CONFIRMED;
            }
            else {
                _state = State::SELECT_RESOURCE;
            }
        }
    }
}

void ProjectListWindow::confirmRemovePopup()
{
    if (_state == State::REMOVE_RESOURCE_INIT) {
        if (canRemoveSelectedIndex()) {
            ImGui::OpenPopup(confirmRemovePopupTitle);
        }
        _state = State::REMOVE_RESOURCE_POPUP_OPEN;
    }

    if (ImGui::BeginPopupModal(confirmRemovePopupTitle)) {
        if (_selectedIndex == std::nullopt || _state != State::REMOVE_RESOURCE_POPUP_OPEN) {
            ImGui::CloseCurrentPopup();
            return;
        }

        // ::TODO include name of item (probably from AbstractEditor)::
        ImGui::TextUnformatted(u8"Are you sure you want to remove this resource?");

        // ::TODO only show this line if a backup cannot be made (and change colour to red)::
        ImGui::TextUnformatted(u8"This action cannot be undone.");

        if (ImGui::Button("Yes")) {
            _state = State::REMOVE_RESOURCE_CONFIRMED;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ProjectListWindow::processGui(const UnTech::Project::CompilerStatus& status)
{
    projectListWindow(status);

    addResourceDialog();
    confirmRemovePopup();
}

void ProjectListWindow::processPendingActions(Project::ProjectFile& projectFile,
                                              std::vector<gsl::not_null<std::shared_ptr<AbstractEditorData>>>& editors)
{
    switch (_state) {
    case State::SELECT_RESOURCE:
    case State::ADD_RESOURCE_DIALOG:
    case State::REMOVE_RESOURCE_INIT:
    case State::REMOVE_RESOURCE_POPUP_OPEN:
        break;

    case State::ADD_RESOURCE_CONFIRMED:
        addResource(projectFile);
        _clean = false;
        break;

    case State::REMOVE_RESOURCE_CONFIRMED:
        removeResource(projectFile, editors);
        _clean = false;
        break;
    }
}

void ProjectListWindow::addResource(Project::ProjectFile& projectFile)
{
    using namespace std::string_literals;

    assert(_state == State::ADD_RESOURCE_CONFIRMED);

    _state = State::SELECT_RESOURCE;

    if (_addMenuIndex < addResourceSettings.size()) {
        try {
            const auto& settings = addResourceSettings.at(_addMenuIndex);

            unsigned i = settings.addResource(projectFile, _addResourceFilename);
            _selectedIndex = ItemIndex{ settings.editorType, i };
        }
        catch (const std::exception& ex) {
            MsgBox::showMessage(u8"Cannot Create Resource"s, ex.what());
        }
    }
}

void ProjectListWindow::removeResource(Project::ProjectFile& projectFile,
                                       std::vector<gsl::not_null<std::shared_ptr<AbstractEditorData>>>& editors)
{
    if (!_selectedIndex
        || _selectedIndex->type == ResourceType::ProjectSettings) {

        _state = State::SELECT_RESOURCE;
        return;
    }

    // Backup old data
    {
        auto it = std::find_if(editors.begin(), editors.end(),
                               [&](auto& e) { return e->itemIndex() == *_selectedIndex; });
        if (it != editors.end()) {
            _removedEditors.push_back(std::move(*it));
            editors.erase(it);
        }
        else {
            // Editor does not exist, usually because an external file could not be loaded.
            // No backup is saved.
        }
    }

    // Remove from project
    {
        auto remove = [&](auto& list) {
            if (_selectedIndex->index < list.size()) {
                list.erase(list.begin() + _selectedIndex->index);
            }
        };

        auto removeExternalFile = [&](auto& list) {
            if (_selectedIndex->index < list.size()) {
                list.remove(_selectedIndex->index);
            }
        };

        switch (_selectedIndex->type) {
        case ResourceType::ProjectSettings:
            // Not allowed to delete project settings
            abort();
            break;

        case ResourceType::FrameSetExportOrders:
            removeExternalFile(projectFile.frameSetExportOrders);
            break;

        case ResourceType::FrameSets:
            remove(projectFile.frameSets);
            break;

        case ResourceType::Palettes:
            remove(projectFile.palettes);
            break;

        case ResourceType::BackgroundImages:
            remove(projectFile.backgroundImages);
            break;

        case ResourceType::MataTileTilesets:
            removeExternalFile(projectFile.metaTileTilesets);
            break;

        case ResourceType::Rooms:
            removeExternalFile(projectFile.rooms);
            break;
        }
    }

    // Update indexes
    for (auto& e : editors) {
        if (e->itemIndex().type == _selectedIndex->type) {
            auto itemIndex = e->itemIndex();
            if (itemIndex.index > _selectedIndex->index) {
                itemIndex.index--;
                e->setItemIndex(itemIndex);
            }
        }
    }

    _selectedIndex = std::nullopt;
    _state = State::SELECT_RESOURCE;
}

}
