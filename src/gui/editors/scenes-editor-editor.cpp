/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "scenes-editor-editor.h"
#include "gui/imgui.h"
#include "gui/list-helpers.h"
#include "models/resources/scene-bgmode.hpp"

namespace UnTech::Gui {

const char* bgModeItems[] = {
    "Mode 0",
    "Mode 1",
    "Mode 1 (bg3 priotity)",
    "Mode 2",
    "Mode 3",
    "Mode 4",
};

const char* layerTypeItems[] = {
    "None",
    "Background Image",
    "MetaTile Tileset",
    "Text Console",
};

ScenesEditor::ScenesEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
{
}

bool ScenesEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    _scenes = projectFile.resourceScenes;

    return true;
}

void ScenesEditor::commitPendingChanges(Project::ProjectFile& projectFile)
{
    projectFile.resourceScenes = _scenes;
}

void ScenesEditor::editorOpened()
{
}

void ScenesEditor::editorClosed()
{
}

void ScenesEditor::settingsWindow()
{
    if (ImGui::Begin("Scene Settings")) {
        ImGui::SetWindowSize(ImVec2(1000, 400), ImGuiCond_FirstUseEver);

        ListButtons(&_settingsSel, &_scenes.settings, 128);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(7);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("BG Mode");
        ImGui::NextColumn();
        ImGui::Text("Layer 0 Type");
        ImGui::NextColumn();
        ImGui::Text("Layer 1 Type");
        ImGui::NextColumn();
        ImGui::Text("Layer 2 Type");
        ImGui::NextColumn();
        ImGui::Text("Layer 3 Type");
        ImGui::NextColumn();
        ImGui::Separator();

        for (unsigned i = 0; i < _scenes.settings.size(); i++) {
            auto& sceneSettings = _scenes.settings.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_settingsSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &sceneSettings.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::EnumCombo("##BgMode", &sceneSettings.bgMode, bgModeItems, IM_ARRAYSIZE(bgModeItems));
            ImGui::NextColumn();

            static const std::array<const char*, 4> layerLabels = { "##LT0", "##LT1", "##LT2", "##LT3" };
            for (unsigned l = 0; l < sceneSettings.layerTypes.size(); l++) {
                ImGui::SetNextItemWidth(-1);
                edited |= ImGui::EnumCombo(layerLabels.at(l), &sceneSettings.layerTypes.at(l),
                                           layerTypeItems, IM_ARRAYSIZE(layerTypeItems));
                ImGui::NextColumn();
            }

            if (edited) {
                ImGui::LogText("Edited Secene Settings");
                this->pendingChanges = true;
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

bool ScenesEditor::sceneLayerCombo(const char* label, idstring* value,
                                   const Project::ProjectFile& projectFile, const Resources::SceneSettingsInput& sceneSettings, const unsigned layerId)
{
    using namespace std::string_literals;

    using LayerType = Resources::LayerType;

    const auto layer = sceneSettings.layerTypes.at(layerId);

    switch (layer) {
    case LayerType::None: {
        return false;
    }

    case LayerType::TextConsole: {
        ImGui::TextUnformatted("Text"s);
        return false;
    }

    case LayerType::BackgroundImage: {
        const unsigned bitDepth = Resources::bitDepthForLayer(sceneSettings.bgMode, layerId);

        bool e = ImGui::IdStringCombo(label, value, projectFile.backgroundImages, false,
                                      [&](auto& item) {
                                          return (item.bitDepth == bitDepth) ? &item.name : nullptr;
                                      });
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Background Image (%d bpp)", bitDepth);
        }
        return e;
    }

    case LayerType::MetaTileTileset: {
        const unsigned bitDepth = Resources::bitDepthForLayer(sceneSettings.bgMode, layerId);

        bool e = ImGui::IdStringCombo(label, value, projectFile.metaTileTilesets, false,
                                      [&](const MetaTiles::MetaTileTilesetInput* mt) {
                                          return (mt->animationFrames.bitDepth == bitDepth) ? &mt->name : nullptr;
                                      });
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("MetaTile Tileset (%d bpp)", bitDepth);
        }
        return e;
    }
    }

    return false;
}

void ScenesEditor::scenesWindow(const Project::ProjectFile& projectFile)
{
    if (ImGui::Begin("Scenes")) {
        ImGui::SetWindowSize(ImVec2(1000, 400), ImGuiCond_FirstUseEver);

        ListButtons(&_scenesSel, &_scenes.scenes, 128);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(7);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Scene Settings");
        ImGui::NextColumn();
        ImGui::Text("Layer 0");
        ImGui::NextColumn();
        ImGui::Text("Layer 1");
        ImGui::NextColumn();
        ImGui::Text("Layer 2");
        ImGui::NextColumn();
        ImGui::Text("Layer 3");
        ImGui::NextColumn();
        ImGui::Separator();

        for (unsigned i = 0; i < _scenes.scenes.size(); i++) {
            auto& scene = _scenes.scenes.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_scenesSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &scene.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::IdStringCombo("##SceneSettings", &scene.sceneSettings, _scenes.settings);
            ImGui::NextColumn();

            const auto sceneSettings = _scenes.settings.find(scene.sceneSettings);

            static const std::array<const char*, 4> layerLabels = { "##LT0", "##LT1", "##LT2", "##LT3" };
            for (unsigned l = 0; l < scene.layers.size(); l++) {
                if (sceneSettings) {
                    ImGui::SetNextItemWidth(-1);
                    edited |= sceneLayerCombo(layerLabels.at(l), &scene.layers.at(l), projectFile, *sceneSettings, l);
                }
                ImGui::NextColumn();
            }

            if (edited) {
                ImGui::LogText("Edited Secene");
                this->pendingChanges = true;
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void ScenesEditor::processGui(const Project::ProjectFile& projectFile)
{
    settingsWindow();
    scenesWindow(projectFile);

    UpdateSelection(&_settingsSel);
    UpdateSelection(&_scenesSel);
}
}
