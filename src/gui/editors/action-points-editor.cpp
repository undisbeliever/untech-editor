/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "action-points-editor.h"
#include "gui/aptable.h"
#include "gui/imgui.h"
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
    : AbstractEditorGui("##AP editor")
    , _data(nullptr)
{
}

bool ActionPointsEditorGui::setEditorData(std::shared_ptr<AbstractEditorData> data)
{
    _data = std::dynamic_pointer_cast<ActionPointsEditorData>(data);
    return _data != nullptr;
}

void ActionPointsEditorGui::resetState()
{
}

void ActionPointsEditorGui::editorClosed()
{
}

void ActionPointsEditorGui::actionPointsGui()
{
    assert(_data);

    ImGui::TextUnformatted(u8"Action Points:");

    apTable<AP::ActionPointFunctions>(
        "Table", _data,
        std::to_array({ "Name", "Manually Invoked" }),

        [&](auto& ap) { return Cell("##name", &ap.name); },
        [&](auto& ap) { return Cell("##Manually Invoked", &ap.manuallyInvoked); });
}

void ActionPointsEditorGui::processGui(const Project::ProjectFile&, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    actionPointsGui();
}

}
