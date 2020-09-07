/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "projectlist.h"
#include "message-box.h"
#include "gui/abstract-editor.h"
#include "gui/enums.h"
#include "gui/imgui-filebrowser.h"
#include "gui/imgui.h"
#include "gui/untech-editor.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/rooms/rooms-serializer.h"
#include <functional>

namespace UnTech::Gui {

const char* const ProjectListWindow::windowTitle = "Project";
const char* const ProjectListWindow::confirmRemovePopupTitle = "Remove Project Resource?";

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

static idstring nameFromPath(const std::filesystem::path& fn)
{
    return idstring::fixup(fn.stem().string());
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
                           [&](auto& i) { return i.filename == fn; });
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
                           [&](auto& i) { return i.filename == fn; });
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
    const char* const menuTitle;
    const char* const dialogTitle;
    const char* const extension;
    bool createFile;
    EditorType editorType;

    // Return value is the index of the newly created item
    // Allowed to throw an exception
    std::function<unsigned(Project::ProjectFile&, const std::filesystem::path&)> addResource;
};

static const std::array<AddResourceSettings, 7> addResourceSettings{
    {
        { "Add FrameSet Export Order", "New FrameSet Export Order", ".utfseo", true,
          EditorType::FrameSetExportOrders,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addExternalFile(pf.frameSetExportOrders, fn, &UnTech::MetaSprite::saveFrameSetExportOrder);
          } },
        { "Add MetaSprite FrameSet", "New MetaSprite FrameSet", ".utms", true,
          EditorType::FrameSets,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addFrameSet<MS::FrameSet, &MS::saveFrameSet>(pf.frameSets, fn);
          } },
        { "Add Sprite Importer FrameSet", "New Sprite Importer FrameSet", ".utsi", true,
          EditorType::FrameSets,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addFrameSet<SI::FrameSet, &SI::saveFrameSet>(pf.frameSets, fn);
          } },
        { "Add Palette", "Select Palette Image", ".png", false,
          EditorType::Palettes,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addPalette(pf.palettes, fn);
          } },
        { "Add Background Image", "Select Background Image", ".png", false,
          EditorType::BackgroundImages,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addBackgroundImage(pf.backgroundImages, fn);
          } },
        { "Add MetaTile Tileset", "New MetaTile Tileset", ".utmt", true,
          EditorType::MataTileTilesets,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addExternalFile(pf.metaTileTilesets, fn, &UnTech::MetaTiles::saveMetaTileTilesetInput);
          } },
        { "Add Room", "New Room", ".utroom", true,
          EditorType::Rooms,
          [](Project::ProjectFile& pf, const std::filesystem::path& fn) {
              return addExternalFile(pf.rooms, fn, &UnTech::Rooms::saveRoomInput);
          } },
    }
};

bool ProjectListWindow::canRemoveSelectedIndex() const
{
    return _selectedIndex && _selectedIndex->type != EditorType::ProjectSettings;
}

void ProjectListWindow::projectListWindow(const UnTech::Project::ProjectFile& pf)
{
    using namespace std::string_literals;

    if (ImGui::Begin(windowTitle)) {
        const ImGuiSelectableFlags leafFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;

        std::optional<ItemIndex> pendingIndex = _selectedIndex;

        auto leaf = [&](EditorType type, unsigned index, const std::string& label) {
            const ItemIndex itemIndex{ type, index };

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
            // ::TODO replace Bullet with status symbol::
            ImGui::Bullet();
            ImGui::TextUnformatted(label);

            ImGui::PopID();
        };

        auto namedList = [&](EditorType type, const auto& list, const char* treeLabel) {
            if (ImGui::TreeNodeEx(treeLabel, ImGuiTreeNodeFlags_DefaultOpen)) {
                assert(list.size() < INT_MAX);
                for (unsigned index = 0; index < list.size(); index++) {
                    const auto& item = list.at(index);
                    if (item.name.isValid()) {
                        leaf(type, index, item.name);
                    }
                    else {
                        std::string id = std::to_string(index);
                        leaf(type, index, id);
                    }
                }
                ImGui::TreePop();
            }
        };

        auto externalFileList = [&](EditorType type, const auto& list, const char* treeLabel) {
            if (ImGui::TreeNodeEx(treeLabel, ImGuiTreeNodeFlags_DefaultOpen)) {
                assert(list.size() < INT_MAX);
                for (unsigned index = 0; index < list.size(); index++) {
                    const auto& efi = list.item(index);
                    if (efi.value && efi.value->name.isValid()) {
                        leaf(type, index, efi.value->name);
                    }
                    else {
                        leaf(type, index, efi.filename.filename());
                    }
                }
                ImGui::TreePop();
            }
        };

        auto frameSetFiles = [&](EditorType type, const auto& list, const char* treeLabel) {
            if (ImGui::TreeNodeEx(treeLabel, ImGuiTreeNodeFlags_DefaultOpen)) {
                assert(list.size() < INT_MAX);
                for (unsigned index = 0; index < list.size(); index++) {
                    const UnTech::MetaSprite::FrameSetFile& fsf = list.at(index);

                    auto& name = fsf.name();
                    if (name.isValid()) {
                        leaf(type, index, name);
                    }
                    else {
                        leaf(type, index, fsf.filename.filename());
                    }
                }
                ImGui::TreePop();
            }
        };

        if (ImGui::TreeNodeEx("Project Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            leaf(EditorType::ProjectSettings, unsigned(ProjectSettingsIndex::ProjectSettings),
                 "Project Settings"s);
            leaf(EditorType::ProjectSettings, unsigned(ProjectSettingsIndex::InteractiveTiles),
                 "Interactive Tiles"s);
            leaf(EditorType::ProjectSettings, unsigned(ProjectSettingsIndex::ActionPoints),
                 "Action Points"s);
            leaf(EditorType::ProjectSettings, unsigned(ProjectSettingsIndex::EntityRomData),
                 "Entities"s);
            leaf(EditorType::ProjectSettings, unsigned(ProjectSettingsIndex::Scenes),
                 "Scenes"s);

            ImGui::TreePop();
        }

        externalFileList(EditorType::FrameSetExportOrders, pf.frameSetExportOrders,
                         "FrameSet Export Orders");

        frameSetFiles(EditorType::FrameSets, pf.frameSets,
                      "FrameSets");

        namedList(EditorType::Palettes, pf.palettes,
                  "Palettes");

        namedList(EditorType::BackgroundImages, pf.backgroundImages,
                  "Background Images");

        externalFileList(EditorType::MataTileTilesets, pf.metaTileTilesets,
                         "MetaTile Tilesets");

        externalFileList(EditorType::Rooms, pf.rooms,
                         "Rooms");

        _selectedIndex = pendingIndex;

        if (ImGui::BeginPopupContextWindow()) {
            processMenu();
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

void ProjectListWindow::processMenu()
{
    if (ImGui::BeginMenu("Add Resource")) {
        for (unsigned i = 0; i < addResourceSettings.size(); i++) {
            const auto& s = addResourceSettings.at(i);
            ImGui::PushID(i);
            if (ImGui::MenuItem(s.menuTitle)) {
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

        const auto [dialogClosed, fn] = settings.createFile ? ImGui::SaveFileDialog("AddResource", settings.dialogTitle, settings.extension)
                                                            : ImGui::OpenFileDialog("AddResource", settings.dialogTitle, settings.extension);
        if (dialogClosed) {
            if (fn) {
                _addResourceFilename = *fn;
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
        ImGui::TextUnformatted("Are you sure you want to remove this resource?");

        // ::TODO only show this line if a backup cannot be made (and change colour to red)::
        ImGui::TextUnformatted("This action cannot be undone.");

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

void ProjectListWindow::processGui(const UnTech::Project::ProjectFile& projectFile)
{
    projectListWindow(projectFile);

    addResourceDialog();
    confirmRemovePopup();
}

void ProjectListWindow::processPendingActions(Project::ProjectFile& projectFile,
                                              std::vector<std::unique_ptr<AbstractEditor>>& editors)
{
    switch (_state) {
    case State::SELECT_RESOURCE:
    case State::ADD_RESOURCE_DIALOG:
    case State::REMOVE_RESOURCE_INIT:
    case State::REMOVE_RESOURCE_POPUP_OPEN:
        break;

    case State::ADD_RESOURCE_CONFIRMED:
        addResource(projectFile);
        break;

    case State::REMOVE_RESOURCE_CONFIRMED:
        removeResource(projectFile, editors);
        break;
    }
}

void ProjectListWindow::addResource(Project::ProjectFile& projectFile)
{
    assert(_state == State::ADD_RESOURCE_CONFIRMED);

    _state = State::SELECT_RESOURCE;

    if (_addMenuIndex < addResourceSettings.size()) {
        try {
            const auto& settings = addResourceSettings.at(_addMenuIndex);

            unsigned i = settings.addResource(projectFile, _addResourceFilename);
            _selectedIndex = ItemIndex{ settings.editorType, i };
        }
        catch (const std::exception& ex) {
            MessageBox::showMessage("Cannot Create Resource", ex.what());
        }
    }
}

void ProjectListWindow::removeResource(Project::ProjectFile& projectFile, std::vector<std::unique_ptr<AbstractEditor>>& editors)
{
    if (_selectedIndex == std::nullopt
        || _selectedIndex->type == EditorType::ProjectSettings) {

        _state = State::SELECT_RESOURCE;
        return;
    }

    // Backup old data
    {
        auto it = std::find_if(editors.begin(), editors.end(),
                               [&](auto& e) { assert(e); return e->itemIndex() == *_selectedIndex; });
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
        case EditorType::ProjectSettings:
            // Not allowed to delete project settings
            abort();
            break;

        case EditorType::FrameSetExportOrders:
            removeExternalFile(projectFile.frameSetExportOrders);
            break;

        case EditorType::FrameSets:
            remove(projectFile.frameSets);
            break;

        case EditorType::Palettes:
            remove(projectFile.palettes);
            break;

        case EditorType::BackgroundImages:
            remove(projectFile.backgroundImages);
            break;

        case EditorType::MataTileTilesets:
            removeExternalFile(projectFile.metaTileTilesets);
            break;

        case EditorType::Rooms:
            removeExternalFile(projectFile.rooms);
            break;
        }
    }

    // Update indexes
    for (auto& e : editors) {
        assert(e);
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
