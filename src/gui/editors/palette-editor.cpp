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
struct PaletteEditorData::AP {
    struct Palette {
        using EditorT = PaletteEditorData;
        using EditorDataT = UnTech::Resources::PaletteInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return namedListItem(&projectFile.palettes, itemIndex.index);
        }
    };
};

PaletteEditorData::PaletteEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
{
}

bool PaletteEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    const auto* p = namedListItem(&projectFile.palettes, itemIndex().index);
    if (p) {
        data = *p;
    }
    return p != nullptr;
}

void PaletteEditorData::updateSelection()
{
}

PaletteEditorGui::PaletteEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool PaletteEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<PaletteEditorData*>(data));
}

void PaletteEditorGui::editorDataChanged()
{
    // ::TODO invalidate texture::
}

void PaletteEditorGui::editorOpened()
{
    // ::TODO load texture::
}

void PaletteEditorGui::editorClosed()
{
}

void PaletteEditorGui::paletteWindow()
{
    using namespace std::string_literals;
    using PaletteInput = UnTech::Resources::PaletteInput;

    assert(_data);
    auto& palette = _data->data;

    const std::string windowName = palette.name + " Palette###Palette"s;

    if (ImGui::Begin(windowName.c_str())) {
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

        {
            ImGui::InputIdstring("Name", &palette.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::name>(_data);
            }

            if (ImGui::InputPngImageFilename("Image", &palette.paletteImageFilename)) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::paletteImageFilename>(_data);

                // ::TODO mark texture out of date::
            }

            ImGui::InputUnsigned("Rows Per Frame", &palette.rowsPerFrame);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::rowsPerFrame>(_data);
            }

            ImGui::InputUnsigned("Animation Delay", &palette.animationDelay);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::animationDelay>(_data);
            }

            ImGui::Checkbox("Skip First Frame", &palette.skipFirstFrame);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::Palette>::fieldEdited<
                    &PaletteInput::skipFirstFrame>(_data);
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

void PaletteEditorGui::processGui(const Project::ProjectFile&)
{
    if (_data == nullptr) {
        return;
    }

    paletteWindow();
}

}
