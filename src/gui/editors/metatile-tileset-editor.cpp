/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatile-tileset-editor.h"
#include "gui/common/tilecollisionimage.h"
#include "gui/editor-actions.h"
#include "gui/imgui-drawing.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/texture.h"
#include <cmath>

namespace UnTech::Gui {

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;

using MetaTileTilesetInput = UnTech::MetaTiles::MetaTileTilesetInput;

std::optional<MetaTileTilesetEditor::TileProperties> MetaTileTilesetEditor::_tileProperties;

// MetaTileTilesetEditor Action Policies
struct MetaTileTilesetEditor::AP {
    struct MtTileset {
        using EditorT = MetaTileTilesetEditor;
        using EditorDataT = UnTech::MetaTiles::MetaTileTilesetInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return fileListData(&projectFile.metaTileTilesets, itemIndex.index);
        }
    };

    struct Palettes final : public MtTileset {
        using ListT = std::vector<idstring>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 255;

        constexpr static auto SelectionPtr = &EditorT::_paletteSel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.palettes; }
    };

    struct FrameImages final : public MtTileset {
        using ListT = std::vector<std::filesystem::path>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 32;

        constexpr static auto SelectionPtr = &EditorT::_tilesetFrameSel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.animationFrames.frameImageFilenames; }
    };
};

MetaTileTilesetEditor::MetaTileTilesetEditor(ItemIndex itemIndex)
    : AbstractMetaTileEditor(itemIndex)
{
    setTilesetIndex(itemIndex.index);
}

bool MetaTileTilesetEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    // This function is called before undo/redo. Invalidate data so it can recalulated next frame.
    resetState();
    resetTileProperties();

    const auto* mt = fileListData(&projectFile.metaTileTilesets, itemIndex().index);
    if (mt) {
        _data = *mt;
    }
    return mt != nullptr;
}

void MetaTileTilesetEditor::editorOpened()
{
    AbstractMetaTileEditor::editorOpened();

    _scratchpadSize = _data.scratchpad.size();
    resetTileProperties();
}

void MetaTileTilesetEditor::editorClosed()
{
}

void MetaTileTilesetEditor::propertiesWindow(const Project::ProjectFile& projectFile)
{
    if (ImGui::Begin("MetaTile Tileset Properties")) {
        ImGui::SetWindowSize(ImVec2(350, 650), ImGuiCond_FirstUseEver);

        ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.5f);

        ImGui::InputIdstring("Name", &_data.name);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            EditorActions<AP::MtTileset>::fieldEdited<
                &MetaTileTilesetInput::name>(this);
        }

        ImGui::InputUsize("Scratchpad Size", &_scratchpadSize, usize(255, 255));
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            // ::TODO use a GridAction to resize scratchpad::
            _data.scratchpad = _data.scratchpad.resized(_scratchpadSize, 0);
            EditorActions<AP::MtTileset>::fieldEdited<
                &MetaTileTilesetInput::scratchpad>(this);
        }
        if (!ImGui::IsItemActive()) {
            // ::TODO use callback to update scratchpad size::
            _scratchpadSize = _data.scratchpad.size();
        }

        {
            bool edited = false;

            if (ImGui::InputUnsigned("Bit Depth", &_data.animationFrames.bitDepth, 0, 0, "%u bpp")) {
                if (_data.animationFrames.isBitDepthValid() == false) {
                    _data.animationFrames.bitDepth = 4;
                }
            }
            edited |= ImGui::IsItemDeactivatedAfterEdit();

            ImGui::InputUnsigned("Animation Delay", &_data.animationFrames.animationDelay, 0, 0);
            edited |= ImGui::IsItemDeactivatedAfterEdit();

            edited |= ImGui::IdStringCombo("Conversion Palette", &_data.animationFrames.conversionPalette, projectFile.palettes);

            edited |= ImGui::Checkbox("Add Transparent Tile", &_data.animationFrames.addTransparentTile);

            if (edited) {
                EditorActions<AP::MtTileset>::fieldEdited<
                    &MetaTileTilesetInput::animationFrames>(this);
            }
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted("Palettes:");

            ImGui::PushID("Palettes");
            ListButtons<AP::Palettes>(this);
            ImGui::PopID();

            ImGui::Indent();
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 40);

            ImGui::PushID("Palettes");

            for (unsigned i = 0; i < _data.palettes.size(); i++) {
                auto& palette = _data.palettes.at(i);

                ImGui::PushID(i);

                ImGui::Selectable(&_paletteSel, i);
                ImGui::NextColumn();

                ImGui::SetNextItemWidth(-1);
                if (ImGui::IdStringCombo("##Palette", &palette, projectFile.palettes)) {
                    ListActions<AP::Palettes>::itemEdited(this, i);
                }
                ImGui::NextColumn();

                ImGui::PopID();
            }

            ImGui::PopID();
            ImGui::Columns(1);
            ImGui::Unindent();
        }

        ImGui::Spacing();
        {
            ImGui::TextUnformatted("Frame Images:");

            ImGui::PushID("FrameImages");
            ListButtons<AP::FrameImages>(this);
            ImGui::PopID();

            ImGui::Indent();
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 40);

            ImGui::PushID("FrameImages");

            for (unsigned i = 0; i < _data.animationFrames.frameImageFilenames.size(); i++) {
                auto& imageFilename = _data.animationFrames.frameImageFilenames.at(i);

                ImGui::PushID(i);

                ImGui::Selectable(&_tilesetFrameSel, i);
                ImGui::NextColumn();

                // ::TODO filename input::
                std::string fn = imageFilename;
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##Image", &fn);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    imageFilename = fn;
                    ListActions<AP::FrameImages>::itemEdited(this, i);
                }
                ImGui::NextColumn();

                ImGui::PopID();
            }

            ImGui::PopID();
            ImGui::Columns(1);
            ImGui::Unindent();
        }

        {
            static constexpr std::array<const char*, 2> labels = {
                "Crumbling Tiles Chain A:",
                "Crumbling Tiles Chain B:",
            };

            bool edited = false;

            for (unsigned i = 0; i < _data.crumblingTiles.size(); i++) {
                auto& ct = _data.crumblingTiles.at(i);
                bool thirdTransition = ct.hasThirdTransition();

                ImGui::PushID(labels.at(i));

                ImGui::Spacing();

                ImGui::TextUnformatted(labels.at(i));
                ImGui::Indent();

                ImGui::InputUint8("First Tile Id", &ct.firstTileId, 0, 0);
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::InputUint16("First Delay", &ct.firstDelay, 0, 0);
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::InputUint8("Second Tile Id", &ct.secondTileId, 0, 0);
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                if (ImGui::Checkbox("Third Transition", &thirdTransition)) {
                    if (thirdTransition == true) {
                        if (ct.secondDelay == ct.NO_THIRD_TRANSITION) {
                            ct.secondDelay = 600;
                        }
                    }
                    else {
                        ct.secondDelay = ct.NO_THIRD_TRANSITION;
                    }
                    edited = true;
                }
                if (thirdTransition) {
                    ImGui::InputUint16("Second Delay", &ct.secondDelay, 0, 0);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    ImGui::InputUint8("Third Tile Id", &ct.thirdTileId, 0, 0);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();
                }

                ImGui::Unindent();
                ImGui::PopID();
            }

            if (edited) {
                static_assert(sizeof(_data.crumblingTiles) < 100);
                EditorActions<AP::MtTileset>::fieldEdited<
                    &MetaTileTilesetInput::crumblingTiles>(this);
            }
        }
    }

    // ::TODO number of static and animated tiles in compiled data::

    ImGui::End();
}

void MetaTileTilesetEditor::resetTileProperties()
{
    // Update _tileProperties on next `tilePropertiesWindow` call.
    // `_data` may be invalidated sometime after this call
    // but before the Tile Properties window is shown.
    _tileProperties = std::nullopt;
}

void MetaTileTilesetEditor::updateTileProperties()
{
    const auto& selected = _selectedTilesetTiles;

    if (selected.empty()) {
        _tileProperties = std::nullopt;
        return;
    }

    TileProperties tp;
    tp.tileCollision = _data.tileCollisions.at(selected.front());
    tp.tileCollisionSame = true;

    tp.functionTable = _data.tileFunctionTables.at(selected.front());
    tp.functionTableSame = true;

    const auto firstTile = selected.front();
    for (unsigned subTile = 0; subTile < tp.tilePriorities.size(); subTile++) {
        tp.tilePriorities.at(subTile) = _data.tilePriorities.getTilePriority(firstTile, subTile);
    }
    tp.tilePrioritiesSame.fill(true);

    for (auto it = selected.begin() + 1; it != selected.end(); it++) {
        const auto tile = *it;

        const auto& tc = _data.tileCollisions.at(tile);
        if (tc != tp.tileCollision) {
            tp.tileCollisionSame = false;
        }

        for (unsigned subTile = 0; subTile < tp.tilePriorities.size(); subTile++) {
            const auto c = _data.tilePriorities.getTilePriority(tile, subTile);
            if (tp.tilePriorities.at(subTile) != c) {
                tp.tilePrioritiesSame.at(subTile) = false;
            }
        }

        const auto& ft = _data.tileFunctionTables.at(tile);
        if (ft != tp.functionTable) {
            tp.functionTableSame = false;
        }
    }

    _tileProperties = tp;
}

void MetaTileTilesetEditor::tileCollisionClicked(const MetaTiles::TileCollisionType tct)
{
    if (_selectedTilesetTiles.empty()) {
        return;
    }

    for (auto& i : _selectedTilesetTiles) {
        _data.tileCollisions.at(i) = tct;
    }

    // ::TODO add set field array items action::
    EditorActions<AP::MtTileset>::fieldEdited<
        &MetaTileTilesetInput::tileCollisions>(this);

    markCollisionTextureOutOfDate();
}

void MetaTileTilesetEditor::tilePriorityClicked(const unsigned subTile, const bool v)
{
    if (_selectedTilesetTiles.empty()) {
        return;
    }

    for (auto& i : _selectedTilesetTiles) {
        _data.tilePriorities.setTilePriority(i, subTile, v);
    }

    EditorActions<AP::MtTileset>::fieldEdited<
        &MetaTileTilesetInput::tilePriorities>(this);
}

void MetaTileTilesetEditor::tileFunctionTableSelected(const idstring& ft)
{
    if (_selectedTilesetTiles.empty()) {
        return;
    }

    for (auto& i : _selectedTilesetTiles) {
        _data.tileFunctionTables.at(i) = ft;
    }

    // ::TODO add set field array items action::
    EditorActions<AP::MtTileset>::fieldEdited<
        &MetaTileTilesetInput::tileCollisions>(this);
}

void MetaTileTilesetEditor::tilePropertiesWindow(const Project::ProjectFile& projectFile)
{
    static const ImVec2 buttonSize = ImVec2(32.0f, 32.0f);

    if (ImGui::Begin("Tile Properties")) {
        ImGui::SetWindowSize(ImVec2(350, 650), ImGuiCond_FirstUseEver);

        if (_selectedTilesetTiles.empty()) {
            ImGui::End();
            return;
        }

        const auto& style = ImGui::GetStyle();
        const float tileCollisionButtonsWidth = (buttonSize.x + style.FramePadding.x * 2) * 6 + style.ItemSpacing.x * 5;

        if (!_tileProperties) {
            updateTileProperties();
        }
        assert(_tileProperties);

        bool edited = false;

        {
            ImGui::TextUnformatted("Tile Collision:");
            ImGui::Indent();
            ImGui::PushID("TC");

            using TC = MetaTiles::TileCollisionType;

            const auto textureId = tileCollisionTypeTexture().imguiTextureId();

            const ImVec4 bgCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

            // ::TODO move into style::
            // ::TODO better name::
            const ImVec4 tintCol = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
            const ImVec4 selectedTintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

            static_assert(TILE_COLLISION_IMAGE_WIDTH == METATILE_SIZE_PX);
            const ImVec2 uvSize = ImVec2(1.0f, 1.0f / (TILE_COLLISION_IMAGE_HEIGHT / METATILE_SIZE_PX));

            auto button = [&](const char* toolTip, const TC tct) {
                const ImVec2 uv(0.0f, unsigned(tct) * uvSize.y);

                const bool isSelected = _tileProperties->tileCollisionSame && _tileProperties->tileCollision == tct;
                const auto& tint = isSelected ? selectedTintCol : tintCol;

                ImGui::PushID(unsigned(tct));

                if (ImGui::ToggledImageButton(textureId, isSelected, buttonSize, uv, uv + uvSize, -1, bgCol, tint)) {
                    tileCollisionClicked(tct);
                    edited = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(toolTip);
                    ImGui::EndTooltip();
                }

                ImGui::PopID();
            };
            button("No Collisions", TC::EMPTY);
            ImGui::SameLine();
            button("Solid", TC::SOLID);
            ImGui::SameLine();
            button("Up Platform", TC::UP_PLATFORM);
            ImGui::SameLine();
            button("Down Platform", TC::DOWN_PLATFORM);
            ImGui::SameLine();
            button("End Slope", TC::END_SLOPE);

            button("Down Right Slope", TC::DOWN_RIGHT_SLOPE);
            ImGui::SameLine();
            button("Down Left Slope", TC::DOWN_LEFT_SLOPE);
            ImGui::SameLine();
            button("Down Right Short Slope", TC::DOWN_RIGHT_SHORT_SLOPE);
            ImGui::SameLine();
            button("Down Right Tall Slope", TC::DOWN_RIGHT_TALL_SLOPE);
            ImGui::SameLine();
            button("Down Left Tall Slope", TC::DOWN_LEFT_TALL_SLOPE);
            ImGui::SameLine();
            button("Down Left Sort Slope", TC::DOWN_LEFT_SHORT_SLOPE);

            button("Up Right Slope", TC::UP_RIGHT_SLOPE);
            ImGui::SameLine();
            button("Up Left Slope", TC::UP_LEFT_SLOPE);
            ImGui::SameLine();
            button("Up Right Short Slope", TC::UP_RIGHT_SHORT_SLOPE);
            ImGui::SameLine();
            button("Up Right Tall Slope", TC::UP_RIGHT_TALL_SLOPE);
            ImGui::SameLine();
            button("Up Left Tall Slope", TC::UP_LEFT_TALL_SLOPE);
            ImGui::SameLine();
            button("Up Left Short Slope", TC::UP_LEFT_SHORT_SLOPE);

            ImGui::PopID();
            ImGui::Unindent();
            ImGui::Spacing();
        }

        {
            ImGui::TextUnformatted("Tile Priority:");
            ImGui::Indent();

            constexpr std::array<const char*, 4> labels = { "##TP_TL", "##TP_TR", "##TP_BL", "##TP_BR" };
            constexpr std::array<const char*, 4> toolTips = { "Top Left", "Top Right", "Bottom Left", "Bottom Right" };

            for (unsigned i = 0; i < _tileProperties->tilePriorities.size(); i++) {
                const bool sel = _tileProperties->tilePriorities.at(i);
                const bool allSame = _tileProperties->tilePrioritiesSame.at(i);

                if (!allSame) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                }
                else if (sel) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                }

                if (ImGui::Button(labels.at(i), buttonSize)) {
                    tilePriorityClicked(i, !sel);
                    edited = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(toolTips.at(i));
                    ImGui::EndTooltip();
                }

                if (sel || !allSame) {
                    ImGui::PopStyleColor();
                }

                if (i % 2 == 0) {
                    ImGui::SameLine();
                }
            }

            ImGui::Unindent();
            ImGui::Spacing();
        }

        {
            ImGui::TextUnformatted("Interactive Tile Functions:");
            ImGui::Indent();

            // Use a different label if the selected tiles have different functions
            const char* comboText = _tileProperties->functionTableSame ? _tileProperties->functionTable.str().c_str() : "------------";

            ImGui::SetNextItemWidth(tileCollisionButtonsWidth);
            if (ImGui::BeginCombo("##ITFT", comboText)) {
                if (ImGui::IdStringComboSelection(&_tileProperties->functionTable, projectFile.interactiveTiles.functionTables, true)) {
                    tileFunctionTableSelected(_tileProperties->functionTable);
                    edited = true;
                }
                ImGui::EndCombo();
            }

            ImGui::Unindent();
            ImGui::Spacing();
        }

        if (edited) {
            resetTileProperties();
        }
    }
    ImGui::End();
}

void MetaTileTilesetEditor::tilesetWindow()
{
    if (ImGui::Begin("MetaTile Tileset")) {
        ImGui::SetWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);

        // ::TODO toolbar::
        ImGui::TextUnformatted("::TODO ToolBar::");

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        // ::TODO zoom::
        auto geo = tilesetGeometryAutoZoom();
        drawTileset(geo);

        ImGui::EndChild();
    }
    ImGui::End();
}

void MetaTileTilesetEditor::scratchpadWindow()
{
    if (ImGui::Begin("Scratchpad")) {
        ImGui::SetWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);

        editModeButtons();
        // ::TODO expand toolbar::
        ImGui::SameLine();
        ImGui::TextUnformatted("::TODO expand toolBar::");

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        // ::TODO zoom::
        const auto geo = mapGeometryAutoZoom(_data.scratchpad.size());
        drawAndEditMap(geo);

        ImGui::EndChild();
    }
    ImGui::End();
}

void MetaTileTilesetEditor::processGui(const Project::ProjectFile& projectFile)
{
    updateTextures(projectFile);

    propertiesWindow(projectFile);
    tilePropertiesWindow(projectFile);

    tilesetWindow();
    scratchpadWindow();

    tilesetMinimapWindow("Minimap###Tileset_MiniMap");
    minimapWindow("Scratchpad Minimap###Tileset_Scratchpad_MiniMap");
}

void MetaTileTilesetEditor::updateSelection()
{
    AbstractMetaTileEditor::updateSelection();
}

grid<uint8_t>& MetaTileTilesetEditor::map()
{
    return _data.scratchpad;
}

void MetaTileTilesetEditor::mapTilesPlaced(const urect r)
{
    assert(_data.scratchpad.size().contains(r));

    // ::TODO add grid editor action::
    EditorActions<AP::MtTileset>::fieldEdited<
        &MetaTileTilesetInput::scratchpad>(this);
}

void MetaTileTilesetEditor::selectedTilesetTilesChanged()
{
    resetTileProperties();
}

void MetaTileTilesetEditor::selectedTilesChanged()
{
    if (!_selectedTiles.empty()) {
        const auto& scratchpad = _data.scratchpad;

        _selectedTilesetTiles.clear();
        for (const upoint& p : _selectedTiles) {
            if (p.x < scratchpad.width() && p.y < scratchpad.height()) {
                _selectedTilesetTiles.insert(scratchpad.at(p));
            }
        }

        selectedTilesetTilesChanged();
    }
}

}
