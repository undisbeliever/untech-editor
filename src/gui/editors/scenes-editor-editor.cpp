/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "scenes-editor-editor.h"
#include "gui/aptable.h"
#include "models/common/iterators.h"
#include "models/resources/scene-bgmode.hpp"
#include "models/resources/scene-error.h"

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

void ScenesEditorData::errorDoubleClicked(const AbstractError* error)
{
    using Type = Resources::SceneErrorType;

    settingsSel.clearSelection();
    scenesSel.clearSelection();

    if (auto* e = dynamic_cast<const Resources::SceneError*>(error)) {
        switch (e->type) {
        case Type::SCENE_SETTINGS:
            settingsSel.setSelected(e->firstIndex);
            break;

        case Type::SCENE_LAYER_ERROR:
            scenesSel.setSelected(e->firstIndex);
            break;
        }
    }
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

void ScenesEditorGui::resetState()
{
}

void ScenesEditorGui::editorClosed()
{
}

void ScenesEditorGui::settingsWindow()
{
    assert(_data);

    ImGui::SetNextWindowSize(ImVec2(1000, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Scene Settings")) {

        apTable<AP::SceneSettings>(
            "Table", _data,
            std::to_array({ "Name", "BG Mode", "Layer 0 Type", "Layer 1 Type", "Layer 2 Type", "Layer 3 Type" }),

            [&](auto& ss) { return Cell("##name", &ss.name); },
            [&](auto& ss) { return Cell("##bgMode", &ss.bgMode); },
            [&](auto& ss) { return Cell("##lt0", &ss.layerTypes.at(0)); },
            [&](auto& ss) { return Cell("##lt1", &ss.layerTypes.at(1)); },
            [&](auto& ss) { return Cell("##lt2", &ss.layerTypes.at(2)); },
            [&](auto& ss) { return Cell("##lt3", &ss.layerTypes.at(3)); });
    }
    ImGui::End();
}

static bool sceneLayerCombo(const char* label, idstring* value,
                            const Project::ProjectFile& projectFile, optional_ref<const Resources::SceneSettingsInput&> sceneSettings, const unsigned layerId)
{
    using namespace std::string_literals;
    using LayerType = Resources::LayerType;

    if (!sceneSettings) {
        return false;
    }

    const auto layer = sceneSettings->layerTypes.at(layerId);

    switch (layer) {
    case LayerType::None: {
        return false;
    }

    case LayerType::TextConsole: {
        ImGui::TextUnformatted(u8"Text"s);
        return false;
    }

    case LayerType::BackgroundImage: {
        const unsigned bitDepth = Resources::bitDepthForLayer(sceneSettings->bgMode, layerId);

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
        const unsigned bitDepth = Resources::bitDepthForLayer(sceneSettings->bgMode, layerId);

        bool e = ImGui::IdStringCombo(label, value, projectFile.metaTileTilesets, false,
                                      [&](const auto& efi) {
                                          const auto& mt = efi.value;
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
    const auto& settings = _data->scenes.settings;

    ImGui::SetNextWindowSize(ImVec2(1000, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Scenes")) {

        apTable<AP::Scenes>(
            "Table", _data,
            std::to_array({ "Name", "Scene Settings", "Palette", "Layer 0", "Layer 1", "Layer 2", "Layer 3" }),

            [&](auto& s) { return Cell("##name", &s.name); },
            [&](auto& s) { return Cell("##scenesettings", &s.sceneSettings); },
            [&](auto& s) { return Cell("##palette", &s.palette, projectFile.palettes); },
            [&](auto& s) { return sceneLayerCombo("##layer0", &s.layers.at(0), projectFile, settings.find(s.sceneSettings), 0); },
            [&](auto& s) { return sceneLayerCombo("##layer1", &s.layers.at(1), projectFile, settings.find(s.sceneSettings), 1); },
            [&](auto& s) { return sceneLayerCombo("##layer2", &s.layers.at(2), projectFile, settings.find(s.sceneSettings), 2); },
            [&](auto& s) { return sceneLayerCombo("##layer3", &s.layers.at(3), projectFile, settings.find(s.sceneSettings), 3); });
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
