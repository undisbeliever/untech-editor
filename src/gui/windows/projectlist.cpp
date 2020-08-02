/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "projectlist.h"
#include "gui/enums.h"
#include "gui/imgui.h"
#include "gui/untech-editor.h"
#include "models/project/project.h"

namespace UnTech::Gui {

const char* ProjectListWindow::windowTitle = "Project";

void ProjectListWindow::processGui(UnTechEditor& editor)
{
    using namespace std::string_literals;

    if (ImGui::Begin(windowTitle)) {
        const ImGuiSelectableFlags leafFlags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;

        const auto currentIndex = editor.selectedItemIndex();

        auto leaf = [&](EditorType type, unsigned index, const std::string& label) {
            bool selected = currentIndex == ItemIndex(type, index);
            ImGui::PushID(index);

            if (ImGui::Selectable("##sel", selected, leafFlags)) {
                editor.openEditor(type, index);
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

        const auto& pf = editor.projectFile();

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
    }

    ImGui::End();
}

}
