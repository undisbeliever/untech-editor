/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "interactive-tiles-editor.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "models/common/iterators.h"
#include "models/metatiles/metatiles-error.h"

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

void InteractiveTilesEditorData::errorDoubleClicked(const AbstractError* error)
{
    using Type = MetaTiles::InteractiveTilesErrorType;

    sel.clearSelection();

    if (auto* e = dynamic_cast<const MetaTiles::InteractiveTilesError*>(error)) {
        switch (e->type) {
        case Type::FUNCTION_TABLE:
            sel.setSelected(e->firstIndex);
            break;
        }
    }
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

void InteractiveTilesEditorGui::resetState()
{
}

void InteractiveTilesEditorGui::editorClosed()
{
}

void InteractiveTilesEditorGui::interactiveTilesWindow()
{
    assert(_data);
    auto& interactiveTiles = _data->interactiveTiles;

    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Interactive Tiles")) {

        ListButtons<AP::FunctionTables>(_data);

        ImGui::BeginChild("scroll");

        ImGui::Columns(3);
        ImGui::SetColumnWidth(0, 40);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Tint");
        ImGui::NextColumn();
        ImGui::Separator();

        unsigned engineId = 0;
        for (const auto& ft : interactiveTiles.FIXED_FUNCTION_TABLES) {
            ImGui::Text("%d", engineId++);
            ImGui::NextColumn();
            ImGui::TextUnformatted(ft.name);
            ImGui::NextColumn();

            ImGui::Text("%06X", ft.tint.rgbHex());
            // ::TODO add color square::
            ImGui::NextColumn();
        }

        ImGui::Separator();

        for (auto [i, ft] : enumerate(interactiveTiles.functionTables)) {
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
            ImGui::InputRgb("##Tint", &ft.tint);
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
