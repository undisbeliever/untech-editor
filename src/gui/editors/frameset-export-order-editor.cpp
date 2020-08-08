/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameset-export-order-editor.h"
#include "gui/editor-actions.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"

namespace UnTech::Gui {

static const char* flipItems[] = {
    "",
    "hFlip",
    "vFlip",
    "hvFlip",
};

struct FrameSetExportOrderEditor::AP {
    struct ExportOrder {
        using EditorT = FrameSetExportOrderEditor;
        using EditorDataT = UnTech::MetaSprite::FrameSetExportOrder;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            const auto i = itemIndex.index;
            if (i < projectFile.frameSetExportOrders.size()) {
                return projectFile.frameSetExportOrders.at(i);
            }
            return nullptr;
        }
    };

    struct Frames final : public ExportOrder {
        using ListT = NamedList<EditorDataT::ExportName>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::_framesSel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.stillFrames; }
    };

    struct FrameAlternatives final : public ExportOrder {
        using ListT = std::vector<MetaSprite::NameReference>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::_frameAlternativesSel;

        static ListT* getList(EditorDataT& editorData, unsigned structIndex)
        {
            return getListField(Frames::getList(editorData), structIndex,
                                &EditorDataT::ExportName::alternatives);
        }
    };

    struct Animations final : public ExportOrder {
        using ListT = NamedList<EditorDataT::ExportName>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::_animationsSel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.animations; }
    };

    struct AnimationsAlternatives final : public ExportOrder {
        using ListT = std::vector<MetaSprite::NameReference>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::_animationAlternativesSel;

        static ListT* getList(EditorDataT& editorData, unsigned structIndex)
        {
            return getListField(Animations::getList(editorData), structIndex,
                                &EditorDataT::ExportName::alternatives);
        }
    };
};

FrameSetExportOrderEditor::FrameSetExportOrderEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
{
}

bool FrameSetExportOrderEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    const auto i = itemIndex().index;
    if (i < projectFile.frameSetExportOrders.size()) {
        auto* eo = projectFile.frameSetExportOrders.at(i);
        if (eo) {
            _data = *eo;
            return true;
        }
    }

    return false;
}

void FrameSetExportOrderEditor::editorOpened()
{
}

void FrameSetExportOrderEditor::editorClosed()
{
}

template <typename ExportNameAP>
void FrameSetExportOrderEditor::exportNameTree(const char* label)
{
    using ExportName = UnTech::MetaSprite::FrameSetExportOrder::ExportName;
    using NameReference = UnTech::MetaSprite::NameReference;

    static_assert(std::is_same_v<ExportNameAP, AP::Frames> || std::is_same_v<ExportNameAP, AP::Animations>);
    using AltAP = std::conditional_t<std::is_same_v<ExportNameAP, AP::Frames>, AP::FrameAlternatives, AP::AnimationsAlternatives>;
    using OtherNameAP = std::conditional_t<!std::is_same_v<ExportNameAP, AP::Frames>, AP::Frames, AP::Animations>;

    SingleSelection& sel = this->*ExportNameAP::SelectionPtr;
    MultipleChildSelection& altSel = this->*AltAP::SelectionPtr;

    const auto indentSpacing = ImGui::GetStyle().IndentSpacing;

    ImGui::Spacing();
    ImGui::Spacing();
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID(label);
        ListButtons<ExportNameAP>(this);
        ImGui::PopID();

        // Putting this code outside the `label` scope allows me to resize
        // both "tables" at the same time.
        ImGui::Columns(3);
        ImGui::SetColumnWidth(0, 50);
        ImGui::SetColumnWidth(1, 300);

        ImGui::Separator();

        ImGui::PushID(label);

        auto* list = ExportNameAP::getList(_data);
        assert(list);

        for (size_t i = 0; i < list->size(); i++) {
            ExportName& en = list->at(i);

            ImGui::PushID(i);

            if (ImGui::Selectable(&sel, i)) {
                // unselect the other items
                (this->*OtherNameAP::SelectionPtr).clearSelection();
            }
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &en.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                ListActions<ExportNameAP>::template fieldEdited<&ExportName::name>(this, i);
            }
            ImGui::NextColumn();

            if (sel.isSelected(i)) {
                ListButtons<AltAP>(this);
            }
            ImGui::NextColumn();

            for (size_t j = 0; j < en.alternatives.size(); j++) {
                NameReference& alt = en.alternatives.at(j);

                bool edited = false;

                ImGui::PushID(j);

                ImGui::Indent(indentSpacing / 2);
                if (ImGui::Selectable(&sel, &altSel, i, j)) {
                    // unselect the other items
                    (this->*OtherNameAP::SelectionPtr).clearSelection();
                }
                ImGui::Unindent(indentSpacing / 2);
                ImGui::NextColumn();

                ImGui::Text("  Alt:");
                ImGui::SameLine();

                ImGui::SetNextItemWidth(-1);
                ImGui::InputIdstring("##Name", &alt.name);
                edited |= ImGui::IsItemDeactivatedAfterEdit();
                ImGui::NextColumn();

                ImGui::SetNextItemWidth(-1);
                int flip = (alt.vFlip << 1) | alt.hFlip;
                if (ImGui::Combo("##Alt", &flip, flipItems, IM_ARRAYSIZE(flipItems))) {
                    alt.hFlip = flip & 1;
                    alt.vFlip = flip & 2;
                    edited = true;
                }
                ImGui::NextColumn();

                if (edited) {
                    ListActions<AltAP>::itemEdited(this, i, j);
                }

                ImGui::PopID();
            }
            ImGui::Separator();

            ImGui::PopID();
        }

        ImGui::Columns(1);

        ImGui::PopID();
    }
}

void FrameSetExportOrderEditor::exportOrderWindow()
{
    using namespace std::string_literals;

    const std::string windowName = _data.name + " Export Order###ExportOrder";

    if (ImGui::Begin(windowName.c_str())) {
        ImGui::SetWindowSize(ImVec2(550, 700), ImGuiCond_FirstUseEver);

        ImGui::InputIdstring("Name", &_data.name);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            EditorActions<AP::ExportOrder>::fieldEdited<
                &MetaSprite::FrameSetExportOrder::name>(this);
        }

        ImGui::Spacing();

        // ::TODO add combined list buttons at the top of the window::

        exportNameTree<AP::Frames>("Still Frames");
        exportNameTree<AP::Animations>("Animations");
    }
    ImGui::End();
}

void FrameSetExportOrderEditor::processGui(const Project::ProjectFile&)
{
    exportOrderWindow();
}

void FrameSetExportOrderEditor::updateSelection()
{
    _framesSel.update();
    _frameAlternativesSel.update(_framesSel);
    _animationsSel.update();
    _animationAlternativesSel.update(_animationsSel);
}

}
