/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "spriteimporter-editor.h"
#include "gui/editor-actions.h"
#include "gui/imgui-combos.h"
#include "gui/imgui-filebrowser.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "gui/style.h"
#include "models/common/bit.h"
#include "models/common/imagecache.h"
#include "models/metasprite/metasprite-error.h"
#include <algorithm>
#include <unordered_set>

namespace UnTech::Gui {

namespace SI = UnTech::MetaSprite::SpriteImporter;
using ObjectSize = UnTech::MetaSprite::ObjectSize;

constexpr static unsigned MINIMIM_FRAME_SIZE = 16;

constexpr static int IMAGE_PADDING = 4;

// MetaSpriteEditor Action Policies
struct SpriteImporterEditorData::AP {
    struct FrameSet {
        using EditorT = SpriteImporterEditorData;
        using EditorDataT = SI::FrameSet;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
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

    struct ExportOrder : public FrameSet {
        constexpr static auto FieldPtr = &EditorDataT::exportOrder;
        constexpr static auto validFlag = &SpriteImporterEditorGui::_exportOrderValid;
    };

    struct Image : public FrameSet {
        constexpr static auto FieldPtr = &EditorDataT::imageFilename;
        constexpr static auto validFlag = &SpriteImporterEditorGui::_imageValid;
    };

    struct Frames : public FrameSet {
        using ListT = NamedList<SI::Frame>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::framesSel;

        constexpr static auto validFlag = &SpriteImporterEditorGui::_exportOrderValid;

        static ListT* getList(SI::FrameSet& fs) { return &fs.frames; }
    };

    // MUST BE used on any action that changes the frame list or changes the name of a frame.
    struct Frames_EditName final : public Frames {
        constexpr static auto validFlag = &SpriteImporterEditorGui::_exportOrderValid;
    };

    struct FrameObjects final : public FrameSet {
        using ListT = std::vector<SI::FrameObject>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        static constexpr const char* name = "Frame Object";
        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_FRAME_OBJECTS;

        constexpr static auto SelectionPtr = &EditorT::frameObjectsSel;

        static ListT* getList(SI::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &SI::Frame::objects);
        }
    };

    struct ActionPoints final : public FrameSet {
        using ListT = std::vector<SI::ActionPoint>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        static constexpr const char* name = "Action Point";
        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_ACTION_POINTS;

        constexpr static auto SelectionPtr = &EditorT::actionPointsSel;

        static ListT* getList(SI::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &SI::Frame::actionPoints);
        }
    };

    struct Animations : public FrameSet {
        using ListT = NamedList<UnTech::MetaSprite::Animation::Animation>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::animationsSel;

        static ListT* getList(SI::FrameSet& fs) { return &fs.animations; }
    };

    // MUST BE used on any action that changes the animation list or changes the name of an animation.
    struct Animations_EditName final : public Animations {
        constexpr static auto validFlag = &SpriteImporterEditorGui::_exportOrderValid;
    };

    struct AnimationFrames final : public FrameSet {
        using ListT = std::vector<UnTech::MetaSprite::Animation::AnimationFrame>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        // ::TODO replace with UnTech::MetaSprite::MAX_ANIMATION_FRAMES ::
        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::animationFramesSel;

        static ListT* getList(SI::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Animations::getList(fs), frameIndex,
                                &UnTech::MetaSprite::Animation::Animation::frames);
        }
    };
};

SpriteImporterEditorData::SpriteImporterEditorData(ItemIndex itemIndex)
    : AbstractMetaSpriteEditorData(itemIndex)
{
}

bool SpriteImporterEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;

    const auto i = itemIndex().index;
    if (i < projectFile.frameSets.size()) {
        auto& f = projectFile.frameSets.at(i);

        setFilename(f.filename);
        if (f.type == FrameSetType::SPRITE_IMPORTER) {
            if (f.siFrameSet) {
                data = *f.siFrameSet;
                return true;
            }
        }
    }

    return false;
}

void SpriteImporterEditorData::saveFile() const
{
    assert(!filename().empty());
    UnTech::MetaSprite::SpriteImporter::saveFrameSet(data, filename());
}

void SpriteImporterEditorData::errorDoubleClicked(const AbstractError* error)
{
    using Type = MetaSprite::MsErrorType;

    animationsSel.clearSelection();
    animationFramesSel.clearSelection();

    framesSel.clearSelection();

    tileHitboxSel.clearSelection();
    shieldSel.clearSelection();
    hitboxSel.clearSelection();
    hurtboxSel.clearSelection();
    frameObjectsSel.clearSelection();
    actionPointsSel.clearSelection();

    if (auto* e = dynamic_cast<const MetaSprite::MetaSpriteError*>(error)) {
        switch (e->type) {
        case Type::FRAME:
            framesSel.setSelected(e->firstIndex);
            break;

        case Type::ANIMATION:
            animationsSel.setSelected(e->firstIndex);
            break;

        case Type::ANIMATION_FRAME:
            animationsSel.setSelected(e->firstIndex);
            animationFramesSel.setSelected(e->firstIndex, e->childIndex);
            break;

        case Type::FRAME_OBJECT:
            framesSel.setSelected(e->firstIndex);
            frameObjectsSel.setSelected(e->firstIndex, e->childIndex);
            break;

        case Type::ACTION_POINT:
            framesSel.setSelected(e->firstIndex);
            actionPointsSel.setSelected(e->firstIndex, e->childIndex);
            break;

        case Type::TILE_HITBOX:
            framesSel.setSelected(e->firstIndex);
            tileHitboxSel.setSelected(true);
            break;

        case Type::SHIELD:
            framesSel.setSelected(e->firstIndex);
            shieldSel.setSelected(true);
            break;

        case Type::HIT_BOX:
            framesSel.setSelected(e->firstIndex);
            hitboxSel.setSelected(true);
            break;

        case Type::HURT_BOX:
            framesSel.setSelected(e->firstIndex);
            hurtboxSel.setSelected(true);
            break;
        }
    }
}

void SpriteImporterEditorData::updateSelection()
{
    AbstractMetaSpriteEditorData::updateSelection();

    if (framesSel.isSelectionChanging()) {
        tileHitboxSel.clearSelection();
        shieldSel.clearSelection();
        hitboxSel.clearSelection();
        hurtboxSel.clearSelection();
    }

    framesSel.update();

    frameObjectsSel.update(framesSel);
    actionPointsSel.update(framesSel);

    tileHitboxSel.update();
    shieldSel.update();
    hitboxSel.update();
    hurtboxSel.update();
}

SpriteImporterEditorGui::SpriteImporterEditorGui()
    : AbstractMetaSpriteEditorGui()
    , _data(nullptr)
    , _graphics()
    , _imageTexture()
    , _transparentColorCombo()
    , _imageValid(false)
    , _transparentColorComboValid(false)
{
}

bool SpriteImporterEditorGui::setEditorData(AbstractEditorData* data)
{
    _data = dynamic_cast<SpriteImporterEditorData*>(data);
    setMetaSpriteData(_data);

    return _data;
}

void SpriteImporterEditorGui::resetState()
{
    AbstractMetaSpriteEditorGui::resetState();

    _imageValid = false;
    _transparentColorComboValid = false;

    _graphics.resetState();
}

void SpriteImporterEditorGui::editorClosed()
{
}

void SpriteImporterEditorGui::addFrame(const idstring& name)
{
    assert(_data);

    SI::Frame frame;
    frame.name = name;
    ListActions<AP::Frames_EditName>::addItemToSelectedList(_data, frame);
}

void SpriteImporterEditorGui::addAnimation(const idstring& name)
{
    assert(_data);

    MetaSprite::Animation::Animation animation;
    animation.name = name;
    ListActions<AP::Animations_EditName>::addItemToSelectedList(_data, animation);
}

void SpriteImporterEditorGui::frameSetPropertiesWindow(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& fs = _data->data;

    ImGui::SetNextWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("FrameSet##SI")) {

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);

        {
            ImGui::InputIdstring("Name", &fs.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &SI::FrameSet::name>(_data);
            }

            if (ImGui::EnumCombo("Tileset Type", &fs.tilesetType)) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &SI::FrameSet::tilesetType>(_data);
            }

            if (ImGui::IdStringCombo("Export Order", &fs.exportOrder, projectFile.frameSetExportOrders)) {
                EditorFieldActions<AP::ExportOrder>::fieldEdited(_data);
            }

            if (ImGui::InputPngImageFilename("Image", &fs.imageFilename)) {
                EditorFieldActions<AP::Image>::fieldEdited(_data);
            }

            {
                bool edited;

                ImColor c(fs.transparentColor.rgb());

                char tcString[16];
                std::snprintf(tcString, IM_ARRAYSIZE(tcString), "#%06X", fs.transparentColor.rgbHex());

                if (ImGui::BeginCombo("Transparent Color", tcString)) {
                    updateTransparentColorCombo();

                    for (const auto& [col, str] : _transparentColorCombo) {
                        ImGui::PushStyleColor(ImGuiCol_Text, col);
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, col);
                        if (ImGui::Selectable(str.c_str())) {
                            fs.transparentColor = rgba::fromRgba(col);
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
                    fs.transparentColor = rgba::fromRgba(c);
                    fs.transparentColor.alpha = 0xff;
                }
                edited = ImGui::IsItemDeactivatedAfterEdit();

                if (edited) {
                    EditorActions<AP::FrameSet>::fieldEdited<
                        &SI::FrameSet::transparentColor>(_data);
                }
            }

            if (ImGui::TreeNodeEx("Grid", ImGuiTreeNodeFlags_DefaultOpen)) {
                bool edited = false;

                auto& grid = fs.grid;

                ImGui::InputUsize("Frame Size", &grid.frameSize, usize(SI::MAX_FRAME_SIZE, SI::MAX_FRAME_SIZE));
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::InputUpoint("Offset", &grid.offset, usize(511, 511));
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::InputUpoint("Padding", &grid.padding, usize(511, 511));
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::InputUpoint("Origin", &grid.origin, grid.originRange());
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                if (edited) {
                    for (auto& frame : fs.frames) {
                        frame.location.update(grid, frame);
                    }

                    _data->startMacro();
                    EditorActions<AP::FrameSet>::fieldEdited<
                        &SI::FrameSet::grid>(_data);
                    ListActions<AP::Frames>::allItemsInSelectedListFieldEdited<
                        &SI::Frame::location>(_data);
                    _data->endMacro();
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Palette", ImGuiTreeNodeFlags_DefaultOpen)) {
                bool changed = false;
                bool edited = false;

                auto& palette = fs.palette;

                bool usePalette = palette.usesUserSuppliedPalette();
                edited |= ImGui::Checkbox("User Supplied Palette", &usePalette);

                if (usePalette) {
                    changed |= ImGui::EnumCombo("Position", &palette.position);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    changed |= ImGui::InputUnsigned("No of Palettes", &palette.nPalettes);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    changed |= ImGui::InputUnsigned("Color Size", &palette.colorSize);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();
                }

                if (changed || edited) {
                    if (usePalette) {
                        palette.nPalettes = std::max(1U, palette.nPalettes);
                        palette.colorSize = std::max(1U, palette.colorSize);
                    }
                    else {
                        palette.nPalettes = 0;
                        palette.colorSize = 0;
                    }
                }

                if (edited) {
                    EditorActions<AP::FrameSet>::fieldEdited<
                        &SI::FrameSet::palette>(_data);
                }

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

template <auto FieldPtr>
void SpriteImporterEditorGui::collisionBox(const char* label, SI::Frame& frame, const usize& frameSize, ToggleSelection* sel)
{
    assert(_data);
    SI::CollisionBox& cb = frame.*FieldPtr;

    const auto style = ImGui::GetStyle();

    if (ImGui::Checkbox(label, &cb.exists)) {
        ListActions<AP::Frames>::selectedFieldEdited<FieldPtr>(_data);
    }
    if (cb.exists) {
        ImGui::PushID(label);

        ImGui::Selectable("##sel", sel, ImGuiSelectableFlags_AllowItemOverlap);
        ImGui::SameLine(style.IndentSpacing * 2);

        ImGui::SetNextItemWidth(-1);
        ImGui::InputUrect("##aabb", &cb.aabb, frameSize);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            ListActions<AP::Frames>::selectedFieldEdited<FieldPtr>(_data);
        }

        ImGui::PopID();
    }
}

void SpriteImporterEditorGui::framePropertiesWindow(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& fs = _data->data;

    ImGui::SetNextWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Frames##SI")) {

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);

        ListButtons<AP::Frames_EditName>(_data);

        ImGui::SetNextItemWidth(-1);
        ImGui::NamedListListBox("##FrameList", &_data->framesSel, fs.frames, 8);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (_data->framesSel.selectedIndex() < fs.frames.size()) {
            SI::Frame& frame = fs.frames.at(_data->framesSel.selectedIndex());

            const usize frameSize = frame.location.aabb.size();

            {
                ImGui::InputIdstring("Name", &frame.name);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<AP::Frames_EditName>::selectedFieldEdited<
                        &SI::Frame::name>(_data);
                }

                unsigned spriteOrder = frame.spriteOrder;
                if (ImGui::InputUnsigned("Sprite Order", &spriteOrder)) {
                    frame.spriteOrder = clamp<unsigned>(spriteOrder, 0, frame.spriteOrder.MASK);
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &SI::Frame::spriteOrder>(_data);
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

                        const auto& imgSize = _imageTexture.size();
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
                        frame.location.update(fs.grid, frame);
                    }

                    if (edited) {
                        ListActions<AP::Frames>::selectedFieldEdited<
                            &SI::Frame::location>(_data);
                    }

                    ImGui::TreePop();
                }
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            {
                CombinedListButtons<AP::FrameObjects, AP::ActionPoints>("FC_Buttons", _data);

                ImGui::BeginChild("FC Scroll");

                collisionBox<&SI::Frame::tileHitbox>("Tile Hitbox", frame, frameSize, &_data->tileHitboxSel);
                collisionBox<&SI::Frame::shield>("Shield", frame, frameSize, &_data->shieldSel);
                collisionBox<&SI::Frame::hitbox>("Entity Hitbox", frame, frameSize, &_data->hitboxSel);
                collisionBox<&SI::Frame::hurtbox>("Entity Hurtbox", frame, frameSize, &_data->hurtboxSel);

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

                    for (auto [i, obj] : enumerate(frame.objects)) {
                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_data->frameObjectsSel, i);
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputUpoint("##location", &obj.location, frameSize, usize(obj.sizePx(), obj.sizePx()));
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        edited |= ImGui::EnumCombo("##size", &obj.size);
                        ImGui::NextColumn();

                        if (edited) {
                            ListActions<AP::FrameObjects>::selectedListItemEdited(_data, i);
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

                    for (auto [i, ap] : enumerate(frame.actionPoints)) {
                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_data->actionPointsSel, i);
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
                            ListActions<AP::ActionPoints>::selectedListItemEdited(_data, i);
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

// NOTE: zoom will be negative if the frame is flipped
void SpriteImporterEditorGui::drawAnimationFrame(const ImVec2& drawPos, ImVec2 zoom, const SI::Frame& frame) const
{
    assert(_data);
    const auto& fs = _data->data;

    const float lineThickness = AabbGraphics::lineThickness;

    const ImVec2 pos(drawPos.x - frame.location.origin.x * zoom.x, drawPos.y - frame.location.origin.y * zoom.y);

    auto* drawList = ImGui::GetWindowDrawList();

    {
        const ImVec2 offset = ImGui::GetWindowPos();
        const ImVec2 size = ImGui::GetWindowSize();
        drawList->AddRectFilled(offset, offset + size, fs.transparentColor.rgbaValue());
    }

    if (showFrameObjects) {
        const auto textureId = _imageTexture.imguiTextureId();

        const ImVec2 uv(1.0f / _imageTexture.width(), 1.0f / _imageTexture.height());

        for (auto [i, obj] : reverse_enumerate(frame.objects)) {
            const unsigned x = frame.location.aabb.x + obj.location.x;
            const unsigned y = frame.location.aabb.y + obj.location.y;

            const ImVec2 uv0(x * uv.x, y * uv.y);
            const ImVec2 uv1((x + obj.sizePx()) * uv.x, (y + obj.sizePx()) * uv.y);

            ImVec2 p1(pos.x + obj.location.x * zoom.x, pos.y + obj.location.y * zoom.y);
            ImVec2 p2(p1.x + obj.sizePx() * zoom.x, p1.y + obj.sizePx() * zoom.y);
            drawList->AddImage(textureId, p1, p2, uv0, uv1);
        }
    }

    auto drawCollisionBox = [&](const SI::CollisionBox& box, const bool showFlag, const ImU32 outlineColor) {
        if (showFlag && box.exists) {
            ImVec2 p1(pos.x + box.aabb.x * zoom.x, pos.y + box.aabb.y * zoom.y);
            ImVec2 p2(pos.x + box.aabb.right() * zoom.x, pos.y + box.aabb.bottom() * zoom.y);
            drawList->AddRect(p1, p2, outlineColor, lineThickness);
        }
    };
    drawCollisionBox(frame.tileHitbox, showTileHitbox, Style::tileHitboxOutlineColor);
    drawCollisionBox(frame.shield, showShield, Style::shieldOutlineColor);
    drawCollisionBox(frame.hitbox, showHitbox, Style::hitboxOutlineColor);
    drawCollisionBox(frame.hurtbox, showHurtbox, Style::hurtboxOutlineColor);

    if (showActionPoints) {
        for (auto [i, ap] : reverse_enumerate(frame.actionPoints)) {
            ImVec2 p1(pos.x + ap.location.x * zoom.x, pos.y + ap.location.y * zoom.y);
            ImVec2 p2(p1.x + zoom.x, p1.y + zoom.y);
            drawList->AddRect(p1, p2, Style::actionPointOutlineColor, lineThickness);
        }
    }
}

void SpriteImporterEditorGui::drawFrame(ImDrawList* drawList, const MetaSprite::SpriteImporter::Frame* frame)
{
    assert(_data);

    if (showFrameObjects) {
        for (auto [i, obj] : reverse_enumerate(frame->objects)) {
            _graphics.addFixedSizeSquare(drawList, &obj.location, obj.sizePx(), Style::frameObjectOutlineColor);
        }
    }

    auto drawCollisionBox = [&](const SI::CollisionBox* box, const bool showFlag, const ImU32 outlineColor) {
        if (showFlag && box->exists) {
            _graphics.addRect(drawList, &box->aabb, outlineColor);
        }
    };
    drawCollisionBox(&frame->tileHitbox, showTileHitbox, Style::tileHitboxOutlineColor);
    drawCollisionBox(&frame->shield, showShield, Style::shieldOutlineColor);
    drawCollisionBox(&frame->hitbox, showHitbox, Style::hitboxOutlineColor);
    drawCollisionBox(&frame->hurtbox, showHurtbox, Style::hurtboxOutlineColor);

    if (showActionPoints) {
        for (auto [i, ap] : reverse_enumerate(frame->actionPoints)) {
            _graphics.addPointRect(drawList, &ap.location, Style::actionPointOutlineColor);
        }
    }
}

void SpriteImporterEditorGui::drawSelectedFrame(ImDrawList* drawList, SI::Frame* frame)
{
    assert(_data);

    if (showFrameObjects) {
        for (auto [i, obj] : reverse_enumerate(frame->objects)) {
            _graphics.addFixedSizeSquare(drawList, &obj.location, obj.sizePx(), Style::frameObjectOutlineColor, &_data->frameObjectsSel, i);
            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::BeginTooltip();
                ImGui::Text("Object %u", unsigned(i));
                ImGui::EndTooltip();
            }
        }
    }

    auto drawCollisionBox = [&](SI::CollisionBox* box, ToggleSelection* sel, const bool showFlag, const ImU32 outlineColor, std::string_view toolTip) {
        if (showFlag && box->exists) {
            _graphics.addRect(drawList, &box->aabb, outlineColor, sel, 1);
            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(toolTip);
                ImGui::EndTooltip();
            }
        }
    };
    drawCollisionBox(&frame->tileHitbox, &_data->tileHitboxSel, showTileHitbox, Style::tileHitboxOutlineColor, "Tile Hitbox");
    drawCollisionBox(&frame->shield, &_data->shieldSel, showShield, Style::shieldOutlineColor, "Shield Box");
    drawCollisionBox(&frame->hitbox, &_data->hitboxSel, showHitbox, Style::hitboxOutlineColor, "Hitbox");
    drawCollisionBox(&frame->hurtbox, &_data->hurtboxSel, showHurtbox, Style::hurtboxOutlineColor, "Hurtbox");

    if (showActionPoints) {
        for (auto [i, ap] : reverse_enumerate(frame->actionPoints)) {
            _graphics.addPointRect(drawList, &ap.location, Style::actionPointOutlineColor, &_data->actionPointsSel, i);

            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::BeginTooltip();
                if (ap.type.isValid()) {
                    ImGui::Text("Action Point %u (%s)", unsigned(i), ap.type.c_str());
                }
                else {
                    ImGui::Text("Action Point %u", unsigned(i));
                }
                ImGui::EndTooltip();
            }
        }
    }
}

void SpriteImporterEditorGui::frameEditorWindow()
{
    assert(_data);
    auto& frames = _data->data.frames;

    const rect graphicsRect(-IMAGE_PADDING, -IMAGE_PADDING, _imageTexture.width() + 2 * IMAGE_PADDING, _imageTexture.height() + 2 * IMAGE_PADDING);

    ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Sprite Importer")) {

        {
            undoStackButtons();
            ImGui::SameLine(0.0f, 12.0f);

            showLayerButtons();
            ImGui::SameLine();

            Style::spriteImporterZoom.zoomCombo("##zoom");
        }

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        auto* drawList = ImGui::GetWindowDrawList();

        const SI::Frame* selectedFrame = _data->framesSel.selectedIndex() < frames.size() ? &frames.at(_data->framesSel.selectedIndex()) : nullptr;

        // The editable area changes depending on which frame is selected
        rect bounds;
        if (selectedFrame) {
            const auto& aabb = selectedFrame->location.aabb;

            bounds.x = aabb.x;
            bounds.y = aabb.y;
            bounds.width = std::max(MINIMIM_FRAME_SIZE, aabb.width);
            bounds.height = std::max(MINIMIM_FRAME_SIZE, aabb.height);
        }

        _graphics.startLoop("##Editor", graphicsRect, bounds, Style::spriteImporterZoom.zoom(),
                            &_data->frameObjectsSel, &_data->actionPointsSel,
                            &_data->tileHitboxSel, &_data->shieldSel, &_data->hitboxSel, &_data->hurtboxSel);

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
                    _data->framesSel.clearSelection();
                }

                for (auto [frameIndex, frame] : const_enumerate(frames)) {
                    if (frame.location.aabb.contains(mousePos)) {
                        ImGui::BeginTooltip();
                        ImGui::TextUnformatted(frame.name);
                        ImGui::EndTooltip();

                        if (mouseDoubleClicked) {
                            _data->framesSel.setSelected(frameIndex);
                        }
                    }
                }
            }
        }

        _graphics.drawBackgroundColor(drawList, Style::spriteImporterBackgroundColor);

        _graphics.drawImage(drawList, _imageTexture, 0, 0);

        for (const auto& frame : frames) {
            const auto& aabb = frame.location.aabb;
            const auto& origin = frame.location.origin;

            // Draw origin (behind frame contents)
            _graphics.drawCrosshair(drawList, aabb.x + origin.x, aabb.y + origin.y, aabb, Style::spriteImporterOriginColor);

            _graphics.addRect(drawList, &aabb, Style::frameOutlineColor);
        }

        for (auto [frameIndex, frame] : enumerate(frames)) {
            _graphics.setOrigin(frame.location.aabb.x, frame.location.aabb.y);

            if (_data->framesSel.selectedIndex() != frameIndex) {
                drawFrame(drawList, &frame);
            }
            else {
                drawSelectedFrame(drawList, &frame);
            }
        }

        _graphics.setOrigin(0, 0);

        if (selectedFrame) {
            _graphics.drawAntiHighlight(drawList, selectedFrame->location.aabb, Style::antiHighlightColor);
        }

        _graphics.endLoop(drawList,
                          &_data->frameObjectsSel, &_data->actionPointsSel,
                          &_data->tileHitboxSel, &_data->shieldSel, &_data->hitboxSel, &_data->hurtboxSel);

        if (_graphics.isEditingFinished()) {
            _data->startMacro();

            ListActions<AP::FrameObjects>::selectedItemsEdited(_data);
            ListActions<AP::ActionPoints>::selectedItemsEdited(_data);
            if (_data->tileHitboxSel.isSelected()) {
                ListActions<AP::Frames>::selectedFieldEdited<&SI::Frame::tileHitbox>(_data);
            }
            if (_data->shieldSel.isSelected()) {
                ListActions<AP::Frames>::selectedFieldEdited<&SI::Frame::shield>(_data);
            }
            if (_data->hitboxSel.isSelected()) {
                ListActions<AP::Frames>::selectedFieldEdited<&SI::Frame::hitbox>(_data);
            }
            if (_data->hurtboxSel.isSelected()) {
                ListActions<AP::Frames>::selectedFieldEdited<&SI::Frame::hurtbox>(_data);
            }

            _data->endMacro();
        }

        Style::spriteImporterZoom.processMouseWheel();

        ImGui::EndChild();
    }
    ImGui::End();
}

void SpriteImporterEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }
    auto& fs = _data->data;

    updateImageTexture();

    updateExportOderTree(fs, projectFile);
    frameSetPropertiesWindow(projectFile);
    framePropertiesWindow(projectFile);

    frameEditorWindow();

    animationPropertiesWindow<AP>("Animations##SI", _data, &fs);
    animationPreviewWindow("Animation Preview##SI", _data, [this](auto... args) { drawAnimationFrame(args...); });
    exportOrderWindow("Export Order##SI");
}

void SpriteImporterEditorGui::updateImageTexture()
{
    assert(_data);
    auto& fs = _data->data;

    if (_imageValid) {
        return;
    }

    _imageTexture.loadPngImage(fs.imageFilename);

    _imageValid = true;
    _transparentColorComboValid = false;
}

void SpriteImporterEditorGui::updateTransparentColorCombo()
{
    constexpr static int MAX_COLORS = 32;

    assert(_data);
    auto& fs = _data->data;

    if (_transparentColorComboValid) {
        return;
    }
    const auto& image = ImageCache::loadPngImage(fs.imageFilename);

    _transparentColorCombo.clear();

    std::unordered_set<ImU32> colorSet;
    for (const auto& pixel : *image) {
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
