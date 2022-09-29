/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "interactive-tiles-editor.h"
#include "gui/aptable.h"
#include "gui/imgui.h"
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

        constexpr auto columnNames = std::to_array({ "Name", "Tint" });

        if (beginApTable("Table", columnNames)) {
            for (auto [i, ft] : enumerate(interactiveTiles.FIXED_FUNCTION_TABLES)) {
                const int engineId = i;

                ImGui::TableNextColumn();
                ImGui::Text("%d", engineId);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ft.name);

                ImGui::TableNextColumn();
                ImGui::Text("%06X", ft.tint.rgbHex());
                // ::TODO add color square::
            }

            const auto nFixedFunctions = interactiveTiles.FIXED_FUNCTION_TABLES.size();

            apTable_data_custom<AP::FunctionTables>(
                _data,
                _data->sel.listArgs(),
                [&](auto* sel, const auto index) {
                    const std::u8string selLabel = stringBuilder(index + nFixedFunctions);
                    ImGui::Selectable(u8Cast(selLabel), sel, index);
                },

                [&](auto& ft) { return Cell("##name", &ft.name); },
                [&](auto& ft) { return Cell("##tint", &ft.tint); });

            endApTable();
        }
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
