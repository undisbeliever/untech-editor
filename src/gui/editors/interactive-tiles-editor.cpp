/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "interactive-tiles-editor.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"

namespace UnTech::Gui {

// InteractiveTilesEditor Action Policies
struct InteractiveTilesEditor::AP {
    struct InteractiveTiles {
        using EditorT = InteractiveTilesEditor;
        using EditorDataT = UnTech::MetaTiles::InteractiveTiles;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._interactiveTiles;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.interactiveTiles;
        }
    };

    struct FunctionTables : public InteractiveTiles {
        using ListT = NamedList<MetaTiles::InteractiveTileFunctionTable>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaTiles::MAX_INTERACTIVE_TILE_FUNCTION_TABLES
                                           - UnTech::MetaTiles::InteractiveTiles::FIXED_FUNCTION_TABLES.size();

        constexpr static auto SelectionPtr = &EditorT::_sel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.functionTables; }
    };
};

InteractiveTilesEditor::InteractiveTilesEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
{
}

bool InteractiveTilesEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    _interactiveTiles = projectFile.interactiveTiles;

    return true;
}

void InteractiveTilesEditor::editorOpened()
{
}

void InteractiveTilesEditor::editorClosed()
{
}

void InteractiveTilesEditor::interactiveTilesWindow()
{
    if (ImGui::Begin("Interactive Tiles")) {
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        ListButtons<AP::FunctionTables>(this);

        ImGui::BeginChild("scroll");

        ImGui::Columns(4);
        ImGui::SetColumnWidth(0, 40);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Symbol");
        ImGui::NextColumn();
        ImGui::Text("Symbol Color");
        ImGui::NextColumn();
        ImGui::Separator();

        unsigned engineId = 0;
        for (const auto& ft : _interactiveTiles.FIXED_FUNCTION_TABLES) {
            ImGui::Text("%d", engineId++);
            ImGui::NextColumn();
            ImGui::TextUnformatted(ft.name);
            ImGui::NextColumn();

            ImGui::TextUnformatted(ft.symbol);
            ImGui::NextColumn();

            ImGui::Text("%06X", ft.symbolColor.rgbHex());
            // ::TODO add color square::
            ImGui::NextColumn();
        }

        ImGui::Separator();

        for (unsigned i = 0; i < _interactiveTiles.functionTables.size(); i++) {
            auto& ft = _interactiveTiles.functionTables.at(i);

            bool edited = false;

            ImGui::PushID(i);

            const std::string selLabel = std::to_string(engineId++);
            ImGui::Selectable(selLabel.c_str(), &_sel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &ft.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##Symbol", &ft.symbol);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputRgb("##Color", &ft.symbolColor);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            if (edited) {
                ListActions<AP::FunctionTables>::itemEdited(this, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void InteractiveTilesEditor::processGui(const Project::ProjectFile&)
{
    interactiveTilesWindow();
}

void InteractiveTilesEditor::updateSelection()
{
    _sel.update();
}

}
