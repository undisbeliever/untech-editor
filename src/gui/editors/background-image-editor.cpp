/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "background-image-editor.h"
#include "gui/editor-actions.h"
#include "gui/imgui.h"

namespace UnTech::Gui {

// Background Image Action Policies
struct BackgroundImageEditor::AP {
    struct BackgroundImage {
        using EditorT = BackgroundImageEditor;
        using EditorDataT = UnTech::Resources::BackgroundImageInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return namedListItem(&projectFile.backgroundImages, itemIndex.index);
        }
    };
};

BackgroundImageEditor::BackgroundImageEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
{
}

bool BackgroundImageEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    const auto* bi = namedListItem(&projectFile.backgroundImages, itemIndex().index);
    if (bi) {
        _data = *bi;
    }
    return bi != nullptr;
}

void BackgroundImageEditor::editorOpened()
{
    // ::TODO load texture::
}

void BackgroundImageEditor::editorClosed()
{
    // ::TODO unload texture::
}

void BackgroundImageEditor::backgroundImageWindow(const Project::ProjectFile& projectFile)
{
    using namespace std::string_literals;
    using BackgroundImageInput = UnTech::Resources::BackgroundImageInput;

    if (ImGui::Begin("Background Image")) {
        ImGui::SetWindowSize(ImVec2(650, 650), ImGuiCond_FirstUseEver);

        {
            ImGui::InputIdstring("Name", &_data.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::name>(this);
            }

            if (ImGui::InputUnsigned("Bit Depth", &_data.bitDepth, 0, 0, "%u bpp")) {
                if (_data.isBitDepthValid() == false) {
                    _data.bitDepth = 4;
                }
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::bitDepth>(this);
            }

            // ::TODO filename input::
            std::string fn = _data.imageFilename;
            ImGui::InputText("Image", &fn);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                _data.imageFilename = fn;

                // ::TODO add callback - loadTexture()::
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::imageFilename>(this);
            }

            if (ImGui::IdStringCombo("Conversion Palette", &_data.conversionPlette, projectFile.palettes)) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::conversionPlette>(this);
            }

            ImGui::InputUnsigned("First Palette", &_data.firstPalette);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::firstPalette>(this);
            }

            ImGui::InputUnsigned("Number of Palettes", &_data.nPalettes);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::nPalettes>(this);
            }

            ImGui::Checkbox("Default Order", &_data.defaultOrder);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::BackgroundImage>::fieldEdited<
                    &BackgroundImageInput::defaultOrder>(this);
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

void BackgroundImageEditor::processGui(const Project::ProjectFile& projectFile)
{
    backgroundImageWindow(projectFile);
}

void BackgroundImageEditor::updateSelection()
{
}

}
