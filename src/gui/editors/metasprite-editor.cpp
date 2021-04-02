/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metasprite-editor.h"
#include "gui/editor-actions.h"
#include "gui/graphics/aabb-graphics.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "gui/style.h"
#include "models/common/bit.h"
#include "models/metasprite/metasprite-error.h"
#include "models/snes/convert-snescolor.h"
#include "models/snes/drawing.hpp"
#include <algorithm>

namespace UnTech::Gui {

namespace MS = UnTech::MetaSprite::MetaSprite;
using ObjectSize = UnTech::MetaSprite::ObjectSize;

enum class MetaSpriteEditorGui::PaletteState {
    EDIT_COLOR,
    DRAW_TILES,
};

constexpr static unsigned SMALL_TILE_SIZE = 8;
constexpr static unsigned LARGE_TILE_SIZE = 16;

constexpr static unsigned N_PALETTE_COLORS = 16;
constexpr static unsigned PALETTE_TEXTURE_WIDTH = 16;
constexpr static unsigned PALETTE_TEXTURE_HEIGHT = 64;

constexpr static unsigned TILESET_IMAGE_WIDTH = 16 * SMALL_TILE_SIZE;
constexpr static unsigned SMALL_TILES_PER_ROW = TILESET_IMAGE_WIDTH / SMALL_TILE_SIZE;
constexpr static unsigned LARGE_TILES_PER_ROW = TILESET_IMAGE_WIDTH / LARGE_TILE_SIZE;

const ImVec2 MetaSpriteEditorGui::_paletteUvSize(1.0f, 1.0f / PALETTE_TEXTURE_HEIGHT);

const char* MetaSpriteEditorGui::colorPopupStrId = "Edit Color##MS";

const static std::array<const char*, 4> backgroundColorNames = {
    "Black",
    "White",
    "Light Grey",
    "Dark Grey",
};
const static std::array<ImU32, 4> backgroundColors = {
    IM_COL32_BLACK,
    IM_COL32_WHITE,
    IM_COL32(192, 192, 192, 255),
    IM_COL32(64, 64, 64, 255),
};
constexpr static int DEFAULT_BACKGROUND_COLOR = 2;

// MetaSpriteEditor Action Policies
struct MetaSpriteEditorData::AP {
    struct FrameSet {
        using EditorT = MetaSpriteEditorData;
        using EditorDataT = MS::FrameSet;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;

            if (itemIndex.index < projectFile.frameSets.size()) {
                auto& f = projectFile.frameSets.at(itemIndex.index);
                if (f.type == FrameSetType::METASPRITE) {
                    return f.msFrameSet.get();
                }
            }
            return nullptr;
        }
    };

    struct ExportOrder : public FrameSet {
        constexpr static auto FieldPtr = &EditorDataT::exportOrder;
        constexpr static auto validFlag = &MetaSpriteEditorGui::_exportOrderValid;
    };

    struct Palettes final : public FrameSet {
        using ListT = std::vector<UnTech::Snes::Palette4bpp>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        static_assert(PALETTE_TEXTURE_HEIGHT <= UnTech::MetaSprite::MAX_PALETTES);

        constexpr static size_t MAX_SIZE = PALETTE_TEXTURE_HEIGHT;

        constexpr static auto SelectionPtr = &EditorT::palettesSel;

        constexpr static auto validFlag = &MetaSpriteEditorGui::_paletteValid;

        static ListT* getList(MS::FrameSet& fs) { return &fs.palettes; }
    };

    struct SmallTileset final : public FrameSet {
        using ListT = std::vector<UnTech::Snes::Tile8px>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 128;
        constexpr static ObjectSize OBJ_SIZE = ObjectSize::SMALL;

        constexpr static auto SelectionPtr = &EditorT::smallTilesetSel;

        constexpr static auto validFlag = &MetaSpriteEditorGui::_tilesetValid;

        static ListT* getList(MS::FrameSet& fs) { return &fs.smallTileset; }
    };

    struct LargeTileset final : public FrameSet {
        using ListT = std::vector<UnTech::Snes::Tile16px>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 128;
        constexpr static ObjectSize OBJ_SIZE = ObjectSize::LARGE;

        constexpr static auto SelectionPtr = &EditorT::largeTilesetSel;

        constexpr static auto validFlag = &MetaSpriteEditorGui::_tilesetValid;

        static ListT* getList(MS::FrameSet& fs) { return &fs.largeTileset; }
    };

    struct Frames final : public FrameSet {
        using ListT = NamedList<MS::Frame>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::framesSel;

        constexpr static auto validFlag = &MetaSpriteEditorGui::_exportOrderValid;

        static ListT* getList(MS::FrameSet& fs) { return &fs.frames; }
    };

    struct FrameObjects final : public FrameSet {
        using ListT = std::vector<MS::FrameObject>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        static constexpr const char* name = "Frame Object";
        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_FRAME_OBJECTS;

        constexpr static auto SelectionPtr = &EditorT::frameObjectsSel;

        static ListT* getList(MS::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &MS::Frame::objects);
        }
    };

    struct ActionPoints final : public FrameSet {
        using ListT = std::vector<MS::ActionPoint>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        static constexpr const char* name = "Action Point";
        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_ACTION_POINTS;

        constexpr static auto SelectionPtr = &EditorT::actionPointsSel;

        static ListT* getList(MS::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &MS::Frame::actionPoints);
        }
    };

    struct EntityHitboxes final : public FrameSet {
        using ListT = std::vector<MS::EntityHitbox>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        static constexpr const char* name = "Entity Hitbox";
        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_ENTITY_HITBOXES;

        constexpr static auto SelectionPtr = &EditorT::entityHitboxesSel;

        static ListT* getList(MS::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &MS::Frame::entityHitboxes);
        }
    };

    struct Animations final : public FrameSet {
        using ListT = NamedList<UnTech::MetaSprite::Animation::Animation>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::animationsSel;

        constexpr static auto validFlag = &MetaSpriteEditorGui::_exportOrderValid;

        static ListT* getList(MS::FrameSet& fs) { return &fs.animations; }
    };

    struct AnimationFrames final : public FrameSet {
        using ListT = std::vector<UnTech::MetaSprite::Animation::AnimationFrame>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        // ::TODO change to UnTech::MetaSprite::MAX_ANIMATION_FRAMES::
        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::animationFramesSel;

        static ListT* getList(MS::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Animations::getList(fs), frameIndex,
                                &UnTech::MetaSprite::Animation::Animation::frames);
        }
    };
};

MetaSpriteEditorData::MetaSpriteEditorData(ItemIndex itemIndex)
    : AbstractMetaSpriteEditorData(itemIndex)
{
    framesSel.setSelected(0);
    palettesSel.setSelected(0);
}

bool MetaSpriteEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;

    _tileSelectionValid = false;

    const auto i = itemIndex().index;
    if (i < projectFile.frameSets.size()) {
        auto& f = projectFile.frameSets.at(i);

        setFilename(f.filename);
        if (f.type == FrameSetType::METASPRITE) {
            if (f.msFrameSet) {
                data = *f.msFrameSet;
                return true;
            }
        }
    }
    else {
        setFilename({});
    }

    return false;
}

void MetaSpriteEditorData::saveFile() const
{
    assert(!filename().empty());
    UnTech::MetaSprite::MetaSprite::saveFrameSet(data, filename());
}

void MetaSpriteEditorData::errorDoubleClicked(const AbstractError* error)
{
    using Type = MetaSprite::MsErrorType;

    animationsSel.clearSelection();
    animationFramesSel.clearSelection();

    framesSel.clearSelection();
    tileHitboxSel.clearSelection();
    frameObjectsSel.clearSelection();
    actionPointsSel.clearSelection();
    entityHitboxesSel.clearSelection();

    smallTilesetSel.clearSelection();
    largeTilesetSel.clearSelection();
    palettesSel.clearSelection();

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

        case Type::ENTITY_HITBOX:
            framesSel.setSelected(e->firstIndex);
            entityHitboxesSel.setSelected(e->firstIndex, e->childIndex);
            break;
        }
    }
}

void MetaSpriteEditorData::updateSelection()
{
    AbstractMetaSpriteEditorData::updateSelection();

    updateTileSelection();

    if (framesSel.isSelectionChanging()) {
        tileHitboxSel.clearSelection();
    }

    if (smallTilesetSel.isSelectionChanging()) {
        largeTilesetSel.clearSelection();
    }
    else if (largeTilesetSel.isSelectionChanging()) {
        smallTilesetSel.clearSelection();
    }

    palettesSel.update();
    framesSel.update();

    tileHitboxSel.update();

    if (frameObjectsSel.isSelectionChanging(framesSel)) {
        _tileSelectionValid = false;
    }

    frameObjectsSel.update(framesSel);
    actionPointsSel.update(framesSel);
    entityHitboxesSel.update(framesSel);

    smallTilesetSel.update();
    largeTilesetSel.update();
}

void MetaSpriteEditorData::updateTileSelection()
{
    if (_tileSelectionValid) {
        return;
    }

    if (framesSel.selectedIndex() < data.frames.size()) {
        const auto& frame = data.frames.at(framesSel.selectedIndex());

        smallTilesetSel.clearSelection();
        largeTilesetSel.clearSelection();

        unsigned tileId = INT_MAX;
        ObjectSize objSize = ObjectSize::SMALL;

        if (frameObjectsSel.hasSelection()) {
            for (auto [i, obj] : enumerate(frame.objects)) {
                if (frameObjectsSel.isSelected(i)) {
                    if (tileId == INT_MAX) {
                        tileId = obj.tileId;
                        objSize = obj.size;
                    }
                    else {
                        if (tileId != obj.tileId && objSize != obj.size) {
                            // Do not set selected tile if the selected objects use different tiles
                            tileId = INT_MAX;
                            break;
                        }
                    }
                }
            }
        }

        if (tileId < INT_MAX) {
            if (objSize == ObjectSize::SMALL) {
                smallTilesetSel.setSelected(tileId);
                smallTilesetSel.update();
            }
            else {
                largeTilesetSel.setSelected(tileId);
                largeTilesetSel.update();
            }
        }
    }

    _tileSelectionValid = true;
}

MetaSpriteEditorGui::MetaSpriteEditorGui()
    : AbstractMetaSpriteEditorGui()
    , _data(nullptr)
    , _colorSel(INT_MAX)
    , _paletteState(PaletteState::EDIT_COLOR)
    , _editedTiles()
    , _selectedEditorBgColor(DEFAULT_BACKGROUND_COLOR)
    , _graphics()
    , _paletteTexture(PALETTE_TEXTURE_WIDTH, PALETTE_TEXTURE_HEIGHT)
    , _tilesetTexture()
    , _paletteImage(PALETTE_TEXTURE_WIDTH, PALETTE_TEXTURE_HEIGHT)
    , _tilesetImage()
    , _paletteBackgroundColor(IM_COL32_WHITE)
    , _smallTilesetUvSize()
    , _largeTilesetUvSize()
    , _smallTilesetUVmax()
    , _largeTilesetUVmax()
    , _paletteValid(false)
    , _tilesetValid(false)
{
}

bool MetaSpriteEditorGui::setEditorData(AbstractEditorData* data)
{
    _data = dynamic_cast<MetaSpriteEditorData*>(data);
    setMetaSpriteData(_data);

    return _data;
}

void MetaSpriteEditorGui::resetState()
{
    AbstractMetaSpriteEditorGui::resetState();

    _paletteValid = false;
    _tilesetValid = false;

    _colorSel = INT_MAX;
    _paletteState = PaletteState::EDIT_COLOR;

    _graphics.resetState();
}

void MetaSpriteEditorGui::editorClosed()
{
}

void MetaSpriteEditorGui::addFrame(const idstring& name)
{
    assert(_data);

    MS::Frame frame;
    frame.name = name;
    ListActions<AP::Frames>::addItemToSelectedList(_data, frame);
}

void MetaSpriteEditorGui::addAnimation(const idstring& name)
{
    assert(_data);

    MetaSprite::Animation::Animation animation;
    animation.name = name;
    ListActions<AP::Animations>::addItemToSelectedList(_data, animation);
}

void MetaSpriteEditorGui::frameSetPropertiesWindow(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& fs = _data->data;

    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("FrameSet##MS")) {

        ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.4f);

        {
            ImGui::TextUnformatted("Properties:");
            ImGui::Indent();

            ImGui::InputIdstring("Name", &fs.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &MS::FrameSet::name>(_data);
            }

            if (ImGui::EnumCombo("Tileset Type", &fs.tilesetType)) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &MS::FrameSet::tilesetType>(_data);
            }

            if (ImGui::IdStringCombo("Export Order", &fs.exportOrder, projectFile.frameSetExportOrders)) {
                EditorFieldActions<AP::ExportOrder>::fieldEdited(_data);
            }

            ImGui::Unindent();
        }
        ImGui::Spacing();
    }
    ImGui::End();
}

void MetaSpriteEditorGui::framePropertiesWindow(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& fs = _data->data;

    ImGui::SetNextWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Frames##MS")) {

        ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.4f);

        ListButtons<AP::Frames>(_data);

        ImGui::SetNextItemWidth(-1);
        ImGui::NamedListListBox("##FrameList", &_data->framesSel, fs.frames, 8);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (_data->framesSel.selectedIndex() < fs.frames.size()) {
            MS::Frame& frame = fs.frames.at(_data->framesSel.selectedIndex());

            {
                ImGui::InputIdstring("Name", &frame.name);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &MS::Frame::name>(_data);
                }

                unsigned spriteOrder = frame.spriteOrder;
                if (ImGui::InputUnsigned("Sprite Order", &spriteOrder)) {
                    frame.spriteOrder = clamp<unsigned>(spriteOrder, 0, frame.spriteOrder.MASK);
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &MS::Frame::spriteOrder>(_data);
                }

                if (ImGui::Checkbox("Solid Tile Hitbox", &frame.solid)) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &MS::Frame::solid>(_data);
                }
                if (frame.solid) {
                    ImGui::InputMs8rect("Tile Hitbox", &frame.tileHitbox);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        ListActions<AP::Frames>::selectedFieldEdited<
                            &MS::Frame::tileHitbox>(_data);
                    }
                }
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            {
                CombinedListButtons<AP::FrameObjects, AP::ActionPoints, AP::EntityHitboxes>("FC_Buttons", _data);

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

                    const unsigned nSmallTiles = fs.smallTileset.size();
                    const unsigned nLargeTiles = fs.smallTileset.size();

                    for (auto [i, obj] : enumerate(frame.objects)) {
                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_data->frameObjectsSel, i);
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputMs8point("##location", &obj.location);
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        edited |= ImGui::EnumCombo("##size", &obj.size);

                        ImGui::SetNextItemWidth(-1);
                        if (ImGui::InputUnsigned("##tileId", &obj.tileId)) {
                            const unsigned nTiles = obj.size == ObjectSize::SMALL ? nSmallTiles : nLargeTiles;
                            const unsigned maxTileId = nTiles > 0 ? nTiles - 1 : 0;
                            obj.tileId = std::min(obj.tileId, maxTileId);
                            _data->_tileSelectionValid = false;
                            edited = true;
                        }

                        ImGui::SetNextItemWidth(-1);
                        edited |= ImGui::FlipCombo("##flip", &obj.hFlip, &obj.vFlip);

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
                        ImGui::InputMs8point("##location", &ap.location);
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

                if (ImGui::TreeNodeEx("Entity Hitboxes", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
                    ImGui::Indent();
                    ImGui::Columns(3);
                    ImGui::PushID("EH");

                    for (auto [i, eh] : enumerate(frame.entityHitboxes)) {
                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_data->entityHitboxesSel, i);
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputMs8rect("##aabb", &eh.aabb);
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
                            ListActions<AP::EntityHitboxes>::selectedListItemEdited(_data, i);
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

void MetaSpriteEditorGui::palettesWindow()
{
    assert(_data);
    auto& fs = _data->data;

    const auto& style = ImGui::GetStyle();

    const ImVec2 buttonSize(24, 24);
    const ImVec2 editButtonSize(buttonSize.x * 2 + style.ItemSpacing.x * 1, buttonSize.y);
    const float colorButtonsHeight = buttonSize.y * 3 + style.ItemSpacing.y * 4;

    const auto palIndex = _data->palettesSel.selectedIndex();

    bool colorSelected = false;

    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Palettes##MS")) {

        {
            ListButtons<AP::Palettes>(_data);

            const float scrollHeight = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - style.WindowPadding.y - colorButtonsHeight;
            ImGui::BeginChild("Scroll", ImVec2(0, scrollHeight));

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 40);

            const int palColorSize = ImGui::GetFontSize();
            const ImVec2 palSize(palColorSize * PALETTE_TEXTURE_WIDTH, palColorSize);
            ImVec2 palUv0(0, 0);
            ImVec2 palUv1 = _paletteUvSize;

            for (const auto i : range(fs.palettes.size())) {
                ImGui::Selectable(&_data->palettesSel, i, ImGuiSelectableFlags_SpanAllColumns);
                ImGui::NextColumn();

                if (i < PALETTE_TEXTURE_HEIGHT) {
                    ImGui::Image(_paletteTexture.imguiTextureId(), palSize, palUv0, palUv1);
                }
                ImGui::NextColumn();

                palUv0.y += _paletteUvSize.y;
                palUv1.y += _paletteUvSize.y;
            }

            ImGui::EndChild();
        }
        ImGui::Spacing();

        if (palIndex < fs.palettes.size()
            && palIndex < PALETTE_TEXTURE_HEIGHT) {

            assert(_paletteImage.size().width == N_PALETTE_COLORS);

            const rgba* colors = _paletteImage.scanline(palIndex);

            const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoAlpha;

            if (ImGui::ToggledButton("Edit", _paletteState == PaletteState::EDIT_COLOR, editButtonSize)) {
                _paletteState = PaletteState::EDIT_COLOR;
            }
            ImGui::SameLine();

            if (ImGui::ToggledButton("Select", _paletteState == PaletteState::DRAW_TILES, editButtonSize)) {
                _paletteState = PaletteState::DRAW_TILES;
            }

            for (const auto i : range(N_PALETTE_COLORS)) {
                ImGui::PushID(i);

                bool buttonClicked = false;
                const ImColor c(colors[i].rgb());

                if (i != _colorSel) {
                    buttonClicked = ImGui::ColorButton("##color", c, flags, buttonSize);
                }
                else {
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 6);
                    buttonClicked = ImGui::ColorButton("##color", c, flags, buttonSize);
                    ImGui::PopStyleVar();
                }

                if (buttonClicked) {
                    _colorSel = i;
                    colorSelected = true;
                }

                ImGui::PopID();

                if (i != 7) {
                    ImGui::SameLine();
                }
            }
        }
    }
    ImGui::End();

    if (colorSelected && _paletteState == PaletteState::EDIT_COLOR) {
        ImGui::OpenPopup(colorPopupStrId);
    }
}

void MetaSpriteEditorGui::colorPopup()
{
    assert(_data);
    auto& fs = _data->data;

    static bool _popupOpen = false;
    static ImVec4 _oldColor;

    const float itemWidth = 80;
    const ImVec2 colorButtonSize(itemWidth, itemWidth * 4 / 6);

    const auto flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview;

    if (ImGui::BeginPopup(colorPopupStrId)) {
        const auto& style = ImGui::GetStyle();

        const auto palIndex = _data->palettesSel.selectedIndex();

        bool colorChanged = false;

        if (_colorSel < MetaSprite::PALETTE_COLORS
            && palIndex < fs.palettes.size()) {

            auto& snesColor = fs.palettes.at(palIndex).at(_colorSel);
            const rgba color = Snes::toRgb(snesColor);
            ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(color.rgb());

            if (!_popupOpen) {
                _oldColor = colorVec;
                _popupOpen = true;
            }

            bool colorVecChanged = false;

            colorVecChanged |= ImGui::ColorPicker3("##picker", (float*)&colorVec, flags);
            const float colorPickerCursorY = ImGui::GetCursorPosY();

            ImGui::SameLine();

            ImGui::BeginGroup();

            ImGui::TextUnformatted("Current:");
            ImGui::ColorButton("##current", colorVec, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoAlpha, colorButtonSize);

            ImGui::Spacing();

            ImGui::TextUnformatted("Old:");
            if (ImGui::ColorButton("##old", _oldColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoAlpha, colorButtonSize)) {
                colorVec = _oldColor;
                colorVecChanged = true;
            }

            ImGui::SetCursorPosY(colorPickerCursorY - (ImGui::GetFrameHeight() + style.ItemSpacing.y) * 3);
            ImGui::PushItemWidth(itemWidth);

            unsigned blue = snesColor.blue();
            unsigned green = snesColor.green();
            unsigned red = snesColor.red();

            if (ImGui::InputUnsigned("Blue", &blue)) {
                snesColor.setBlue(clamp<unsigned>(blue, 0, 31));
                colorChanged = true;
            }
            if (ImGui::InputUnsigned("Green", &green)) {
                snesColor.setGreen(clamp<unsigned>(green, 0, 31));
                colorChanged = true;
            }
            if (ImGui::InputUnsigned("Red", &red)) {
                snesColor.setRed(clamp<unsigned>(red, 0, 31));
                colorChanged = true;
            }

            ImGui::PopItemWidth();
            ImGui::EndGroup();

            if (colorVecChanged) {
                snesColor = Snes::toSnesColor(rgba::fromRgba(ImGui::ColorConvertFloat4ToU32(colorVec)));
                colorChanged = true;
            }
        }
        else {
            ImGui::CloseCurrentPopup();
            _popupOpen = false;
        }

        ImGui::EndPopup();

        if (colorChanged) {
            _paletteValid = false;
        }
    }
    else {
        if (_popupOpen) {
            ListActions<AP::Palettes>::selectedItemEdited(_data);
            _popupOpen = false;
            _colorSel = INT_MAX;
        }
    }
}

void MetaSpriteEditorGui::tilesetButtons()
{
    assert(_data);

    if (ImGui::Button("Add Small Tile")) {
        ListActions<AP::SmallTileset>::editList(_data, EditListAction::ADD);
    }
    ImGui::SameLine();

    if (ImGui::Button("Add Large Tile")) {
        ListActions<AP::LargeTileset>::editList(_data, EditListAction::ADD);
    }
    ImGui::SameLine();

    if (ImGui::Button("Remove")) {
        auto tileRemoved = [this](const unsigned tileId, const ObjectSize size) {
            auto& fs = _data->data;

            for (auto [i, frame] : enumerate(fs.frames)) {
                bool edited = false;
                for (auto& obj : frame.objects) {
                    if (obj.tileId >= tileId && obj.size == size) {
                        obj.tileId--;
                        edited = true;
                    }
                }
                if (edited) {
                    ListActions<AP::Frames>::fieldEdited<
                        &MS::Frame::objects>(_data, i);
                }
            }
        };

        _data->startMacro();
        if (_data->smallTilesetSel.hasSelection()) {
            ListActions<AP::SmallTileset>::editList(_data, EditListAction::REMOVE);
            tileRemoved(_data->smallTilesetSel.selectedIndex(), ObjectSize::SMALL);
        }
        if (_data->largeTilesetSel.hasSelection()) {
            ListActions<AP::LargeTileset>::editList(_data, EditListAction::REMOVE);
            tileRemoved(_data->largeTilesetSel.selectedIndex(), ObjectSize::LARGE);
        }
        _data->endMacro();
    }
}

void MetaSpriteEditorGui::setSelectedFrameObjectsTile(const unsigned tileId, const ObjectSize objSize)
{
    assert(_data);
    auto& fs = _data->data;

    if (_data->framesSel.selectedIndex() < fs.frames.size()) {
        auto& frame = fs.frames.at(_data->framesSel.selectedIndex());

        if (_data->frameObjectsSel.hasSelection()) {
            for (auto [i, obj] : enumerate(frame.objects)) {
                if (_data->frameObjectsSel.isSelected(i)) {
                    obj.tileId = tileId;
                    obj.size = objSize;
                }
            }
            ListActions<AP::Frames>::selectedFieldEdited<
                &MS::Frame::objects>(_data);
        }
    }
}

template <typename TilesetPolicy>
void MetaSpriteEditorGui::drawTileset(const char* label, typename TilesetPolicy::ListT* tileset,
                                      ImDrawList* drawList, const int z, const ImVec2 uv0, const ImVec2 uv1)
{
    constexpr unsigned N_COLORS = 16;
    constexpr unsigned TILE_SIZE = TilesetPolicy::ListT::value_type::TILE_SIZE;
    constexpr unsigned TILES_PER_ROW = TILESET_IMAGE_WIDTH / TILE_SIZE;
    const float tileSize = TILE_SIZE * z;

    assert(_data);

    if (tileset->empty()) {
        const ImVec2 size(TILESET_IMAGE_WIDTH * z, tileSize);
        ImGui::InvisibleButton(label, size);

        return;
    }

    SingleSelection& sel = _data->*TilesetPolicy::SelectionPtr;

    const ImVec2 offset = ImGui::GetCursorScreenPos();

    const unsigned remaining = tileset->size() % TILES_PER_ROW;
    const unsigned completeRows = tileset->size() / TILES_PER_ROW;

    assert(remaining != 0 || completeRows != 0);

    const ImVec2 completeRowSize = ImVec2(TILESET_IMAGE_WIDTH * z, completeRows * tileSize);
    const ImVec2 imageSize = ImVec2(completeRowSize.x, completeRowSize.y + (remaining > 0 ? tileSize : 0));

    if (completeRows > 0) {
        drawList->AddRectFilled(offset, offset + completeRowSize, _paletteBackgroundColor);
    }
    if (remaining != 0) {
        const ImVec2 partialPos(offset.x, offset.y + completeRowSize.y);
        const ImVec2 partialSize(remaining * tileSize, tileSize);

        drawList->AddRectFilled(partialPos, partialPos + partialSize, _paletteBackgroundColor);
    }

    drawList->AddImage(_tilesetTexture.imguiTextureId(), offset, offset + imageSize, uv0, uv1);

    // Draw grid
    {
        const unsigned nFullLines = remaining > 0 ? remaining - 1 : TILES_PER_ROW;

        const float startY = offset.y;
        const float fullEndY = startY + imageSize.y;
        const float partialEndY = fullEndY - tileSize;
        float xPos = offset.x + tileSize;

        for (const auto x : range(TILES_PER_ROW - 1)) {
            if (x < nFullLines) {
                drawList->AddLine(ImVec2(xPos, startY), ImVec2(xPos, fullEndY), Style::gridColor);
            }
            else {
                if (completeRows == 0) {
                    break;
                }
                drawList->AddLine(ImVec2(xPos, startY), ImVec2(xPos, partialEndY), Style::gridColor);
            }
            xPos += tileSize;
        }
    }
    {
        const float startX = offset.x;
        const float endX = offset.x + imageSize.x;

        float yPos = offset.y + tileSize;
        if (completeRows > 0) {
            for ([[maybe_unused]] const auto y : range(completeRows - 1)) {
                drawList->AddLine(ImVec2(startX, yPos), ImVec2(endX, yPos), Style::gridColor);
                yPos += tileSize;
            }
        }
        if (remaining > 0) {
            drawList->AddLine(ImVec2(startX, yPos), ImVec2(startX + remaining * tileSize, yPos), Style::gridColor);
        }
    }

    // Draw Selected Tile
    if (sel.selectedIndex() < tileset->size()) {
        const unsigned tileId = sel.selectedIndex();
        const ImVec2 pMin(offset.x + int(tileId % TILES_PER_ROW) * tileSize,
                          offset.y + int(tileId / TILES_PER_ROW) * tileSize);
        const ImVec2 pMax(pMin.x + tileSize, pMin.y + tileSize);

        drawList->AddRect(pMin, pMax, IM_COL32_WHITE, 0.0f, ImDrawCornerFlags_None, 3.0f);
        drawList->AddRect(pMin, pMax, IM_COL32_BLACK, 0.0f, ImDrawCornerFlags_None, 1.0f);
    }

    ImGui::InvisibleButton(label, imageSize);

    if (ImGui::IsItemHovered()) {
        if (_paletteState == PaletteState::DRAW_TILES) {
            // ::TODO find a way to use a pen cursor::
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }
    }
    if (ImGui::IsItemActive()) {
        const auto mousePos = ImGui::GetMousePos() - offset;
        const point p(mousePos.x / z, mousePos.y / z);
        unsigned tileId = int(p.x / TILE_SIZE) + int(p.y / TILE_SIZE) * TILES_PER_ROW;

        if (tileId < tileset->size()) {
            switch (_paletteState) {
            case PaletteState::DRAW_TILES: {
                if (_colorSel >= N_COLORS) {
                    _colorSel = 0;
                }

                if (ImGui::IsMouseClicked(0)) {
                    _editedTiles.clear();
                }
                if (ImGui::IsMouseDown(0)) {
                    tileset->at(tileId).setPixel(p.x % TILE_SIZE, p.y % TILE_SIZE, _colorSel);
                    _editedTiles.insert(tileId);

                    // Immediatly display changed tile data
                    _tilesetValid = false;
                }
            } break;

            case PaletteState::EDIT_COLOR: {
                if (ImGui::IsMouseClicked(0)) {
                    if (tileId != sel.selectedIndex()) {
                        setSelectedFrameObjectsTile(tileId, TilesetPolicy::OBJ_SIZE);
                    }
                    sel.setSelected(tileId);
                }
            } break;
            }
        }
    }
    else if (ImGui::IsItemDeactivated()) {
        if (!_editedTiles.empty()) {
            ListActions<TilesetPolicy>::selectedListItemsEdited(_data, _editedTiles.vector());
            _editedTiles.empty();
        }
    }
};

void MetaSpriteEditorGui::tilesetWindow()
{
    assert(_data);
    auto& fs = _data->data;

    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Tileset##MS")) {

        tilesetButtons();

        ImGui::BeginChild("Scroll");

        const int z = std::max<int>(1, ImGui::GetContentRegionAvailWidth() / _tilesetTexture.width());

        auto* drawList = ImGui::GetWindowDrawList();

        // Needed to draw the top line of the selected tile
        ImGui::Spacing();

        drawTileset<AP::SmallTileset>("Small", &fs.smallTileset, drawList,
                                      z, ImVec2(0.0f, 0.0f), _smallTilesetUVmax);

        ImGui::Separator();

        drawTileset<AP::LargeTileset>("Large", &fs.largeTileset, drawList,
                                      z, ImVec2(0.0f, _smallTilesetUVmax.y), _largeTilesetUVmax);

        ImGui::EndChild();
    }
    ImGui::End();
}

// NOTE: zoom will be negative if the frame is flipped
inline void MetaSpriteEditorGui::drawAnimationFrame(const ImVec2& pos, const ImVec2& zoom, const MS::Frame& frame) const
{
    assert(_data);
    const auto& fs = _data->data;

    const float lineThickness = AabbGraphics::lineThickness;

    auto* drawList = ImGui::GetWindowDrawList();

    if (showFrameObjects) {
        const ImTextureID textureId = _tilesetTexture.imguiTextureId();

        for (auto [i, obj] : reverse_enumerate(frame.objects)) {
            bool valid = false;
            ImVec2 uv0, uv1;
            if (obj.size == ObjectSize::SMALL) {
                valid = obj.tileId < fs.smallTileset.size();
                uv0.x = unsigned(obj.tileId % SMALL_TILES_PER_ROW) * _smallTilesetUvSize.x;
                uv0.y = unsigned(obj.tileId / SMALL_TILES_PER_ROW) * _smallTilesetUvSize.y;
                uv1 = uv0 + _smallTilesetUvSize;
            }
            else {
                valid = obj.tileId < fs.largeTileset.size();
                uv0.x = unsigned(obj.tileId % LARGE_TILES_PER_ROW) * _largeTilesetUvSize.x;
                uv0.y = unsigned(obj.tileId / LARGE_TILES_PER_ROW) * _largeTilesetUvSize.y + _smallTilesetUVmax.y;
                uv1 = uv0 + _largeTilesetUvSize;
            }

            if (valid) {
                if (obj.hFlip) {
                    std::swap(uv0.x, uv1.x);
                }
                if (obj.vFlip) {
                    std::swap(uv0.y, uv1.y);
                }
            }
            else {
                // ::TODO add semitransparent invalid image with a checkbox pattern::
                uv0 = ImVec2(0, 0);
                uv1 = ImVec2(1, 1);
            }

            ImVec2 p1(pos.x + obj.location.x * zoom.x, pos.y + obj.location.y * zoom.y);
            ImVec2 p2(p1.x + obj.sizePx() * zoom.x, p1.y + obj.sizePx() * zoom.y);
            drawList->AddImage(textureId, p1, p2, uv0, uv1);
        }
    }

    if (showTileHitbox) {
        if (frame.solid) {
            ImVec2 p1(pos.x + frame.tileHitbox.x * zoom.x, pos.y + frame.tileHitbox.y * zoom.y);
            ImVec2 p2(p1.x + frame.tileHitbox.width * zoom.x, p1.y + frame.tileHitbox.height * zoom.y);
            drawList->AddRect(p1, p2, Style::tileHitboxOutlineColor, lineThickness);
        }
    }

    if (showEntityHitboxes) {
        for (auto [i, eh] : reverse_enumerate(frame.entityHitboxes)) {
            ImVec2 p1(pos.x + eh.aabb.x * zoom.x, pos.y + eh.aabb.y * zoom.y);
            ImVec2 p2(p1.x + eh.aabb.width * zoom.x, p1.y + eh.aabb.height * zoom.y);
            drawList->AddRect(p1, p2, Style::entityHitboxOutlineColor, lineThickness);
        }
    }

    if (showActionPoints) {
        for (auto [i, ap] : reverse_enumerate(frame.actionPoints)) {
            ImVec2 p1(pos.x + ap.location.x * zoom.x, pos.y + ap.location.y * zoom.y);
            ImVec2 p2(p1.x + zoom.x, p1.y + zoom.y);
            drawList->AddRect(p1, p2, Style::actionPointOutlineColor, lineThickness);
        }
    }
}

void MetaSpriteEditorGui::frameEditorWindow()
{
    assert(_data);
    auto& fs = _data->data;

    static const std::string windowSuffix = "###MetaSprite_Frame";

    static const rect ms8RectBounds(int_ms8_t::MIN, int_ms8_t::MIN, int_ms8_t::MAX - int_ms8_t::MIN, int_ms8_t::MAX - int_ms8_t::MIN);

    MS::Frame* const frame = _data->framesSel.selectedIndex() < fs.frames.size()
                                 ? &fs.frames.at(_data->framesSel.selectedIndex())
                                 : nullptr;

    const std::string windowTitle = frame ? frame->name + windowSuffix : windowSuffix;

    ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(windowTitle.c_str())) {

        if (frame == nullptr) {
            ImGui::End();
            return;
        }

        {
            undoStackButtons();
            ImGui::SameLine(0.0f, 12.0f);

            showLayerButtons();
            ImGui::SameLine();

            Style::metaSpriteZoom.zoomCombo("##zoom");
        }

        static_assert(backgroundColorNames.size() == backgroundColors.size());
        ImGui::Combo("Background Color", &_selectedEditorBgColor, backgroundColorNames.data(), backgroundColorNames.size());
        const unsigned bgIndex = clamp<int>(_selectedEditorBgColor, 0, backgroundColors.size() - 1);
        const ImU32 bgColor = backgroundColors.at(bgIndex);

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        auto* drawList = ImGui::GetWindowDrawList();

        _graphics.startLoop("##Editor", ms8RectBounds, Style::metaSpriteZoom.zoom(),
                            &_data->tileHitboxSel, &_data->frameObjectsSel, &_data->actionPointsSel, &_data->entityHitboxesSel);

        _graphics.drawBackgroundColor(drawList, bgColor);
        _graphics.drawBoundedCrosshair(drawList, 0, 0, Style::metaSpriteCrosshairColor);

        const ImTextureID textureId = _tilesetTexture.imguiTextureId();

        if (showFrameObjects) {
            for (auto [i, obj] : reverse_enumerate(frame->objects)) {
                bool valid = false;
                ImVec2 uv0, uv1;
                if (obj.size == ObjectSize::SMALL) {
                    valid = obj.tileId < fs.smallTileset.size();
                    uv0.x = unsigned(obj.tileId % SMALL_TILES_PER_ROW) * _smallTilesetUvSize.x;
                    uv0.y = unsigned(obj.tileId / SMALL_TILES_PER_ROW) * _smallTilesetUvSize.y;
                    uv1 = uv0 + _smallTilesetUvSize;
                }
                else {
                    valid = obj.tileId < fs.largeTileset.size();
                    uv0.x = unsigned(obj.tileId % LARGE_TILES_PER_ROW) * _largeTilesetUvSize.x;
                    uv0.y = unsigned(obj.tileId / LARGE_TILES_PER_ROW) * _largeTilesetUvSize.y + _smallTilesetUVmax.y;
                    uv1 = uv0 + _largeTilesetUvSize;
                }

                if (valid) {
                    if (obj.hFlip) {
                        std::swap(uv0.x, uv1.x);
                    }
                    if (obj.vFlip) {
                        std::swap(uv0.y, uv1.y);
                    }
                }
                else {
                    // ::TODO add semitransparent invalid image with a checkbox pattern::
                    uv0 = ImVec2(0, 0);
                    uv1 = ImVec2(1, 1);
                }

                _graphics.addSquareImage(drawList, &obj.location, obj.sizePx(), textureId, uv0, uv1, Style::frameObjectOutlineColor, &_data->frameObjectsSel, i);

                if (_graphics.isHoveredAndNotEditing()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Object %u", unsigned(i));
                    ImGui::EndTooltip();
                }
            }
        }

        if (showTileHitbox) {
            if (frame->solid) {
                _graphics.addRect(drawList, &frame->tileHitbox, Style::tileHitboxOutlineColor, &_data->tileHitboxSel, 1);
                if (_graphics.isHoveredAndNotEditing()) {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted("Tile Hitbox");
                    ImGui::EndTooltip();
                }
            }
        }

        if (showEntityHitboxes) {
            for (auto [i, eh] : reverse_enumerate(frame->entityHitboxes)) {
                _graphics.addRect(drawList, &eh.aabb, Style::entityHitboxOutlineColor, &_data->entityHitboxesSel, i);

                if (_graphics.isHoveredAndNotEditing()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Entity Hitbox %u (%s)", unsigned(i), eh.hitboxType.to_string().c_str());
                    ImGui::EndTooltip();
                }
            }
        }

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

        _graphics.endLoop(drawList,
                          &_data->tileHitboxSel, &_data->frameObjectsSel, &_data->actionPointsSel, &_data->entityHitboxesSel);

        if (_graphics.isEditingFinished()) {
            _data->startMacro();

            if (_data->tileHitboxSel.isSelected()) {
                ListActions<AP::Frames>::selectedFieldEdited<&MS::Frame::tileHitbox>(_data);
            }
            ListActions<AP::FrameObjects>::selectedItemsEdited(_data);
            ListActions<AP::ActionPoints>::selectedItemsEdited(_data);
            ListActions<AP::EntityHitboxes>::selectedItemsEdited(_data);

            _data->endMacro();
        }

        Style::metaSpriteZoom.processMouseWheel();

        ImGui::EndChild();
    }
    ImGui::End();
}

void MetaSpriteEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }
    auto& fs = _data->data;

    updateExportOderTree(fs, projectFile);
    updatePaletteTexture();
    updateTilesetTexture();

    frameSetPropertiesWindow(projectFile);
    framePropertiesWindow(projectFile);

    frameEditorWindow();

    palettesWindow();
    tilesetWindow();

    animationPropertiesWindow<AP>("Animations##MS", _data, &fs);
    animationPreviewWindow("Animation Preview##MS", _data, [this](auto... args) { drawAnimationFrame(args...); });
    exportOrderWindow("Export Order##MS");

    colorPopup();

    updateSelection();
}

void MetaSpriteEditorGui::viewMenu()
{
    AbstractMetaSpriteEditorGui::viewMenu();

    ImGui::Separator();

    if (ImGui::BeginMenu("Background Color")) {
        for (auto [i, bcn] : enumerate(backgroundColorNames)) {
            if (ImGui::MenuItem(bcn, nullptr, _selectedEditorBgColor == int(i))) {
                _selectedEditorBgColor = i;
            }
        }

        ImGui::EndMenu();
    }
}

void MetaSpriteEditorGui::updateSelection()
{
    assert(_data);

    if (_data->palettesSel.isSelectionChanging()) {
        _tilesetValid = false;
    }

    if (_paletteState == PaletteState::DRAW_TILES) {
        _data->smallTilesetSel.clearSelection();
        _data->largeTilesetSel.clearSelection();
    }
}

void MetaSpriteEditorGui::updatePaletteTexture()
{
    assert(_data);
    auto& fs = _data->data;

    if (_paletteValid) {
        return;
    }

    _paletteImage.fill(rgba());

    const size_t nPalettes = std::min<size_t>(fs.palettes.size(), PALETTE_TEXTURE_HEIGHT);

    rgba* imgBits = _paletteImage.data();
    const rgba* imgBitsEnd = _paletteImage.dataEnd();

    for (const auto pId : range(nPalettes)) {
        const auto& palette = fs.palettes.at(pId);

        assert(PALETTE_TEXTURE_WIDTH == palette.size());

        for (const auto c : range(palette.size())) {
            *imgBits++ = Snes::toRgb(palette.at(c));
        }
    }
    assert(imgBits <= imgBitsEnd);

    _paletteTexture.replace(_paletteImage);

    _paletteValid = true;
    _tilesetValid = false;
}

void MetaSpriteEditorGui::updateTilesetTexture()
{
    static const Snes::Palette4bpp BLANK_PALETTE;

    assert(_data);
    auto& fs = _data->data;

    if (_tilesetValid) {
        return;
    }

    const rgba* palette = nullptr;
    {
        const auto palIndex = _data->palettesSel.selectedIndex();

        if (palIndex < fs.palettes.size()
            && palIndex < PALETTE_TEXTURE_HEIGHT) {

            assert(_paletteImage.size().width == N_PALETTE_COLORS);
            palette = _paletteImage.scanline(palIndex);
        }
    }

    const unsigned nSmallRows = (std::max<unsigned>(1, fs.smallTileset.size()) - 1) / SMALL_TILES_PER_ROW + 1;
    const unsigned nLargeRows = (std::max<unsigned>(1, fs.largeTileset.size()) - 1) / LARGE_TILES_PER_ROW + 1;

    const usize tilesetSize(TILESET_IMAGE_WIDTH,
                            nextPowerOfTwo(nSmallRows * SMALL_TILE_SIZE + nLargeRows * LARGE_TILE_SIZE));

    if (_tilesetImage == nullptr || _tilesetImage->size() != tilesetSize) {
        _tilesetImage = std::make_unique<Image>(tilesetSize);
    }
    else {
        _tilesetImage->fill(rgba());
    }

    _smallTilesetUvSize = ImVec2(float(SMALL_TILE_SIZE) / tilesetSize.width, float(SMALL_TILE_SIZE) / tilesetSize.height);
    _largeTilesetUvSize = ImVec2(float(LARGE_TILE_SIZE) / tilesetSize.width, float(LARGE_TILE_SIZE) / tilesetSize.height);
    _smallTilesetUVmax = ImVec2(1.0f, _smallTilesetUvSize.y * nSmallRows);
    _largeTilesetUVmax = ImVec2(1.0f, _smallTilesetUVmax.y + _largeTilesetUvSize.y * nLargeRows);

    if (palette) {
        const rgba* imgBitsEnd = _tilesetImage->dataEnd();
        constexpr size_t stride = TILESET_IMAGE_WIDTH;
        assert(stride == _tilesetImage->pixelsPerScanline());

        auto* imgBits = _tilesetImage->data();
        Snes::drawTileset_transparent(fs.smallTileset, imgBits, imgBitsEnd, stride, palette, N_PALETTE_COLORS);

        imgBits = _tilesetImage->scanline(nSmallRows * SMALL_TILE_SIZE);
        Snes::drawTileset_transparent(fs.largeTileset, imgBits, imgBitsEnd, stride, palette, N_PALETTE_COLORS);
    }

    _tilesetTexture.replace(*_tilesetImage);

    // ALso update background color
    if (palette) {
        _paletteBackgroundColor = palette[0].rgbaValue();
    }
    else {
        _paletteBackgroundColor = 0;
    }

    _tilesetValid = true;
}

}
