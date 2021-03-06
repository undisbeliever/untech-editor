/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "action-points-editor.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "models/common/iterators.h"
#include "models/metasprite/metasprite-error.h"

namespace UnTech::Gui {

// ActionPointsEditor Action Policies
struct ActionPointsEditorData::AP {
    struct ActionPointFunctions {
        using EditorT = ActionPointsEditorData;
        using EditorDataT = NamedList<UnTech::MetaSprite::ActionPointFunction>;
        using ListT = NamedList<UnTech::MetaSprite::ActionPointFunction>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 255;

        constexpr static auto SelectionPtr = &EditorT::sel;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.actionPointFunctions;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.actionPointFunctions;
        }

        static ListT* getList(EditorDataT& editorData) { return &editorData; }
    };
};

ActionPointsEditorData::ActionPointsEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
    , actionPointFunctions()
    , sel()
{
}

bool ActionPointsEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    actionPointFunctions = projectFile.actionPointFunctions;

    return true;
}

void ActionPointsEditorData::errorDoubleClicked(const AbstractError* error)
{
    using Type = MetaSprite::ApfErrorType;

    sel.clearSelection();

    if (auto* e = dynamic_cast<const MetaSprite::ActionPointFunctionError*>(error)) {
        switch (e->type) {
        case Type::ACTION_POINT_FUNCTIONS:
            sel.setSelected(e->firstIndex);
            break;
        }
    }
}

void ActionPointsEditorData::updateSelection()
{
    sel.update();
}

ActionPointsEditorGui::ActionPointsEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool ActionPointsEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<ActionPointsEditorData*>(data));
}

void ActionPointsEditorGui::resetState()
{
}

void ActionPointsEditorGui::editorClosed()
{
}

void ActionPointsEditorGui::actionPointsWindow()
{
    assert(_data);
    auto& actionPointFunctions = _data->actionPointFunctions;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Action Points")) {

        ListButtons<AP::ActionPointFunctions>(_data);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(3);
        ImGui::SetColumnWidth(0, 40);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Manually Invoked");
        ImGui::NextColumn();
        ImGui::Separator();

        for (auto [i, ap] : enumerate(actionPointFunctions)) {
            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_data->sel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &ap.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::Checkbox("##ManuallyInvoked", &ap.manuallyInvoked);
            ImGui::NextColumn();

            if (edited) {
                ListActions<AP::ActionPointFunctions>::itemEdited(_data, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void ActionPointsEditorGui::processGui(const Project::ProjectFile&, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    actionPointsWindow();
}

}
