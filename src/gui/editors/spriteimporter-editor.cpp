/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "spriteimporter-editor.h"
#include "gui/common/aabb-graphics.h"
#include "gui/editor-actions.h"
#include "gui/imgui-combos.h"
#include "gui/imgui-filebrowser.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "models/common/bit.h"
#include "models/common/imagecache.h"
#include <algorithm>
#include <unordered_set>

namespace UnTech::Gui {

namespace SI = UnTech::MetaSprite::SpriteImporter;
using ObjectSize = UnTech::MetaSprite::ObjectSize;

// ::TODO move to style::
constexpr static ImU32 frameOutlineCol = IM_COL32(160, 160, 160, 240);
constexpr static ImU32 antiHighlightCol = IM_COL32(128, 128, 128, 128);

constexpr static ImU32 frameObjectOutlineCol = IM_COL32(64, 128, 64, 240);
constexpr static ImU32 actionPointOutlineCol = IM_COL32(192, 192, 192, 240);
constexpr static ImU32 entityHitboxOutlineCol = IM_COL32(0, 0, 255, 240);
constexpr static ImU32 tileHitboxOutlineCol = IM_COL32(192, 0, 0, 240);

constexpr static ImU32 backgroundCol = IM_COL32(192, 192, 192, 255);
constexpr static ImU32 originColor = IM_COL32(128, 128, 128, 255);

constexpr static unsigned MINIMIM_FRAME_SIZE = 16;

constexpr static int IMAGE_PADDING = 4;

// ::TODO dynamic zoom::
static const ImVec2 zoom(6.0f, 6.0f);

// ::TODO find better name::
AabbGraphics SpriteImporterEditor::_graphics;

std::vector<std::pair<ImU32, std::string>> SpriteImporterEditor::_transparentColorCombo;

bool SpriteImporterEditor::_imageValid = false;
bool SpriteImporterEditor::_transparentColorComboValid = false;

// MetaSpriteEditor Action Policies
struct SpriteImporterEditor::AP {
    struct FrameSet {
        using EditorT = SpriteImporterEditor;
        using EditorDataT = SI::FrameSet;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;

            if (itemIndex.index < projectFile.frameSets.size()) {
                auto& f = projectFile.frameSets.at(itemIndex.index);
                if (f.type == FrameSetType::SPRITE_IMPORTER) {
                    return f.siFrameSet.get();
                }
            }
            return nullptr;
        }
    };

    struct Frames final : public FrameSet {
        using ListT = NamedList<SI::Frame>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::_framesSel;

        static ListT* getList(SI::FrameSet& fs) { return &fs.frames; }
    };

    struct FrameObjects final : public FrameSet {
        using ListT = std::vector<SI::FrameObject>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_FRAME_OBJECTS;

        constexpr static auto SelectionPtr = &EditorT::_frameObjectsSel;

        static ListT* getList(SI::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &SI::Frame::objects);
        }
    };

    struct ActionPoints final : public FrameSet {
        using ListT = std::vector<SI::ActionPoint>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_ACTION_POINTS;

        constexpr static auto SelectionPtr = &EditorT::_actionPointsSel;

        static ListT* getList(SI::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &SI::Frame::actionPoints);
        }
    };

    struct EntityHitboxes final : public FrameSet {
        using ListT = std::vector<SI::EntityHitbox>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_ENTITY_HITBOXES;

        constexpr static auto SelectionPtr = &EditorT::_entityHitboxesSel;

        static ListT* getList(SI::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &SI::Frame::entityHitboxes);
        }
    };

    struct Animations final : public FrameSet {
        using ListT = NamedList<UnTech::MetaSprite::Animation::Animation>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::_animationsSel;

        static ListT* getList(SI::FrameSet& fs) { return &fs.animations; }
    };

    struct AnimationFrames final : public FrameSet {
        using ListT = std::vector<UnTech::MetaSprite::Animation::AnimationFrame>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        // ::TODO replace with UnTech::MetaSprite::MAX_ANIMATION_FRAMES ::
        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::_animationFramesSel;

        static ListT* getList(SI::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Animations::getList(fs), frameIndex,
                                &UnTech::MetaSprite::Animation::Animation::frames);
        }
    };
};

Texture& SpriteImporterEditor::imageTexture()
{
    static Texture texture;
    return texture;
}

SpriteImporterEditor::SpriteImporterEditor(ItemIndex itemIndex)
    : AbstractMetaSpriteEditor(itemIndex)
{
}

bool SpriteImporterEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;

    _imageValid = false;
    _transparentColorComboValid = false;

    const auto i = itemIndex().index;
    if (i < projectFile.frameSets.size()) {
        auto& f = projectFile.frameSets.at(i);

        setFilename(f.filename);
        if (f.type == FrameSetType::SPRITE_IMPORTER) {
            if (f.siFrameSet) {
                _data = *f.siFrameSet;
                return true;
            }
        }
    }

    return false;
}

void SpriteImporterEditor::saveFile() const
{
    assert(!filename().empty());
    UnTech::MetaSprite::SpriteImporter::saveFrameSet(_data, filename());
}

void SpriteImporterEditor::editorOpened()
{
    _imageValid = false;
    _transparentColorComboValid = false;

    _graphics.resetState();
}

void SpriteImporterEditor::editorClosed()
{
}

void SpriteImporterEditor::frameSetPropertiesWindow(const Project::ProjectFile& projectFile)
{
    if (ImGui::Begin("FrameSet##SI")) {
        ImGui::SetWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);

        {
            ImGui::InputIdstring("Name", &_data.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &SI::FrameSet::name>(this);
            }

            if (ImGui::EnumCombo("Tileset Type", &_data.tilesetType)) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &SI::FrameSet::tilesetType>(this);
            }

            if (ImGui::IdStringCombo("Export Order", &_data.exportOrder, projectFile.frameSetExportOrders)) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &SI::FrameSet::exportOrder>(this);
            }

            if (ImGui::InputPngImageFilename("Image", &_data.imageFilename)) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &SI::FrameSet::imageFilename>(this);

                _imageValid = false;
                _transparentColorComboValid = false;
            }

            {
                bool edited;

                ImColor c(_data.transparentColor.rgb());

                char tcString[16];
                std::snprintf(tcString, IM_ARRAYSIZE(tcString), "#%06X", _data.transparentColor.rgbHex());

                if (ImGui::BeginCombo("Transparent Color", tcString)) {
                    updateTransparentColorCombo();

                    for (const auto& [col, str] : _transparentColorCombo) {
                        ImGui::PushStyleColor(ImGuiCol_Text, col);
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, col);
                        if (ImGui::Selectable(str.c_str())) {
                            _data.transparentColor = rgba::fromRgba(col);
                        }
                        ImGui::PopStyleColor(2);

                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::TextUnformatted(str);
                            ImGui::EndTooltip();
                        }
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::ColorEdit3("##ColorEdit", (float*)&c.Value)) {
                    _data.transparentColor = rgba::fromRgba(c);
                    _data.transparentColor.alpha = 0xff;
                }
                edited = ImGui::IsItemDeactivatedAfterEdit();

                if (edited) {
                    EditorActions<AP::FrameSet>::fieldEdited<
                        &SI::FrameSet::transparentColor>(this);
                }
            }

            if (ImGui::TreeNodeEx("Grid", ImGuiTreeNodeFlags_DefaultOpen)) {
                bool edited = false;

                ImGui::InputUsize("Frame Size", &_data.grid.frameSize, usize(SI::MAX_FRAME_SIZE, SI::MAX_FRAME_SIZE));
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::InputUpoint("Offset", &_data.grid.offset, usize(511, 511));
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::InputUpoint("Padding", &_data.grid.padding, usize(511, 511));
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::InputUpoint("Origin", &_data.grid.origin, _data.grid.originRange());
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                if (edited) {
                    for (auto& frame : _data.frames) {
                        frame.location.update(_data.grid, frame);
                    }

                    // ::TODO undo macro::
                    EditorActions<AP::FrameSet>::fieldEdited<
                        &SI::FrameSet::grid>(this);
                    ListActions<AP::Frames>::allItemsInSelectedListFieldEdited<
                        &SI::Frame::location>(this);
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Palette", ImGuiTreeNodeFlags_DefaultOpen)) {
                bool changed = false;
                bool edited = false;

                bool usePalette = _data.palette.usesUserSuppliedPalette();
                edited |= ImGui::Checkbox("User Supplied Palette", &usePalette);

                if (usePalette) {
                    changed |= ImGui::EnumCombo("Position", &_data.palette.position);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    changed |= ImGui::InputUnsigned("No of Palettes", &_data.palette.nPalettes);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    changed |= ImGui::InputUnsigned("Color Size", &_data.palette.colorSize);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();
                }

                if (changed || edited) {
                    if (usePalette) {
                        _data.palette.nPalettes = std::max(1U, _data.palette.nPalettes);
                        _data.palette.colorSize = std::max(1U, _data.palette.colorSize);
                    }
                    else {
                        _data.palette.nPalettes = 0;
                        _data.palette.colorSize = 0;
                    }
                }

                if (edited) {
                    EditorActions<AP::FrameSet>::fieldEdited<
                        &SI::FrameSet::palette>(this);
                }

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

void SpriteImporterEditor::framePropertiesWindow(const Project::ProjectFile& projectFile)
{
    if (ImGui::Begin("Frames##SI")) {
        ImGui::SetWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);

        ListButtons<AP::Frames>(this);

        ImGui::SetNextItemWidth(-1);
        ImGui::NamedListListBox("##FrameList", &_framesSel, _data.frames, 8);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (_framesSel.selectedIndex() < _data.frames.size()) {
            SI::Frame& frame = _data.frames.at(_framesSel.selectedIndex());

            const usize frameSize = frame.location.aabb.size();

            {
                ImGui::InputIdstring("Name", &frame.name);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &SI::Frame::name>(this);
                }

                unsigned spriteOrder = frame.spriteOrder;
                if (ImGui::InputUnsigned("Sprite Order", &spriteOrder)) {
                    frame.spriteOrder = std::clamp<unsigned>(spriteOrder, 0, frame.spriteOrder.MASK);
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &SI::Frame::spriteOrder>(this);
                }

                if (ImGui::TreeNodeEx("Location", ImGuiTreeNodeFlags_DefaultOpen)) {
                    bool changed = false;
                    bool edited = false;

                    edited |= ImGui::Checkbox("Use Grid Location", &frame.location.useGridLocation);

                    if (frame.location.useGridLocation) {
                        changed |= ImGui::InputUpoint("Grid Location", &frame.location.gridLocation, usize(UINT8_MAX, UINT8_MAX));
                        edited |= ImGui::IsItemDeactivatedAfterEdit();

                        ImGui::LabelText("AABB", "%u, %u  %u x %u", frame.location.aabb.x, frame.location.aabb.y, frame.location.aabb.width, frame.location.aabb.height);
                    }
                    else {
                        ImGui::LabelText("Grid Location", " ");

                        const auto& imgSize = imageTexture().size();
                        const usize bounds = (imgSize.width != 0 && imgSize.height != 0) ? imgSize : usize(4096, 4096);

                        changed |= ImGui::InputUrect("AABB", &frame.location.aabb, bounds);
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                    }

                    edited |= ImGui::Checkbox("Use Grid Origin", &frame.location.useGridOrigin);

                    if (frame.location.useGridOrigin) {
                        ImGui::LabelText("Origin", "%u, %u", frame.location.origin.x, frame.location.origin.y);
                    }
                    else {
                        changed |= ImGui::InputUpoint("Origin", &frame.location.origin, frame.location.originRange());
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                    }

                    if (edited || changed) {
                        frame.location.update(_data.grid, frame);
                    }

                    if (edited) {
                        ListActions<AP::Frames>::selectedFieldEdited<
                            &SI::Frame::location>(this);
                    }

                    ImGui::TreePop();
                }

                if (ImGui::Checkbox("Solid Tile Hitbox", &frame.solid)) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &SI::Frame::solid>(this);
                }
                ImGui::Indent();
                if (frame.solid) {
                    ImGui::InputUrect("Tile Hitbox", &frame.tileHitbox, frameSize);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        ListActions<AP::Frames>::selectedFieldEdited<
                            &SI::Frame::tileHitbox>(this);
                    }
                }
                else {
                    ImGui::LabelText("Tile Hitbox", " ");
                }
                ImGui::Unindent();
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            {
                // ::TODO combine these into a single row with combined remove buttons::
                ImGui::PushID("Obj");
                ListButtons<AP::FrameObjects>(this);
                ImGui::PopID();
                ImGui::PushID("AP");
                ListButtons<AP::ActionPoints>(this);
                ImGui::PopID();
                ImGui::PushID("EH");
                ListButtons<AP::EntityHitboxes>(this);
                ImGui::PopID();

                ImGui::BeginChild("FC Scroll");

                // Indent required to prevent a glitch when the columns are resized
                ImGui::Indent();
                ImGui::Columns(3);
                ImGui::SetColumnWidth(0, 40);
                ImGui::SetColumnWidth(1, 150);
                ImGui::Unindent();
                ImGui::Columns(1);

                if (ImGui::TreeNodeEx("Objects", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
                    ImGui::Indent();
                    ImGui::Columns(3);
                    ImGui::PushID("Obj");

                    for (unsigned i = 0; i < frame.objects.size(); i++) {
                        auto& obj = frame.objects.at(i);

                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_frameObjectsSel, i);
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputUpoint("##location", &obj.location, frameSize, usize(obj.sizePx(), obj.sizePx()));
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        edited |= ImGui::EnumCombo("##size", &obj.size);
                        ImGui::NextColumn();

                        if (edited) {
                            ListActions<AP::FrameObjects>::selectedListItemEdited(this, i);
                        }

                        ImGui::PopID();
                    }

                    ImGui::PopID();
                    ImGui::Columns(1);
                    ImGui::Unindent();
                }

                if (ImGui::TreeNodeEx("Action Points", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
                    ImGui::Indent();
                    ImGui::Columns(3);
                    ImGui::PushID("AP");

                    for (unsigned i = 0; i < frame.actionPoints.size(); i++) {
                        auto& ap = frame.actionPoints.at(i);

                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_actionPointsSel, i);
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputUpoint("##location", &ap.location, frameSize);
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        edited |= ImGui::IdStringCombo("##type", &ap.type, projectFile.actionPointFunctions);
                        if (ImGui::IsItemHovered()) {
                            if (ap.type.isValid()) {
                                ImGui::BeginTooltip();
                                ImGui::TextUnformatted(ap.type);
                                ImGui::EndTooltip();
                            }
                        }
                        ImGui::NextColumn();

                        if (edited) {
                            ListActions<AP::ActionPoints>::selectedListItemEdited(this, i);
                        }

                        ImGui::PopID();
                    }

                    ImGui::PopID();
                    ImGui::Columns(1);
                    ImGui::Unindent();
                }

                if (ImGui::TreeNodeEx("Entity Hitboxes", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
                    ImGui::Indent();
                    ImGui::Columns(3);
                    ImGui::PushID("EH");

                    for (unsigned i = 0; i < frame.entityHitboxes.size(); i++) {
                        auto& eh = frame.entityHitboxes.at(i);

                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_entityHitboxesSel, i);
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputUrect("##aabb", &eh.aabb, frameSize);
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        edited |= ImGui::EntityHitboxTypeCombo("##type", &eh.hitboxType);
                        if (ImGui::IsItemHovered()) {
                            const std::string& toolTip = eh.hitboxType.to_long_string();
                            if (!toolTip.empty()) {
                                ImGui::BeginTooltip();
                                ImGui::TextUnformatted(toolTip);
                                ImGui::EndTooltip();
                            }
                        }
                        ImGui::NextColumn();

                        if (edited) {
                            ListActions<AP::EntityHitboxes>::selectedListItemEdited(this, i);
                        }

                        ImGui::PopID();
                    }

                    ImGui::PopID();
                    ImGui::Columns(1);
                    ImGui::Unindent();
                }

                ImGui::EndChild();
            }
        }
    }
    ImGui::End();
}

void SpriteImporterEditor::drawFrame(ImDrawList* drawList, const MetaSprite::SpriteImporter::Frame* frame)
{
    // ::TODO make layers optional::

    if (true) {
        unsigned i = frame->objects.size();
        while (i > 0) {
            i--;
            auto& obj = frame->objects.at(i);

            _graphics.addFixedSizeSquare(drawList, &obj.location, obj.sizePx(), frameObjectOutlineCol);
        }
    }

    if (true) {
        if (frame->solid) {
            _graphics.addRect(drawList, &frame->tileHitbox, tileHitboxOutlineCol);
        }
    }

    if (true) {
        unsigned i = frame->entityHitboxes.size();
        while (i > 0) {
            i--;
            auto& eh = frame->entityHitboxes.at(i);
            _graphics.addRect(drawList, &eh.aabb, entityHitboxOutlineCol);
        }
    }

    if (true) {
        unsigned i = frame->actionPoints.size();
        while (i > 0) {
            i--;
            auto& ap = frame->actionPoints.at(i);
            _graphics.addPointRect(drawList, &ap.location, actionPointOutlineCol);
        }
    }
}

void SpriteImporterEditor::drawSelectedFrame(ImDrawList* drawList, SI::Frame* frame)
{
    // ::TODO make layers optional::

    if (true) {
        unsigned i = frame->objects.size();
        while (i > 0) {
            i--;
            auto& obj = frame->objects.at(i);

            _graphics.addFixedSizeSquare(drawList, &obj.location, obj.sizePx(), frameObjectOutlineCol, &_frameObjectsSel, i);
            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::BeginTooltip();
                ImGui::Text("Object %u", i);
                ImGui::EndTooltip();
            }
        }
    }

    if (true) {
        if (frame->solid) {
            _graphics.addRect(drawList, &frame->tileHitbox, tileHitboxOutlineCol, &_tileHitboxSel, 1);
            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted("Tile Hitbox");
                ImGui::EndTooltip();
            }
        }
    }

    if (true) {
        unsigned i = frame->entityHitboxes.size();
        while (i > 0) {
            i--;
            auto& eh = frame->entityHitboxes.at(i);
            _graphics.addRect(drawList, &eh.aabb, entityHitboxOutlineCol, &_entityHitboxesSel, i);

            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::BeginTooltip();
                ImGui::Text("Entity Hitbox %u (%s)", i, eh.hitboxType.to_string().c_str());
                ImGui::EndTooltip();
            }
        }
    }

    if (true) {
        unsigned i = frame->actionPoints.size();
        while (i > 0) {
            i--;
            auto& ap = frame->actionPoints.at(i);
            _graphics.addPointRect(drawList, &ap.location, actionPointOutlineCol, &_actionPointsSel, i);

            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::BeginTooltip();
                if (ap.type.isValid()) {
                    ImGui::Text("Action Point %u (%s)", i, ap.type.str().c_str());
                }
                else {
                    ImGui::Text("Action Point %u", i);
                }
                ImGui::EndTooltip();
            }
        }
    }
}

void SpriteImporterEditor::frameEditorWindow()
{
    const Texture& texture = imageTexture();
    const rect graphicsRect(-IMAGE_PADDING, -IMAGE_PADDING, texture.width() + 2 * IMAGE_PADDING, texture.height() + 2 * IMAGE_PADDING);

    if (ImGui::Begin("Sprite Importer")) {
        ImGui::SetWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);

        // ::TODO add toolbar::
        ImGui::TextUnformatted("::TODO add toolbar::");

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        auto* drawList = ImGui::GetWindowDrawList();

        const SI::Frame* selectedFrame = _framesSel.selectedIndex() < _data.frames.size() ? &_data.frames.at(_framesSel.selectedIndex()) : nullptr;

        // The editable area changes depending on which frame is selected
        rect bounds;
        if (selectedFrame) {
            const auto& aabb = selectedFrame->location.aabb;

            bounds.x = aabb.x;
            bounds.y = aabb.y;
            bounds.width = std::max(MINIMIM_FRAME_SIZE, aabb.width);
            bounds.height = std::max(MINIMIM_FRAME_SIZE, aabb.height);
        }

        _graphics.startLoop("##Editor", graphicsRect, bounds, zoom,
                            &_tileHitboxSel, &_frameObjectsSel, &_actionPointsSel, &_entityHitboxesSel);

        // Select frame with double click
        {
            // Cannot use `ImGuiHoveredFlags_AllowWhenBlockedByActiveItem` as it returns true when I click on the scrollbar.
            const upoint mousePos = _graphics.mousePosUpoint();
            const bool mouseOverSelectedFrame = selectedFrame && selectedFrame->location.aabb.contains(mousePos);
            const bool windowHovered = ImGui::IsWindowHovered() || ImGui::IsItemActive();

            // Using double-click to select a frame as it easily allows me to determine
            // if we are selecting a frame or selecting/editing frame contents.
            const bool mouseDoubleClicked = ImGui::IsMouseDoubleClicked(0);

            if (windowHovered && not mouseOverSelectedFrame) {
                if (mouseDoubleClicked) {
                    _framesSel.clearSelection();
                }

                for (unsigned frameIndex = 0; frameIndex < _data.frames.size(); frameIndex++) {
                    auto& frame = _data.frames.at(frameIndex);

                    if (frame.location.aabb.contains(mousePos)) {
                        ImGui::BeginTooltip();
                        ImGui::TextUnformatted(frame.name);
                        ImGui::EndTooltip();

                        if (mouseDoubleClicked) {
                            _framesSel.setSelected(frameIndex);
                        }
                    }
                }
            }
        }

        _graphics.drawBackgroundColor(drawList, backgroundCol);

        _graphics.drawImage(drawList, texture, 0, 0);

        for (const auto& frame : _data.frames) {
            const auto& aabb = frame.location.aabb;
            const auto& origin = frame.location.origin;

            // Draw origin (behind frame contents)
            _graphics.drawCrosshair(drawList, aabb.x + origin.x, aabb.y + origin.y, aabb, originColor);

            _graphics.addRect(drawList, &aabb, frameOutlineCol);
        }

        for (unsigned frameIndex = 0; frameIndex < _data.frames.size(); frameIndex++) {
            auto& frame = _data.frames.at(frameIndex);

            _graphics.setOrigin(frame.location.aabb.x, frame.location.aabb.y);

            if (_framesSel.selectedIndex() != frameIndex) {
                drawFrame(drawList, &frame);
            }
            else {
                drawSelectedFrame(drawList, &frame);
            }
        }

        _graphics.setOrigin(0, 0);

        if (selectedFrame) {
            _graphics.drawAntiHighlight(drawList, selectedFrame->location.aabb, antiHighlightCol);
        }

        _graphics.endLoop(drawList,
                          &_tileHitboxSel, &_frameObjectsSel, &_actionPointsSel, &_entityHitboxesSel);

        if (_graphics.isEditingFinished()) {
            // ::TODO add action macros::
            if (_tileHitboxSel.isSelected()) {
                ListActions<AP::Frames>::selectedFieldEdited<&SI::Frame::tileHitbox>(this);
            }
            ListActions<AP::FrameObjects>::selectedItemsEdited(this);
            ListActions<AP::ActionPoints>::selectedItemsEdited(this);
            ListActions<AP::EntityHitboxes>::selectedItemsEdited(this);
        }

        ImGui::EndChild();
    }
    ImGui::End();
}

void SpriteImporterEditor::processGui(const Project::ProjectFile& projectFile)
{
    updateImageTexture();

    frameSetPropertiesWindow(projectFile);
    framePropertiesWindow(projectFile);

    frameEditorWindow();

    animationPropertiesWindow<AP>("Animations##SI", this, &_data);
    animationPreviewWindow<AP>("Animation Preview##SI", this, &_data);
    exportOrderWindow<AP>("Export Order##SI", this, &_data);
}

void SpriteImporterEditor::updateSelection()
{
    AbstractMetaSpriteEditor::updateSelection();

    if (_framesSel.isSelectionChanging()) {
        _tileHitboxSel.clearSelection();
    }

    _framesSel.update();

    _tileHitboxSel.update();

    _frameObjectsSel.update(_framesSel);
    _actionPointsSel.update(_framesSel);
    _entityHitboxesSel.update(_framesSel);
}

void SpriteImporterEditor::updateImageTexture()
{
    // ::TODO update the texture when the PNG file changes::

    if (_imageValid) {
        return;
    }

    auto image = ImageCache::loadPngImage(_data.imageFilename);
    assert(image);

    Texture& texture = imageTexture();

    if (image->dataSize() != 0) {
        texture.replace(*image);
    }
    else {
        texture.replaceWithMissingImageSymbol();
    }

    _imageValid = true;
    _transparentColorComboValid = false;
}

void SpriteImporterEditor::updateTransparentColorCombo()
{
    constexpr static int MAX_COLORS = 32;

    if (_transparentColorComboValid) {
        return;
    }
    const auto& image = ImageCache::loadPngImage(_data.imageFilename);

    _transparentColorCombo.clear();

    std::unordered_set<ImU32> colorSet;
    for (size_t i = 0; i < image->dataSize(); i++) {
        const auto pixel = image->data()[i];
        const ImU32 color = pixel.rgbaValue();

        bool newColor = colorSet.insert(color).second;
        if (newColor) {
            // ::TODO add rgba::toHexString function::
            char str[12];
            std::snprintf(str, IM_ARRAYSIZE(str), "#%06X", pixel.rgbHex());

            _transparentColorCombo.emplace_back(color, str);

            if (_transparentColorCombo.size() >= MAX_COLORS) {
                return;
            }
        }
    }

    _transparentColorComboValid = true;
}

}
