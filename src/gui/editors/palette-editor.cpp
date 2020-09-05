/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palette-editor.h"
#include "gui/editor-actions.h"
#include "gui/imgui-filebrowser.h"
#include "gui/imgui.h"

namespace UnTech::Gui {

// PaletteEditor Action Policies
struct PaletteEditor::AP {
    struct Palette {
        using EditorT = PaletteEditor;
        using EditorDataT = UnTech::Resources::PaletteInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return namedListItem(&projectFile.palettes, itemIndex.index);
        }
    };
};

PaletteEditor::PaletteEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
{
}

bool PaletteEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    const auto* p = namedListItem(&projectFile.palettes, itemIndex().index);
    if (p) {
        _data = *p;
    }
    return p != nullptr;
}

void PaletteEditor::editorOpened()
{
    // ::TODO load texture::
}

void PaletteEditor::editorClosed()
{
    // ::TODO unload texture::
}

void PaletteEditor::paletteWindow()
{
    using namespace std::string_literals;
    using PaletteInput = UnTech::Resources::PaletteInput;

    const std::string windowName = _data.name + " Palette###Palette"s;

    if (ImGui::Begin(windowName.c_str())) {
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

        {
            ImGui::InputIdstring("Name", &_data.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::name>(this);
            }

            if (ImGui::InputPngImageFilename("Image", &_data.paletteImageFilename)) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::paletteImageFilename>(this);

                // ::TODO mark texture out of date::
            }

            ImGui::InputUnsigned("Rows Per Frame", &_data.rowsPerFrame);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::rowsPerFrame>(this);
            }

            ImGui::InputUnsigned("Animation Delay", &_data.animationDelay);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::animationDelay>(this);
            }

            ImGui::Checkbox("Skip First Frame", &_data.skipFirstFrame);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::skipFirstFrame>(this);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        {
            ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

            // ::TODO show palette image::

            ImGui::EndChild();
        }
    }
    ImGui::End();
}

void PaletteEditor::processGui(const Project::ProjectFile&)
{
    paletteWindow();
}

void PaletteEditor::updateSelection()
{
}

}
