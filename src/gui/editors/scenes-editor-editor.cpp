/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "scenes-editor-editor.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "models/resources/scene-bgmode.hpp"

namespace UnTech::Gui {

// ScenesEditor Action Policies
struct ScenesEditorData::AP {
    struct ResourceScenes {
        using EditorT = ScenesEditorData;
        using EditorDataT = UnTech::Resources::ResourceScenes;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.scenes;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.resourceScenes;
        }
    };

    struct SceneSettings : public ResourceScenes {
        using ListT = NamedList<Resources::SceneSettingsInput>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::Resources::MAX_N_SCENE_SETTINGS;

        constexpr static auto SelectionPtr = &EditorT::settingsSel;

        static ListT* getList(EditorDataT& editorData)
        {
            return &editorData.settings;
        }
    };

    struct Scenes : public ResourceScenes {
        using ListT = NamedList<Resources::SceneInput>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 255;

        constexpr static auto SelectionPtr = &EditorT::scenesSel;

        static ListT* getList(EditorDataT& editorData)
        {
            return &editorData.scenes;
        }
    };
};

ScenesEditorData::ScenesEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
{
}

bool ScenesEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    scenes = projectFile.resourceScenes;

    return true;
}

void ScenesEditorData::updateSelection()
{
    settingsSel.update();
    scenesSel.update();
}

ScenesEditorGui::ScenesEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool ScenesEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<ScenesEditorData*>(data));
}

void ScenesEditorGui::editorDataChanged()
{
}

void ScenesEditorGui::editorOpened()
{
}

void ScenesEditorGui::editorClosed()
{
}

void ScenesEditorGui::settingsWindow()
{
    assert(_data);
    auto& scenes = _data->scenes;

    ImGui::SetNextWindowSize(ImVec2(1000, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Scene Settings")) {

        ListButtons<AP::SceneSettings>(_data);

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

        for (unsigned i = 0; i < scenes.settings.size(); i++) {
            auto& sceneSettings = scenes.settings.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_data->settingsSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &sceneSettings.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::EnumCombo("##BgMode", &sceneSettings.bgMode);
            ImGui::NextColumn();

            static const std::array<const char*, 4> layerLabels = { "##LT0", "##LT1", "##LT2", "##LT3" };
            for (unsigned l = 0; l < sceneSettings.layerTypes.size(); l++) {
                ImGui::SetNextItemWidth(-1);
                edited |= ImGui::EnumCombo(layerLabels.at(l), &sceneSettings.layerTypes.at(l));
                ImGui::NextColumn();
            }

            if (edited) {
                ListActions<AP::SceneSettings>::itemEdited(_data, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

bool ScenesEditorGui::sceneLayerCombo(const char* label, idstring* value,
                                      const Project::ProjectFile& projectFile, const Resources::SceneSettingsInput& sceneSettings, const unsigned layerId)
{
    using namespace std::string_literals;
    using LayerType = Resources::LayerType;

    assert(_data);

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

void ScenesEditorGui::scenesWindow(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& scenes = _data->scenes;

    ImGui::SetNextWindowSize(ImVec2(1000, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Scenes")) {

        ListButtons<AP::Scenes>(_data);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(8);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Scene Settings");
        ImGui::NextColumn();
        ImGui::Text("Palette");
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

        for (unsigned i = 0; i < scenes.scenes.size(); i++) {
            auto& scene = scenes.scenes.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_data->scenesSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &scene.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::IdStringCombo("##SceneSettings", &scene.sceneSettings, _data->scenes.settings);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::IdStringCombo("##Palette", &scene.palette, projectFile.palettes);
            ImGui::NextColumn();

            const auto sceneSettings = scenes.settings.find(scene.sceneSettings);

            static const std::array<const char*, 4> layerLabels = { "##LT0", "##LT1", "##LT2", "##LT3" };
            for (unsigned l = 0; l < scene.layers.size(); l++) {
                if (sceneSettings) {
                    ImGui::SetNextItemWidth(-1);
                    edited |= sceneLayerCombo(layerLabels.at(l), &scene.layers.at(l), projectFile, *sceneSettings, l);
                }
                ImGui::NextColumn();
            }

            if (edited) {
                ListActions<AP::Scenes>::itemEdited(_data, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void ScenesEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    settingsWindow();
    scenesWindow(projectFile);
}

}
