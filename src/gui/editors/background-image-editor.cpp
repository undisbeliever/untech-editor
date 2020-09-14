/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "background-image-editor.h"
#include "gui/editor-actions.h"
#include "gui/imgui-filebrowser.h"
#include "gui/imgui.h"

namespace UnTech::Gui {

// Background Image Action Policies
struct BackgroundImageEditorData::AP {
    struct BackgroundImage {
        using EditorT = BackgroundImageEditorData;
        using EditorDataT = UnTech::Resources::BackgroundImageInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return namedListItem(&projectFile.backgroundImages, itemIndex.index);
        }
    };
};

BackgroundImageEditorData::BackgroundImageEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
{
}

bool BackgroundImageEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    const auto* bi = namedListItem(&projectFile.backgroundImages, itemIndex().index);
    if (bi) {
        data = *bi;
    }
    return bi != nullptr;
}

void BackgroundImageEditorData::updateSelection()
{
}

BackgroundImageEditorGui::BackgroundImageEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool BackgroundImageEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<BackgroundImageEditorData*>(data));
}

void BackgroundImageEditorGui::editorDataChanged()
{
    // ::TODO invalidate texture::
}

void BackgroundImageEditorGui::editorOpened()
{
    // ::TODO load texture::
}

void BackgroundImageEditorGui::editorClosed()
{
}

void BackgroundImageEditorGui::backgroundImageWindow(const Project::ProjectFile& projectFile)
{
    using namespace std::string_literals;
    using BackgroundImageInput = UnTech::Resources::BackgroundImageInput;

    assert(_data);
    auto& bi = _data->data;

    if (ImGui::Begin("Background Image")) {
        ImGui::SetWindowSize(ImVec2(650, 650), ImGuiCond_FirstUseEver);

        {
            ImGui::InputIdstring("Name", &bi.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::name>(_data);
            }

            if (ImGui::InputUnsigned("Bit Depth", &bi.bitDepth, 0, 0, "%u bpp")) {
                if (bi.isBitDepthValid() == false) {
                    bi.bitDepth = 4;
                }
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::bitDepth>(_data);
            }

            if (ImGui::InputPngImageFilename("Image", &bi.imageFilename)) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::imageFilename>(_data);

                // ::TODO mark texture out of date::
            }

            if (ImGui::IdStringCombo("Conversion Palette", &bi.conversionPlette, projectFile.palettes)) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::conversionPlette>(_data);
            }

            ImGui::InputUnsigned("First Palette", &bi.firstPalette);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::firstPalette>(_data);
            }

            ImGui::InputUnsigned("Number of Palettes", &bi.nPalettes);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::nPalettes>(_data);
            }

            ImGui::Checkbox("Default Order", &bi.defaultOrder);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::defaultOrder>(_data);
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        {
            // ::TODO read BackgroundImageData::tiles.size()::
            const unsigned numberOfTiles = 0;

            ImGui::Text("Number of Tiles: %u", numberOfTiles);
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        {
            ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

            // ::TODO show background image::

            ImGui::EndChild();
        }
    }
    ImGui::End();
}

void BackgroundImageEditorGui::processGui(const Project::ProjectFile& projectFile)
{
    if (_data == nullptr) {
        return;
    }

    backgroundImageWindow(projectFile);
}

}
