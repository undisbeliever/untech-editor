/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameset-export-order-editor.h"
#include "gui/editor-actions.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "models/metasprite/metasprite-error.h"

namespace UnTech::Gui {

static const char* flipItems[] = {
    "",
    "hFlip",
    "vFlip",
    "hvFlip",
};

struct FrameSetExportOrderEditorData::AP {
    struct ExportOrder {
        using EditorT = FrameSetExportOrderEditorData;
        using EditorDataT = UnTech::MetaSprite::FrameSetExportOrder;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
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

        static constexpr const char* name = "Frame";
        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::framesSel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.stillFrames; }
    };

    struct FrameAlternatives final : public ExportOrder {
        using ListT = std::vector<MetaSprite::NameReference>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::frameAlternativesSel;

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

        static constexpr const char* name = "Animation";
        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::animationsSel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.animations; }
    };

    struct AnimationsAlternatives final : public ExportOrder {
        using ListT = std::vector<MetaSprite::NameReference>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::animationAlternativesSel;

        static ListT* getList(EditorDataT& editorData, unsigned structIndex)
        {
            return getListField(Animations::getList(editorData), structIndex,
                                &EditorDataT::ExportName::alternatives);
        }
    };
};

FrameSetExportOrderEditorData::FrameSetExportOrderEditorData(ItemIndex itemIndex)
    : AbstractExternalFileEditorData(itemIndex)
{
}

bool FrameSetExportOrderEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    const auto [ptr, fn] = fileListItem(&projectFile.frameSetExportOrders, itemIndex().index);
    setFilename(fn);
    if (ptr) {
        data = *ptr;
    }
    return ptr != nullptr;
}

void FrameSetExportOrderEditorData::saveFile() const
{
    assert(!filename().empty());
    UnTech::MetaSprite::saveFrameSetExportOrder(data, filename());
}

void FrameSetExportOrderEditorData::errorDoubleClicked(const AbstractSpecializedError* error)
{
    using Type = MetaSprite::EoErrorType;

    framesSel.clearSelection();
    frameAlternativesSel.clearSelection();
    animationsSel.clearSelection();
    animationAlternativesSel.clearSelection();

    if (auto* e = dynamic_cast<const MetaSprite::ExportOrderError*>(error)) {
        switch (e->type) {
        case Type::STILL_FRAMES:
            framesSel.setSelected(e->firstIndex);
            break;

        case Type::STILL_FRAMES_ALT:
            framesSel.setSelected(e->firstIndex);
            frameAlternativesSel.setSelected(e->firstIndex, e->childIndex);
            break;

        case Type::ANIMATIONS:
            animationsSel.setSelected(e->firstIndex);
            break;

        case Type::ANIMATIONS_ALT:
            animationsSel.setSelected(e->firstIndex);
            animationAlternativesSel.setSelected(e->firstIndex, e->childIndex);
            break;
        }
    }
}

void FrameSetExportOrderEditorData::updateSelection()
{
    framesSel.update();
    frameAlternativesSel.update(framesSel);
    animationsSel.update();
    animationAlternativesSel.update(animationsSel);
}

FrameSetExportOrderEditorGui::FrameSetExportOrderEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool FrameSetExportOrderEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<FrameSetExportOrderEditorData*>(data));
}

void FrameSetExportOrderEditorGui::resetState()
{
}

void FrameSetExportOrderEditorGui::editorClosed()
{
}

template <typename ExportNameAP>
void FrameSetExportOrderEditorGui::exportNameTree(const char* label)
{
    using ExportName = UnTech::MetaSprite::FrameSetExportOrder::ExportName;

    static_assert(std::is_same_v<ExportNameAP, AP::Frames> || std::is_same_v<ExportNameAP, AP::Animations>);
    using AltAP = std::conditional_t<std::is_same_v<ExportNameAP, AP::Frames>, AP::FrameAlternatives, AP::AnimationsAlternatives>;
    using OtherNameAP = std::conditional_t<!std::is_same_v<ExportNameAP, AP::Frames>, AP::Frames, AP::Animations>;

    SingleSelection& sel = _data->*ExportNameAP::SelectionPtr;
    MultipleChildSelection& altSel = _data->*AltAP::SelectionPtr;

    assert(_data);
    auto& exportOrder = _data->data;

    const auto indentSpacing = ImGui::GetStyle().IndentSpacing;

    ImGui::Spacing();
    ImGui::Spacing();
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
        // Putting this code outside the `label` scope allows me to resize
        // both "tables" at the same time.
        ImGui::Columns(3);
        ImGui::SetColumnWidth(0, 50);
        ImGui::SetColumnWidth(1, 300);

        ImGui::Separator();

        ImGui::PushID(label);

        auto* list = ExportNameAP::getList(exportOrder);
        assert(list);

        for (auto [i, en] : enumerate(*list)) {
            ImGui::PushID(i);

            if (ImGui::Selectable(&sel, i)) {
                // unselect the other items
                (_data->*OtherNameAP::SelectionPtr).clearSelection();
            }
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &en.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                ListActions<ExportNameAP>::template fieldEdited<&ExportName::name>(_data, i);
            }
            ImGui::NextColumn();

            if (sel.isSelected(i)) {
                ListButtons<AltAP>(_data);
            }
            ImGui::NextColumn();

            for (auto [j, alt] : enumerate(en.alternatives)) {
                bool edited = false;

                ImGui::PushID(j);

                ImGui::Indent(indentSpacing / 2);
                if (ImGui::Selectable(&sel, &altSel, i, j)) {
                    // unselect the other items
                    (_data->*OtherNameAP::SelectionPtr).clearSelection();
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
                    ListActions<AltAP>::itemEdited(_data, i, j);
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

void FrameSetExportOrderEditorGui::exportOrderWindow()
{
    using namespace std::string_literals;

    assert(_data);
    auto& exportOrder = _data->data;

    const std::string windowName = exportOrder.name + " Export Order###ExportOrder";

    ImGui::SetNextWindowSize(ImVec2(550, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(windowName.c_str())) {

        ImGui::InputIdstring("Name", &exportOrder.name);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            EditorActions<AP::ExportOrder>::fieldEdited<
                &MetaSprite::FrameSetExportOrder::name>(_data);
        }

        ImGui::Spacing();

        CombinedListButtons<AP::Frames, AP::Animations>("List_Buttons", _data);

        exportNameTree<AP::Frames>("Still Frames");
        exportNameTree<AP::Animations>("Animations");
    }
    ImGui::End();
}

void FrameSetExportOrderEditorGui::processGui(const Project::ProjectFile&, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    exportOrderWindow();
}

}
