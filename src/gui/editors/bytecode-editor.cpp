/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "bytecode-editor.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"

namespace UnTech::Gui {

// ActionPointsEditor Action Policies
struct BytecodeEditorData::AP {
    struct Bytecode {
        using EditorT = BytecodeEditorData;
        using EditorDataT = UnTech::Scripting::BytecodeInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.bytecode;
        }
    };

    struct Instructions : public Bytecode {
        using ListT = std::vector<Scripting::Instruction>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 256 / 2;

        constexpr static auto SelectionPtr = &EditorT::instructionSel;

        static ListT* getList(EditorDataT& editorData)
        {
            return &editorData.instructions;
        }
    };
};

BytecodeEditorData::BytecodeEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
    , data()
    , instructionSel()
{
}

bool BytecodeEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    data = projectFile.bytecode;

    return true;
}

void BytecodeEditorData::updateSelection()
{
    instructionSel.update();
}

BytecodeEditorGui::BytecodeEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool BytecodeEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<BytecodeEditorData*>(data));
}

void BytecodeEditorGui::editorDataChanged()
{
}

void BytecodeEditorGui::editorOpened()
{
}

void BytecodeEditorGui::editorClosed()
{
}

void BytecodeEditorGui::instructionsWindow()
{
    assert(_data);
    auto& bytecode = _data->data;

    ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Bytecode Instructions")) {

        ListButtons<AP::Instructions>(_data);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(4);
        ImGui::SetColumnWidth(0, 40);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Argument 1");
        ImGui::NextColumn();
        ImGui::Text("Argument 2");
        ImGui::NextColumn();
        ImGui::Separator();

        for (auto& inst : bytecode.BASE_INSTRUCTIONS) {
            ImGui::NextColumn();

            ImGui::TextUnformatted(inst.name);
            ImGui::NextColumn();

            for (auto& a : inst.arguments) {
                ImGui::TextEnum(a);
                ImGui::NextColumn();
            }
        }

        for (unsigned i = 0; i < bytecode.instructions.size(); i++) {
            auto& inst = bytecode.instructions.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_data->instructionSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &inst.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            edited |= ImGui::EnumCombo("##Arg1", &inst.arguments.at(0));
            ImGui::NextColumn();

            edited |= ImGui::EnumCombo("##Arg2", &inst.arguments.at(1));
            ImGui::NextColumn();

            if (edited) {
                ListActions<AP::Instructions>::itemEdited(_data, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void BytecodeEditorGui::processGui(const Project::ProjectFile&, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    instructionsWindow();
}

}
