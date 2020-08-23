/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metasprite-editor.h"
#include "gui/common/aabb-graphics.h"
#include "gui/editor-actions.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "models/common/bit.h"
#include <algorithm>

namespace UnTech::Gui {

namespace MS = UnTech::MetaSprite::MetaSprite;

enum class MetaSpriteEditor::PaletteState {
    EDIT_COLOR,
    DRAW_TILES,
};

// ::TODO move to style::
constexpr static ImU32 frameObjectOutlineCol = IM_COL32(64, 128, 64, 240);
constexpr static ImU32 actionPointOutlineCol = IM_COL32(192, 192, 192, 240);
constexpr static ImU32 entityHitboxOutlineCol = IM_COL32(0, 0, 255, 240);
constexpr static ImU32 tileHitboxOutlineCol = IM_COL32(192, 0, 0, 240);

constexpr static ImU32 crosshairColor = IM_COL32(128, 128, 128, 255);
constexpr static ImU32 tilesetGridColor = IM_COL32(128, 128, 128, 128);

constexpr static unsigned SMALL_TILE_SIZE = 8;
constexpr static unsigned LARGE_TILE_SIZE = 16;

constexpr static unsigned PALETTE_TEXTURE_WIDTH = 16;
constexpr static unsigned TILESET_IMAGE_WIDTH = 16 * SMALL_TILE_SIZE;
constexpr static unsigned SMALL_TILES_PER_ROW = TILESET_IMAGE_WIDTH / SMALL_TILE_SIZE;
constexpr static unsigned LARGE_TILES_PER_ROW = TILESET_IMAGE_WIDTH / LARGE_TILE_SIZE;

// ::TODO dynamic zoom::
static const ImVec2 zoom(5.0f, 5.0f);

const char* MetaSpriteEditor::colorPopupStrId = "Edit Color##MS";

// ::TODO find better name::
AabbGraphics MetaSpriteEditor::_graphics;

unsigned MetaSpriteEditor::_colorSel = INT_MAX;
MetaSpriteEditor::PaletteState MetaSpriteEditor::_paletteState = MetaSpriteEditor::PaletteState::EDIT_COLOR;
vectorset<size_t> MetaSpriteEditor::_editedTiles;

Image MetaSpriteEditor::_paletteImage{};
Image MetaSpriteEditor::_tilesetImage{};
ImU32 MetaSpriteEditor::_paletteBackgroundColor = IM_COL32_WHITE;

ImVec2 MetaSpriteEditor::_paletteUvSize{};
ImVec2 MetaSpriteEditor::_smallTilesetUvSize{};
ImVec2 MetaSpriteEditor::_largeTilesetUvSize{};
ImVec2 MetaSpriteEditor::_smallTilesetUVmax{};
ImVec2 MetaSpriteEditor::_largeTilesetUVmax{};

bool MetaSpriteEditor::_paletteValid = false;
bool MetaSpriteEditor::_tilesetValid = false;
bool MetaSpriteEditor::_tileSelectionValid = false;

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
struct MetaSpriteEditor::AP {
    struct FrameSet {
        using EditorT = MetaSpriteEditor;
        using EditorDataT = MS::FrameSet;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._data;
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

    struct Palettes final : public FrameSet {
        using ListT = std::vector<UnTech::Snes::Palette4bpp>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_PALETTES;

        constexpr static auto SelectionPtr = &EditorT::_palettesSel;

        static ListT* getList(MS::FrameSet& fs) { return &fs.palettes; }
    };

    struct SmallTileset final : public FrameSet {
        using ListT = UnTech::Snes::Tileset8px;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 128;
        constexpr static ObjectSize OBJ_SIZE = ObjectSize::SMALL;

        constexpr static auto SelectionPtr = &EditorT::_smallTilesetSel;

        static ListT* getList(MS::FrameSet& fs) { return &fs.smallTileset; }
    };

    struct LargeTileset final : public FrameSet {
        using ListT = UnTech::Snes::TilesetTile16;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 128;
        constexpr static ObjectSize OBJ_SIZE = ObjectSize::LARGE;

        constexpr static auto SelectionPtr = &EditorT::_largeTilesetSel;

        static ListT* getList(MS::FrameSet& fs) { return &fs.largeTileset; }
    };

    struct Frames final : public FrameSet {
        using ListT = NamedList<MS::Frame>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_EXPORT_NAMES;

        constexpr static auto SelectionPtr = &EditorT::_framesSel;

        static ListT* getList(MS::FrameSet& fs) { return &fs.frames; }
    };

    struct FrameObjects final : public FrameSet {
        using ListT = std::vector<MS::FrameObject>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_FRAME_OBJECTS;

        constexpr static auto SelectionPtr = &EditorT::_frameObjectsSel;

        static ListT* getList(MS::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &MS::Frame::objects);
        }
    };

    struct ActionPoints final : public FrameSet {
        using ListT = std::vector<MS::ActionPoint>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_ACTION_POINTS;

        constexpr static auto SelectionPtr = &EditorT::_actionPointsSel;

        static ListT* getList(MS::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Frames::getList(fs), frameIndex, &MS::Frame::actionPoints);
        }
    };

    struct EntityHitboxes final : public FrameSet {
        using ListT = std::vector<MS::EntityHitbox>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = UnTech::MetaSprite::MAX_ENTITY_HITBOXES;

        constexpr static auto SelectionPtr = &EditorT::_entityHitboxesSel;

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

        constexpr static auto SelectionPtr = &EditorT::_animationsSel;

        static ListT* getList(MS::FrameSet& fs) { return &fs.animations; }
    };

    struct AnimationFrames final : public FrameSet {
        using ListT = std::vector<UnTech::MetaSprite::Animation::AnimationFrame>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        // ::TODO change to UnTech::MetaSprite::MAX_ANIMATION_FRAMES::
        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::_animationFramesSel;

        static ListT* getList(MS::FrameSet& fs, unsigned frameIndex)
        {
            return getListField(Animations::getList(fs), frameIndex,
                                &UnTech::MetaSprite::Animation::Animation::frames);
        }
    };
};

Texture& MetaSpriteEditor::paletteTexture()
{
    static Texture texture;
    return texture;
}

Texture& MetaSpriteEditor::tilesetTexture()
{
    static Texture texture;
    return texture;
}

MetaSpriteEditor::MetaSpriteEditor(ItemIndex itemIndex)
    : AbstractMetaSpriteEditor(itemIndex)
{
    _framesSel.setSelected(0);
    _palettesSel.setSelected(0);

    _selectedEditorBgColor = DEFAULT_BACKGROUND_COLOR;
}

bool MetaSpriteEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;

    _paletteValid = false;
    _tilesetValid = false;
    _tileSelectionValid = false;

    const auto i = itemIndex().index;
    if (i < projectFile.frameSets.size()) {
        auto& f = projectFile.frameSets.at(i);
        if (f.type == FrameSetType::METASPRITE) {
            if (f.msFrameSet) {
                _data = *f.msFrameSet;
                return true;
            }
        }
    }

    return false;
}

void MetaSpriteEditor::editorOpened()
{
    _paletteValid = false;
    _tilesetValid = false;
    _tileSelectionValid = false;

    _colorSel = INT_MAX;
    _paletteState = PaletteState::EDIT_COLOR;

    _graphics.resetState();
}

void MetaSpriteEditor::editorClosed()
{
}

void MetaSpriteEditor::frameSetPropertiesWindow(const Project::ProjectFile& projectFile)
{
    if (ImGui::Begin("FrameSet##MS")) {
        ImGui::SetWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

        ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.4f);

        {
            ImGui::TextUnformatted("Properties:");
            ImGui::Indent();

            ImGui::InputIdstring("Name", &_data.name);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &MS::FrameSet::name>(this);
            }

            if (ImGui::EnumCombo("Tileset Type", &_data.tilesetType)) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &MS::FrameSet::tilesetType>(this);
            }

            if (ImGui::IdStringCombo("Export Order", &_data.exportOrder, projectFile.frameSetExportOrders)) {
                EditorActions<AP::FrameSet>::fieldEdited<
                    &MS::FrameSet::exportOrder>(this);
            }

            ImGui::Unindent();
        }
        ImGui::Spacing();
    }
    ImGui::End();
}

void MetaSpriteEditor::framePropertiesWindow(const Project::ProjectFile& projectFile)
{
    if (ImGui::Begin("Frames##MS")) {
        ImGui::SetWindowSize(ImVec2(325, 650), ImGuiCond_FirstUseEver);

        ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.4f);

        ListButtons<AP::Frames>(this);

        ImGui::SetNextItemWidth(-1);
        ImGui::NamedListListBox("##FrameList", &_framesSel, _data.frames, 8);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (_framesSel.selectedIndex() < _data.frames.size()) {
            MS::Frame& frame = _data.frames.at(_framesSel.selectedIndex());

            {
                ImGui::InputIdstring("Name", &frame.name);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &MS::Frame::name>(this);
                }

                unsigned spriteOrder = frame.spriteOrder;
                if (ImGui::InputUnsigned("Sprite Order", &spriteOrder)) {
                    frame.spriteOrder = std::clamp<unsigned>(spriteOrder, 0, frame.spriteOrder.MASK);
                }
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &MS::Frame::spriteOrder>(this);
                }

                if (ImGui::Checkbox("Solid Tile Hitbox", &frame.solid)) {
                    ListActions<AP::Frames>::selectedFieldEdited<
                        &MS::Frame::solid>(this);
                }
                if (frame.solid) {
                    ImGui::InputMs8rect("Tile Hitbox", &frame.tileHitbox);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        ListActions<AP::Frames>::selectedFieldEdited<
                            &MS::Frame::tileHitbox>(this);
                    }
                }
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

                    const unsigned nSmallTiles = _data.smallTileset.size();
                    const unsigned nLargeTiles = _data.smallTileset.size();

                    for (unsigned i = 0; i < frame.objects.size(); i++) {
                        auto& obj = frame.objects.at(i);

                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_frameObjectsSel, i);
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
                            _tileSelectionValid = false;
                            edited = true;
                        }

                        ImGui::SetNextItemWidth(-1);
                        edited |= ImGui::FlipCombo("##flip", &obj.hFlip, &obj.vFlip);

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

void MetaSpriteEditor::palettesWindow()
{
    const auto& style = ImGui::GetStyle();

    const ImVec2 buttonSize(24, 24);
    const ImVec2 editButtonSize(buttonSize.x * 2 + style.ItemSpacing.x * 1, buttonSize.y);
    const float colorButtonsHeight = buttonSize.y * 3 + style.ItemSpacing.y * 4;

    bool colorSelected = false;

    if (ImGui::Begin("Palettes##MS")) {
        ImGui::SetWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

        {
            if (ListButtons<AP::Palettes>(this)) {
                _paletteValid = false;
            }

            const float scrollHeight = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - style.WindowPadding.y - colorButtonsHeight;
            ImGui::BeginChild("Scroll", ImVec2(0, scrollHeight));

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 40);

            const int palColorSize = ImGui::GetFontSize();
            const ImVec2 palSize(palColorSize * PALETTE_TEXTURE_WIDTH, palColorSize);
            ImVec2 palUv0(0, 0);
            ImVec2 palUv1 = _paletteUvSize;

            for (unsigned i = 0; i < _data.palettes.size(); i++) {
                ImGui::Selectable(&_palettesSel, i, ImGuiSelectableFlags_SpanAllColumns);
                ImGui::NextColumn();

                ImGui::Image(paletteTexture().imguiTextureId(), palSize, palUv0, palUv1);
                ImGui::NextColumn();

                palUv0.y += _paletteUvSize.y;
                palUv1.y += _paletteUvSize.y;
            }

            ImGui::EndChild();
        }
        ImGui::Spacing();

        if (_palettesSel.selectedIndex() < _data.palettes.size()) {
            auto& colors = _data.palettes.at(_palettesSel.selectedIndex()).colors();

            const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoAlpha;

            if (ImGui::ToggledButton("Edit", _paletteState == PaletteState::EDIT_COLOR, editButtonSize)) {
                _paletteState = PaletteState::EDIT_COLOR;
            }
            ImGui::SameLine();

            if (ImGui::ToggledButton("Select", _paletteState == PaletteState::DRAW_TILES, editButtonSize)) {
                _paletteState = PaletteState::DRAW_TILES;
            }

            for (unsigned i = 0; i < colors.size(); i++) {
                ImGui::PushID(i);

                bool buttonClicked = false;
                const ImColor c(colors.at(i).rgb().rgb());
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

void MetaSpriteEditor::colorPopup()
{
    static bool _popupOpen = false;
    static ImVec4 _oldColor;

    const float itemWidth = 80;
    const ImVec2 colorButtonSize(itemWidth, itemWidth * 4 / 6);

    const auto flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview;

    if (ImGui::BeginPopup(colorPopupStrId)) {
        const auto& style = ImGui::GetStyle();

        bool colorChanged = false;

        if (_colorSel < MetaSprite::PALETTE_COLORS
            && _palettesSel.selectedIndex() < _data.palettes.size()) {

            auto& color = _data.palettes.at(_palettesSel.selectedIndex()).colors().at(_colorSel);
            ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(color.rgb().rgb());

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

            unsigned blue = color.blue();
            unsigned green = color.green();
            unsigned red = color.red();

            if (ImGui::InputUnsigned("Blue", &blue)) {
                color.setBlue(std::clamp<unsigned>(blue, 0, 31));
                colorChanged = true;
            }
            if (ImGui::InputUnsigned("Green", &green)) {
                color.setGreen(std::clamp<unsigned>(green, 0, 31));
                colorChanged = true;
            }
            if (ImGui::InputUnsigned("Red", &red)) {
                color.setRed(std::clamp<unsigned>(red, 0, 31));
                colorChanged = true;
            }

            ImGui::PopItemWidth();
            ImGui::EndGroup();

            if (colorVecChanged) {
                color.setRgb(rgba::fromRgba(ImGui::ColorConvertFloat4ToU32(colorVec)));
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
            ListActions<AP::Palettes>::selectedItemEdited(this);
            _popupOpen = false;
            _colorSel = INT_MAX;
        }
    }
}

void MetaSpriteEditor::tilesetButtons()
{
    if (ImGui::Button("Add Small Tile")) {
        ListActions<AP::SmallTileset>::editList(this, EditListAction::ADD);
        _tilesetValid = false;
    }
    ImGui::SameLine();

    if (ImGui::Button("Add Large Tile")) {
        ListActions<AP::LargeTileset>::editList(this, EditListAction::ADD);
        _tilesetValid = false;
    }
    ImGui::SameLine();

    if (ImGui::Button("Remove")) {
        auto tileRemoved = [this](const unsigned tileId, const ObjectSize size) {
            for (unsigned i = 0; i < _data.frames.size(); i++) {
                auto& frame = _data.frames.at(i);

                bool edited = false;
                for (auto& obj : frame.objects) {
                    if (obj.tileId >= tileId && obj.size == size) {
                        obj.tileId--;
                        edited = true;
                    }
                }
                if (edited) {
                    ListActions<AP::Frames>::fieldEdited<
                        &MS::Frame::objects>(this, i);
                }
            }
        };

        // ::TODO combine these actions into a macro::
        if (_smallTilesetSel.hasSelection()) {
            ListActions<AP::SmallTileset>::editList(this, EditListAction::REMOVE);
            tileRemoved(_smallTilesetSel.selectedIndex(), ObjectSize::SMALL);
        }
        if (_largeTilesetSel.hasSelection()) {
            ListActions<AP::LargeTileset>::editList(this, EditListAction::REMOVE);
            tileRemoved(_largeTilesetSel.selectedIndex(), ObjectSize::LARGE);
        }
        _tilesetValid = false;
    }
}

void MetaSpriteEditor::setSelectedFrameObjectsTile(const unsigned tileId, const MetaSpriteEditor::ObjectSize objSize)
{

    if (_framesSel.selectedIndex() < _data.frames.size()) {
        auto& frame = _data.frames.at(_framesSel.selectedIndex());

        if (_frameObjectsSel.hasSelection()) {
            for (unsigned i = 0; i < frame.objects.size(); i++) {
                if (_frameObjectsSel.isSelected(i)) {
                    auto& obj = frame.objects.at(i);
                    obj.tileId = tileId;
                    obj.size = objSize;
                }
            }
            ListActions<AP::Frames>::selectedFieldEdited<
                &MS::Frame::objects>(this);
        }
    }
}

template <typename TilesetPolicy>
void MetaSpriteEditor::drawTileset(const char* label, typename TilesetPolicy::ListT* tileset,
                                   ImDrawList* drawList, const int z, const ImVec2 uv0, const ImVec2 uv1)
{
    constexpr unsigned N_COLORS = 16;
    constexpr unsigned TILE_SIZE = TilesetPolicy::ListT::TILE_SIZE;
    constexpr unsigned TILES_PER_ROW = TILESET_IMAGE_WIDTH / TILE_SIZE;
    const float tileSize = TILE_SIZE * z;

    if (tileset->empty()) {
        const ImVec2 size(TILESET_IMAGE_WIDTH * z, tileSize);
        ImGui::InvisibleButton(label, size);

        return;
    }

    SingleSelection& sel = this->*TilesetPolicy::SelectionPtr;

    const auto& texture = tilesetTexture();

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

    drawList->AddImage(texture.imguiTextureId(), offset, offset + imageSize, uv0, uv1);

    // Draw grid
    {
        const unsigned nFullLines = remaining > 0 ? remaining - 1 : TILES_PER_ROW;

        const float startY = offset.y;
        const float fullEndY = startY + imageSize.y;
        const float partialEndY = fullEndY - tileSize;
        float xPos = offset.x + tileSize;

        for (unsigned x = 0; x < TILES_PER_ROW - 1; x++) {
            if (x < nFullLines) {
                drawList->AddLine(ImVec2(xPos, startY), ImVec2(xPos, fullEndY), tilesetGridColor);
            }
            else {
                if (completeRows == 0) {
                    break;
                }
                drawList->AddLine(ImVec2(xPos, startY), ImVec2(xPos, partialEndY), tilesetGridColor);
            }
            xPos += tileSize;
        }
    }
    {
        const float startX = offset.x;
        const float endX = offset.x + imageSize.x;

        float yPos = offset.y + tileSize;
        if (completeRows > 0) {
            for (unsigned y = 0; y < completeRows - 1; y++) {
                drawList->AddLine(ImVec2(startX, yPos), ImVec2(endX, yPos), tilesetGridColor);
                yPos += tileSize;
            }
        }
        if (remaining > 0) {
            drawList->AddLine(ImVec2(startX, yPos), ImVec2(startX + remaining * tileSize, yPos), tilesetGridColor);
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
            ListActions<TilesetPolicy>::selectedListItemsEdited(this, _editedTiles.vector());
            _editedTiles.empty();
        }
    }
};

void MetaSpriteEditor::tilesetWindow()
{
    if (ImGui::Begin("Tileset##MS")) {
        ImGui::SetWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);

        tilesetButtons();

        const auto& texture = tilesetTexture();

        ImGui::BeginChild("Scroll");

        const int z = std::max<int>(1, ImGui::GetContentRegionAvailWidth() / texture.width());

        auto* drawList = ImGui::GetWindowDrawList();

        // Needed to draw the top line of the selected tile
        ImGui::Spacing();

        drawTileset<AP::SmallTileset>("Small", &_data.smallTileset, drawList,
                                      z, ImVec2(0.0f, 0.0f), _smallTilesetUVmax);

        ImGui::Separator();

        drawTileset<AP::LargeTileset>("Large", &_data.largeTileset, drawList,
                                      z, ImVec2(0.0f, _smallTilesetUVmax.y), _largeTilesetUVmax);

        ImGui::EndChild();
    }
    ImGui::End();
}

void MetaSpriteEditor::frameEditorWindow()
{
    static const std::string windowSuffix = "###MetaSprite_Frame";

    static const rect ms8RectBounds(int_ms8_t::MIN, int_ms8_t::MIN, int_ms8_t::MAX - int_ms8_t::MIN, int_ms8_t::MAX - int_ms8_t::MIN);

    MS::Frame* const frame = _framesSel.selectedIndex() < _data.frames.size()
                                 ? &_data.frames.at(_framesSel.selectedIndex())
                                 : nullptr;

    const std::string windowTitle = frame ? frame->name + windowSuffix : windowSuffix;

    if (ImGui::Begin(windowTitle.c_str())) {
        ImGui::SetWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);
        if (frame == nullptr) {
            ImGui::End();
            return;
        }

        // ::TODO add toolbar::
        ImGui::TextUnformatted("::TODO add toolbar::");
        ImGui::TextUnformatted("::TODO add frame Selection Combo::");

        static_assert(backgroundColorNames.size() == backgroundColors.size());
        ImGui::Combo("Background Color", &_selectedEditorBgColor, backgroundColorNames.data(), backgroundColorNames.size());
        const unsigned bgIndex = std::clamp<int>(_selectedEditorBgColor, 0, backgroundColors.size() - 1);
        const ImU32 bgColor = backgroundColors.at(bgIndex);

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        auto* drawList = ImGui::GetWindowDrawList();

        _graphics.startLoop("##Editor", ms8RectBounds, zoom,
                            &_tileHitboxSel, &_frameObjectsSel, &_actionPointsSel, &_entityHitboxesSel);

        _graphics.drawBackgroundColor(drawList, bgColor);
        _graphics.drawBoundedCrosshair(drawList, 0, 0, crosshairColor);

        const ImTextureID textureId = tilesetTexture().imguiTextureId();

        // ::TODO make layers optional::

        if (true) {
            unsigned i = frame->objects.size();
            while (i > 0) {
                i--;
                auto& obj = frame->objects.at(i);

                bool valid = false;
                ImVec2 uv0, uv1;
                if (obj.size == ObjectSize::SMALL) {
                    valid = obj.tileId < _data.smallTileset.size();
                    uv0.x = unsigned(obj.tileId % SMALL_TILES_PER_ROW) * _smallTilesetUvSize.x;
                    uv0.y = unsigned(obj.tileId / SMALL_TILES_PER_ROW) * _smallTilesetUvSize.y;
                    uv1 = uv0 + _smallTilesetUvSize;
                }
                else {
                    valid = obj.tileId < _data.largeTileset.size();
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

                _graphics.addSquareImage(drawList, &obj.location, obj.sizePx(), textureId, uv0, uv1, frameObjectOutlineCol, &_frameObjectsSel, i);

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

        _graphics.endLoop(drawList,
                          &_tileHitboxSel, &_frameObjectsSel, &_actionPointsSel, &_entityHitboxesSel);

        if (_graphics.isEditingFinished()) {
            // ::TODO add action macros::
            if (_tileHitboxSel.isSelected()) {
                ListActions<AP::Frames>::selectedFieldEdited<&MS::Frame::tileHitbox>(this);
            }
            ListActions<AP::FrameObjects>::selectedItemsEdited(this);
            ListActions<AP::ActionPoints>::selectedItemsEdited(this);
            ListActions<AP::EntityHitboxes>::selectedItemsEdited(this);
        }

        ImGui::EndChild();
    }
    ImGui::End();
}

void MetaSpriteEditor::processGui(const Project::ProjectFile& projectFile)
{
    updatePaletteTexture();
    updateTilesetTexture();
    updateTileSelection();

    frameSetPropertiesWindow(projectFile);
    framePropertiesWindow(projectFile);

    frameEditorWindow();

    palettesWindow();
    tilesetWindow();

    animationPropertiesWindow<AP>("Animations##MS", this, &_data);
    animationPreviewWindow<AP>("Animation Preview##MS", this, &_data);
    exportOrderWindow<AP>("Export Order##MS", this, &_data);

    colorPopup();
}

void MetaSpriteEditor::updateSelection()
{
    AbstractMetaSpriteEditor::updateSelection();

    if (_framesSel.isSelectionChanging()) {
        _tileHitboxSel.clearSelection();
    }

    if (_palettesSel.isSelectionChanging()) {
        _tilesetValid = false;
    }

    if (_paletteState == PaletteState::DRAW_TILES) {
        _smallTilesetSel.clearSelection();
        _largeTilesetSel.clearSelection();
    }
    else if (_smallTilesetSel.isSelectionChanging()) {
        _largeTilesetSel.clearSelection();
    }
    else if (_largeTilesetSel.isSelectionChanging()) {
        _smallTilesetSel.clearSelection();
    }

    if (_frameObjectsSel.isSelectionChanging(_framesSel)) {
    }

    _palettesSel.update();
    _framesSel.update();

    _tileHitboxSel.update();

    if (_frameObjectsSel.isSelectionChanging(_framesSel)) {
        _tileSelectionValid = false;
    }

    _frameObjectsSel.update(_framesSel);
    _actionPointsSel.update(_framesSel);
    _entityHitboxesSel.update(_framesSel);

    _smallTilesetSel.update();
    _largeTilesetSel.update();
}

void MetaSpriteEditor::updatePaletteTexture()
{
    if (_paletteValid) {
        return;
    }

    const usize palSize(PALETTE_TEXTURE_WIDTH,
                        nextPowerOfTwo(_data.palettes.size()));

    if (_paletteImage.size() != palSize) {
        _paletteImage = Image(palSize);
        _paletteUvSize = ImVec2(1.0f, 1.0f / palSize.height);
    }
    _paletteImage.fill(rgba());

    assert(_data.palettes.size() <= _paletteImage.size().height);

    rgba* imgBits = _paletteImage.data();
    const rgba* imgBitsEnd = imgBits + _paletteImage.dataSize();
    for (const auto& palette : _data.palettes) {
        static_assert(PALETTE_TEXTURE_WIDTH == std::remove_reference_t<decltype(palette)>::N_COLORS);

        for (unsigned c = 0; c < palette.N_COLORS; c++) {
            *imgBits++ = palette.color(c).rgb();
        }
    }
    assert(imgBits <= imgBitsEnd);

    paletteTexture().replace(_paletteImage);

    _paletteValid = true;
    _tilesetValid = false;
}

void MetaSpriteEditor::updateTilesetTexture()
{
    static const Snes::Palette4bpp BLANK_PALETTE;

    if (_tilesetValid) {
        return;
    }

    const Snes::Palette4bpp& palette = [&]() -> const auto&
    {
        if (_data.palettes.empty()) {
            return BLANK_PALETTE;
        }
        const auto index = _palettesSel.selectedIndex();
        return index < _data.palettes.size() ? _data.palettes.at(index) : _data.palettes.at(0);
    }
    ();

    const unsigned nSmallRows = (std::max<unsigned>(1, _data.smallTileset.size()) - 1) / SMALL_TILES_PER_ROW + 1;
    const unsigned nLargeRows = (std::max<unsigned>(1, _data.largeTileset.size()) - 1) / LARGE_TILES_PER_ROW + 1;

    const usize tilesetSize(TILESET_IMAGE_WIDTH,
                            nextPowerOfTwo(nSmallRows * SMALL_TILE_SIZE + nLargeRows * LARGE_TILE_SIZE));

    if (_tilesetImage.size() != tilesetSize) {
        _tilesetImage = Image(tilesetSize);
    }
    _tilesetImage.fill(rgba());

    _smallTilesetUvSize = ImVec2(float(SMALL_TILE_SIZE) / tilesetSize.width, float(SMALL_TILE_SIZE) / tilesetSize.height);
    _largeTilesetUvSize = ImVec2(float(LARGE_TILE_SIZE) / tilesetSize.width, float(LARGE_TILE_SIZE) / tilesetSize.height);
    _smallTilesetUVmax = ImVec2(1.0f, _smallTilesetUvSize.y * nSmallRows);
    _largeTilesetUVmax = ImVec2(1.0f, _smallTilesetUVmax.y + _largeTilesetUvSize.y * nLargeRows);

    // ::TODO optimize this (write a specific draw Tileset routine that uses _paletteImage for palette data)::
    unsigned x = 0;
    unsigned y = 0;
    auto drawTiles = [&](const auto& tileset) {
        // ::TODO find out why using foreach loop here does not work::
        for (unsigned i = 0; i < tileset.size(); i++) {
            const auto& tile = tileset.at(i);
            tile.draw(_tilesetImage, palette, x, y);

            x += tile.TILE_SIZE;
            if (x >= TILESET_IMAGE_WIDTH) {
                x = 0;
                y += tile.TILE_SIZE;
            }
        }
    };

    drawTiles(_data.smallTileset);

    x = 0;
    y = nSmallRows * _data.smallTileset.TILE_SIZE;
    drawTiles(_data.largeTileset);

    tilesetTexture().replace(_tilesetImage);

    // ALso update background color
    _paletteBackgroundColor = palette.color(0).rgb().rgbaValue();

    _tilesetValid = true;
}

void MetaSpriteEditor::updateTileSelection()
{
    if (_tileSelectionValid) {
        return;
    }

    if (_framesSel.selectedIndex() < _data.frames.size()) {
        const auto& frame = _data.frames.at(_framesSel.selectedIndex());

        _smallTilesetSel.clearSelection();
        _largeTilesetSel.clearSelection();

        unsigned tileId = INT_MAX;
        ObjectSize objSize = ObjectSize::SMALL;

        if (_frameObjectsSel.hasSelection()) {
            for (unsigned i = 0; i < frame.objects.size(); i++) {
                if (_frameObjectsSel.isSelected(i)) {
                    const auto& obj = frame.objects.at(i);
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
                _smallTilesetSel.setSelected(tileId);
                _smallTilesetSel.update();
            }
            else {
                _largeTilesetSel.setSelected(tileId);
                _largeTilesetSel.update();
            }
        }
    }

    _tileSelectionValid = true;
}

}
