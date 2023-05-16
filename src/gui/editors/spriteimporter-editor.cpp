/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "spriteimporter-editor.h"
#include "gui/aptable.h"
#include "gui/editor-actions.h"
#include "gui/imgui-filebrowser.h"
#include "gui/imgui.h"
#include "gui/splitter.hpp"
#include "gui/style.h"
#include "models/common/bit.h"
#include "models/common/imagecache.h"
#include "models/metasprite/metasprite-error.h"
#include "vendor/imgui/imgui.h"
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

static usize originRange(const usize& frameSize)
{
    return {
        std::min(SI::MAX_ORIGIN, frameSize.width),
        std::min(SI::MAX_ORIGIN, frameSize.height)
    };
}

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
    : AbstractMetaSpriteEditorGui("##SI editor")
    , _data(nullptr)
    , _graphics()
    , _imageTexture()
    , _transparentColorCombo()
    , _sidebar{ 550, 400, 250 }
    , _imageValid(false)
    , _transparentColorComboValid(false)
{
}

bool SpriteImporterEditorGui::setEditorData(const std::shared_ptr<AbstractEditorData>& data)
{
    _data = std::dynamic_pointer_cast<SpriteImporterEditorData>(data);
    return _data != nullptr;
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

void SpriteImporterEditorGui::frameSetPropertiesGui(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& fs = _data->data;

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);

    {
        if (Cell("Name", &fs.name)) {
            EditorActions<AP::FrameSet>::fieldEdited<
                &SI::FrameSet::name>(_data);
        }

        if (Cell("Tileset Type", &fs.tilesetType)) {
            EditorActions<AP::FrameSet>::fieldEdited<
                &SI::FrameSet::tilesetType>(_data);
        }

        if (Cell("Export Order", &fs.exportOrder, projectFile.frameSetExportOrders)) {
            EditorFieldActions<AP::ExportOrder>::fieldEdited(_data);
        }

        if (ImGui::InputPngImageFilename("Image", &fs.imageFilename)) {
            EditorFieldActions<AP::Image>::fieldEdited(_data);
        }

        {
            ImColor c(fs.transparentColor.rgb());

            const std::u8string tcString = fs.transparentColor.rgbHexString();

            if (ImGui::BeginCombo("Transparent Color", u8Cast(tcString))) {
                updateTransparentColorCombo();

                for (const auto& [col, str] : _transparentColorCombo) {
                    ImGui::PushStyleColor(ImGuiCol_Text, col);
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, col);
                    if (ImGui::Selectable(u8Cast(str))) {
                        fs.transparentColor = rgba::fromRgba(col);
                    }
                    ImGui::PopStyleColor(2);

                    if (ImGui::IsItemHovered()) {
                        ImGui::ShowTooltip(str);
                    }
                }

                ImGui::EndCombo();
            }

            if (ImGui::ColorEdit3("##ColorEdit", &c)) {
                fs.transparentColor = rgba::fromRgba(c);
                fs.transparentColor.alpha = 0xff;
            }
            const bool edited = ImGui::IsItemDeactivatedAfterEdit();

            if (edited) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &SI::FrameSet::transparentColor>(_data);
            }
        }

        if (ImGui::TreeNodeEx("Grid", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool edited = false;

            auto& grid = fs.grid;

            edited |= Cell("Frame Size", &grid.frameSize, usize(SI::MAX_FRAME_SIZE, SI::MAX_FRAME_SIZE));
            edited |= Cell("Offset", &grid.offset, usize(511, 511));
            edited |= Cell("Padding", &grid.padding, usize(511, 511));
            edited |= Cell("Origin", &grid.origin, originRange(grid.frameSize));

            if (edited) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &SI::FrameSet::grid>(_data);
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Palette", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool edited = false;

            auto& palette = fs.palette;

            bool usePalette = palette.usesUserSuppliedPalette();
            edited |= Cell("User Supplied Palette", &usePalette);

            if (usePalette) {
                edited |= Cell("Position", &palette.position);
                edited |= Cell("No of Palettes", &palette.nPalettes);
                edited |= Cell("Color Size", &palette.colorSize);

                if (palette.nPalettes == 0) {
                    palette.nPalettes = 1;
                }
                if (palette.colorSize == 0) {
                    palette.colorSize = 1;
                }
            }

            if (edited) {
                if (usePalette == false) {
                    palette.nPalettes = 0;
                    palette.colorSize = 0;
                }

                EditorActions<AP::FrameSet>::fieldEdited<
                    &SI::FrameSet::palette>(_data);
            }

            ImGui::TreePop();
        }
    }
}

template <auto FieldPtr>
void SpriteImporterEditorGui::collisionBox(const char* label, SI::Frame& frame, const usize& frameSize, ToggleSelection* sel)
{
    assert(_data);
    SI::CollisionBox& cb = frame.*FieldPtr;

    const auto style = ImGui::GetStyle();

    if (Cell(label, &cb.exists)) {
        ListActions<AP::Frames>::selectedFieldEdited<FieldPtr>(_data);
    }
    if (cb.exists) {
        ImGui::PushID(label);

        ImGui::Selectable("##sel", sel, ImGuiSelectableFlags_AllowItemOverlap);
        ImGui::SameLine(style.IndentSpacing * 2);

        ImGui::SetNextItemWidth(-1);
        if (Cell("##aabb", &cb.aabb, frameSize)) {
            ListActions<AP::Frames>::selectedFieldEdited<FieldPtr>(_data);
        }

        ImGui::PopID();
    }
}

void SpriteImporterEditorGui::framePropertiesGui(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& fs = _data->data;

    NamedListSidebar<AP::Frames_EditName>(_data);

    ImGui::SameLine();
    ImGui::BeginChild("Scroll");

    if (_data->framesSel.selectedIndex() < fs.frames.size()) {
        SI::Frame& frame = fs.frames.at(_data->framesSel.selectedIndex());

        const urect frameAabb = frame.frameLocation(fs.grid);
        const usize frameSize = frameAabb.size();

        {
            if (Cell("Name", &frame.name)) {
                ListActions<AP::Frames_EditName>::selectedFieldEdited<
                    &SI::Frame::name>(_data);
            }

            if (Cell("Sprite Order", &frame.spriteOrder)) {
                ListActions<AP::Frames>::selectedFieldEdited<
                    &SI::Frame::spriteOrder>(_data);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        {
            bool useGridLocation = !frame.locationOverride.has_value();

            bool loEdited = Cell("Use Grid Location", &useGridLocation);
            if (loEdited) {
                if (useGridLocation) {
                    frame.locationOverride = std::nullopt;
                }
                else {
                    frame.locationOverride = frameAabb;
                }
            }

            if (!frame.locationOverride.has_value()) {
                if (Cell("Grid Location", &frame.gridLocation, usize(UINT8_MAX, UINT8_MAX))) {
                    ListActions<AP::Frames_EditName>::selectedFieldEdited<&SI::Frame::gridLocation>(_data);
                }

                ImGui::LabelText("AABB", "%u, %u  %u x %u", frameAabb.x, frameAabb.y, frameAabb.width, frameAabb.height);
            }
            else {
                ImGui::LabelText("Grid Location", " ");

                const auto& imgSize = _imageTexture.size();
                const usize bounds = (imgSize.width != 0 && imgSize.height != 0) ? imgSize : usize(4096, 4096);

                loEdited |= Cell("AABB", &frame.locationOverride.value(), bounds);
            }

            if (loEdited) {
                ListActions<AP::Frames>::selectedFieldEdited<
                    &SI::Frame::locationOverride>(_data);
            }
        }

        {
            bool useGridOrigin = !frame.originOverride.has_value();

            bool edited = Cell("Use Grid Origin", &useGridOrigin);
            if (edited) {
                if (useGridOrigin) {
                    frame.originOverride = std::nullopt;
                }
                else {
                    frame.originOverride = fs.grid.origin;
                }
            }

            if (frame.originOverride.has_value() == false) {
                ImGui::LabelText("Origin", "%u, %u", fs.grid.origin.x, fs.grid.origin.y);
            }
            else {
                edited |= Cell("Origin", &frame.originOverride.value(), originRange(frameAabb.size()));
            }

            if (edited) {
                ListActions<AP::Frames>::selectedFieldEdited<
                    &SI::Frame::originOverride>(_data);
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        collisionBox<&SI::Frame::tileHitbox>("Tile Hitbox", frame, frameSize, &_data->tileHitboxSel);
        collisionBox<&SI::Frame::shield>("Shield", frame, frameSize, &_data->shieldSel);
        collisionBox<&SI::Frame::hitbox>("Entity Hitbox", frame, frameSize, &_data->hitboxSel);
        collisionBox<&SI::Frame::hurtbox>("Entity Hurtbox", frame, frameSize, &_data->hurtboxSel);

        ImGui::Spacing();
        {
            ImGui::TextUnformatted(u8"Objects:");
            ImGui::Indent();

            apTable_noScrolling<AP::FrameObjects>(
                "Objects", _data,
                std::to_array({ "Location", "Size" }),

                [&](auto& obj) { return Cell("##location", &obj.location, frameSize, usize(obj.sizePx(), obj.sizePx())); },
                [&](auto& obj) { return Cell("##size", &obj.size); });

            ImGui::Unindent();
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted(u8"Action Points:");
            ImGui::Indent();

            apTable_noScrolling<AP::ActionPoints>(
                "AP", _data,
                std::to_array({ "Location", "Type" }),

                [&](auto& ap) { return Cell("##location", &ap.location, frameSize); },
                [&](auto& ap) { return Cell("##type", &ap.type, projectFile.actionPointFunctions); });

            ImGui::Unindent();
        }
    }

    ImGui::EndChild();
}

// NOTE: zoom will be negative if the frame is flipped
void SpriteImporterEditorGui::drawAnimationFrame(const ImVec2& drawPos, ImVec2 zoom, const SI::Frame& frame) const
{
    assert(_data);
    const auto& fs = _data->data;

    const auto frameAabb = frame.frameLocation(fs.grid);
    const auto origin = frame.origin(fs.grid);

    const float lineThickness = AabbGraphics::lineThickness;

    const ImVec2 pos(drawPos.x - origin.x * zoom.x, drawPos.y - origin.y * zoom.y);

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
            const unsigned x = frameAabb.x + obj.location.x;
            const unsigned y = frameAabb.y + obj.location.y;

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
                ImGui::ShowTooltipFmt("Object %u", unsigned(i));
            }
        }
    }

    auto drawCollisionBox = [&](SI::CollisionBox* box, ToggleSelection* sel, const bool showFlag, const ImU32 outlineColor, std::u8string_view toolTip) {
        if (showFlag && box->exists) {
            _graphics.addRect(drawList, &box->aabb, outlineColor, sel, 1);
            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::ShowTooltip(toolTip);
            }
        }
    };
    drawCollisionBox(&frame->tileHitbox, &_data->tileHitboxSel, showTileHitbox, Style::tileHitboxOutlineColor, u8"Tile Hitbox");
    drawCollisionBox(&frame->shield, &_data->shieldSel, showShield, Style::shieldOutlineColor, u8"Shield Box");
    drawCollisionBox(&frame->hitbox, &_data->hitboxSel, showHitbox, Style::hitboxOutlineColor, u8"Hitbox");
    drawCollisionBox(&frame->hurtbox, &_data->hurtboxSel, showHurtbox, Style::hurtboxOutlineColor, u8"Hurtbox");

    if (showActionPoints) {
        for (auto [i, ap] : reverse_enumerate(frame->actionPoints)) {
            _graphics.addPointRect(drawList, &ap.location, Style::actionPointOutlineColor, &_data->actionPointsSel, i);

            if (_graphics.isHoveredAndNotEditing()) {
                if (ap.type.isValid()) {
                    ImGui::ShowTooltipFmt("Action Point %u (%s)", unsigned(i), u8Cast(ap.type));
                }
                else {
                    ImGui::ShowTooltipFmt("Action Point %u", unsigned(i));
                }
            }
        }
    }
}

void SpriteImporterEditorGui::frameEditorGui()
{
    assert(_data);
    auto& fs = _data->data;
    auto& frames = fs.frames;

    const rect graphicsRect(-IMAGE_PADDING, -IMAGE_PADDING, _imageTexture.width() + 2 * IMAGE_PADDING, _imageTexture.height() + 2 * IMAGE_PADDING);

    {
        undoStackButtons();
        ImGui::SameLine(0.0f, 12.0f);

        showLayerButtons();
        ImGui::SameLine();

        Style::spriteImporterZoom.zoomCombo("##zoom");

        ImGui::SameLine(0.0f, 12.0f);
        showExtraWindowButtons();
    }

    ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    auto* drawList = ImGui::GetWindowDrawList();

    const SI::Frame* selectedFrame = _data->framesSel.selectedIndex() < frames.size() ? &frames.at(_data->framesSel.selectedIndex()) : nullptr;
    const urect selectedFrameAabb = selectedFrame ? selectedFrame->frameLocation(fs.grid) : urect();

    // The editable area changes depending on which frame is selected
    rect bounds;
    if (selectedFrame) {
        bounds.x = selectedFrameAabb.x;
        bounds.y = selectedFrameAabb.y;
        bounds.width = std::max(MINIMIM_FRAME_SIZE, selectedFrameAabb.width);
        bounds.height = std::max(MINIMIM_FRAME_SIZE, selectedFrameAabb.height);
    }

    _graphics.startLoop("##Editor", graphicsRect, bounds, Style::spriteImporterZoom.zoom(),
                        &_data->frameObjectsSel, &_data->actionPointsSel,
                        &_data->tileHitboxSel, &_data->shieldSel, &_data->hitboxSel, &_data->hurtboxSel);

    // Select frame with double click
    {
        // Cannot use `ImGuiHoveredFlags_AllowWhenBlockedByActiveItem` as it returns true when I click on the scrollbar.
        const upoint mousePos = _graphics.mousePosUpoint();
        const bool mouseOverSelectedFrame = selectedFrame && selectedFrameAabb.contains(mousePos);
        const bool windowHovered = ImGui::IsWindowHovered() || ImGui::IsItemActive();

        // Using double-click to select a frame as it easily allows me to determine
        // if we are selecting a frame or selecting/editing frame contents.
        const bool mouseDoubleClicked = ImGui::IsMouseDoubleClicked(0);

        if (windowHovered && not mouseOverSelectedFrame) {
            if (mouseDoubleClicked) {
                _data->framesSel.clearSelection();
            }

            for (auto [frameIndex, frame] : const_enumerate(frames)) {
                if (frame.frameLocation(fs.grid).contains(mousePos)) {
                    ImGui::ShowTooltip(frame.name);

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
        const auto& aabb = frame.frameLocation(fs.grid);
        const auto& origin = frame.origin(fs.grid);

        // Draw origin (behind frame contents)
        _graphics.drawCrosshair(drawList, aabb.x + origin.x, aabb.y + origin.y, aabb, Style::spriteImporterOriginColor);

        _graphics.addRect(drawList, &aabb, Style::frameOutlineColor);
    }

    for (auto [frameIndex, frame] : enumerate(frames)) {
        const auto& aabb = frame.frameLocation(fs.grid);

        _graphics.setOrigin(aabb.x, aabb.y);

        if (_data->framesSel.selectedIndex() != frameIndex) {
            drawFrame(drawList, &frame);
        }
        else {
            drawSelectedFrame(drawList, &frame);
        }
    }

    _graphics.setOrigin(0, 0);

    if (selectedFrame) {
        _graphics.drawAntiHighlight(drawList, selectedFrameAabb, Style::antiHighlightColor);
    }

    _graphics.endLoop(drawList,
                      &_data->frameObjectsSel, &_data->actionPointsSel,
                      &_data->tileHitboxSel, &_data->shieldSel, &_data->hitboxSel, &_data->hurtboxSel);

    if (_graphics.isEditingFinished()) {
        _data->undoStack().startMacro();

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

        _data->undoStack().endMacro();
    }

    Style::spriteImporterZoom.processMouseWheel();

    ImGui::EndChild();
}

void SpriteImporterEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }
    auto& fs = _data->data;

    updateImageTexture();
    updateExportOderTree(fs, projectFile);

    splitterSidebarRight(
        "##splitter", &_sidebar,
        "##Content",
        [&] {
            frameEditorGui();
        },
        "##Sidebar",
        [&] {
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                if (ImGui::BeginTabItem("FrameSet")) {
                    frameSetPropertiesGui(projectFile);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Frames")) {
                    framePropertiesGui(projectFile);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Animations")) {
                    animationPropertiesGui<AP>(_data, &fs);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        });
}

void SpriteImporterEditorGui::processExtraWindows(const Project::ProjectFile&, const Project::ProjectData&)
{
    animationPreviewWindow(_data, [this](auto... args) { drawAnimationFrame(args...); });
    exportOrderWindow();
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
    for (const auto& pixel : image->data()) {
        const ImU32 color = pixel.rgbaValue();

        bool newColor = colorSet.insert(color).second;
        if (newColor) {
            _transparentColorCombo.emplace_back(color, pixel.rgbHexString());

            if (_transparentColorCombo.size() >= MAX_COLORS) {
                return;
            }
        }
    }

    _transparentColorComboValid = true;
}

}
