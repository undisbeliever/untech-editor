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
struct InteractiveTilesEditorData::AP {
    struct InteractiveTiles {
        using EditorT = InteractiveTilesEditorData;
        using EditorDataT = UnTech::MetaTiles::InteractiveTiles;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.interactiveTiles;
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

        constexpr static auto SelectionPtr = &EditorT::sel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.functionTables; }
    };
};

InteractiveTilesEditorData::InteractiveTilesEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
{
}

bool InteractiveTilesEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    interactiveTiles = projectFile.interactiveTiles;

    return true;
}

void InteractiveTilesEditorData::updateSelection()
{
    sel.update();
}

InteractiveTilesEditorGui::InteractiveTilesEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool InteractiveTilesEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<InteractiveTilesEditorData*>(data));
}

void InteractiveTilesEditorGui::editorDataChanged()
{
}

void InteractiveTilesEditorGui::editorOpened()
{
}

void InteractiveTilesEditorGui::editorClosed()
{
}

void InteractiveTilesEditorGui::interactiveTilesWindow()
{
    assert(_data);
    auto& interactiveTiles = _data->interactiveTiles;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Interactive Tiles")) {

        ListButtons<AP::FunctionTables>(_data);

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
        for (const auto& ft : interactiveTiles.FIXED_FUNCTION_TABLES) {
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

        for (unsigned i = 0; i < interactiveTiles.functionTables.size(); i++) {
            auto& ft = interactiveTiles.functionTables.at(i);

            bool edited = false;

            ImGui::PushID(i);

            const std::string selLabel = std::to_string(engineId++);
            ImGui::Selectable(selLabel.c_str(), &_data->sel, i);
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
                ListActions<AP::FunctionTables>::itemEdited(_data, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void InteractiveTilesEditorGui::processGui(const Project::ProjectFile&, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    interactiveTilesWindow();
}

}
