/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metatile-tileset-editor.h"
#include "gui/aptable.h"
#include "gui/editor-actions.h"
#include "gui/graphics/tilecollisionimage.h"
#include "gui/grid-actions.h"
#include "gui/imgui-drawing.h"
#include "gui/imgui-filebrowser.h"
#include "gui/imgui.h"
#include "gui/splitter.hpp"
#include "gui/style.h"
#include "gui/texture.h"
#include "models/common/iterators.h"
#include "models/metatiles/metatiles-error.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/project/project-data.h"
#include <cmath>

namespace UnTech::Gui {

// ::TODO do not recompile tileset if scratchpad changes::

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;

using MetaTileTilesetInput = UnTech::MetaTiles::MetaTileTilesetInput;

// MetaTileTilesetEditor Action Policies
struct MetaTileTilesetEditorData::AP {
    struct MtTileset {
        using EditorT = MetaTileTilesetEditorData;
        using EditorDataT = UnTech::MetaTiles::MetaTileTilesetInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return fileListData(&projectFile.metaTileTilesets, itemIndex.index);
        }
    };

    struct TileCollisions : public MtTileset {
        constexpr static auto FieldPtr = &EditorDataT::tileCollisions;
        constexpr static auto validFlag = &MetaTileTilesetEditorGui::_tileCollisionsValid;
    };

    struct TileFunctionTables : public MtTileset {
        constexpr static auto FieldPtr = &EditorDataT::tileFunctionTables;
        constexpr static auto validFlag = &MetaTileTilesetEditorGui::_tileCollisionsValid;
    };

    struct Scratchpad : public MtTileset {
        using GridT = grid<uint8_t>;
        using ListArgsT = std::tuple<>;

        const static usize MAX_SIZE;
        constexpr static uint8_t DEFAULT_VALUE = 0;

        constexpr static auto SelectionPtr = &EditorT::selectedTiles;

        constexpr static auto validFlag = &MetaTileTilesetEditorGui::_tilemapValid;

        // cppcheck-suppress unusedFunction
        static GridT* getGrid(EditorDataT& editorData) { return &editorData.scratchpad; }
    };

    struct Palettes final : public MtTileset {
        using ListT = std::vector<idstring>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 255;

        constexpr static auto SelectionPtr = &EditorT::paletteSel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.palettes; }
    };

    struct FrameImages final : public MtTileset {
        using ListT = std::vector<std::filesystem::path>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 32;

        constexpr static auto SelectionPtr = &EditorT::tilesetFrameSel;

        constexpr static auto validFlag = &MetaTileTilesetEditorGui::_tilesetShaderImageFilenamesValid;

        static ListT* getList(EditorDataT& editorData) { return &editorData.animationFrames.frameImageFilenames; }
    };
};

const usize MetaTileTilesetEditorData::AP::Scratchpad::MAX_SIZE(UINT8_MAX, UINT8_MAX);

MetaTileTilesetEditorData::MetaTileTilesetEditorData(ItemIndex itemIndex)
    : AbstractMetaTileEditorData(itemIndex)
{
    tilesetFrameSel.setSelected(0);
    paletteSel.setSelected(0);
}

bool MetaTileTilesetEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    const auto [ptr, fn] = fileListItem(&projectFile.metaTileTilesets, itemIndex().index);
    setFilename(fn);
    if (ptr) {
        data = *ptr;
    }
    return ptr != nullptr;
}

void MetaTileTilesetEditorData::saveFile() const
{
    assert(!filename().empty());
    UnTech::MetaTiles::saveMetaTileTilesetInput(data, filename());
}

void MetaTileTilesetEditorData::errorDoubleClicked(const AbstractError* error)
{
    using Type = MetaTiles::TilesetErrorType;

    selectedTilesetTiles.clear();
    selectedTiles.clear();

    tilesetFrameSel.clearSelection();
    paletteSel.clearSelection();

    if (auto* e = dynamic_cast<const MetaTiles::TilesetError*>(error)) {
        switch (e->type) {
        case Type::TILE:
            selectedTilesetTiles.insert(e->firstIndex);
            tilePropertiesWindowValid = false;

            break;
        }
    }

    if (auto* e = dynamic_cast<const MetaTiles::InteractiveTilesError*>(error)) {
        tilesetFrameSel.setSelected(e->firstIndex);
        paletteSel.setSelected(0);
    }
}

void MetaTileTilesetEditorData::updateSelection()
{
    tilesetFrameSel.update();
    paletteSel.update();
}

void MetaTileTilesetEditorGui::selectedTilesChanged()
{
    assert(_data);
    const auto& tileset = _data->data;

    if (!_data->selectedTiles.empty()) {
        const auto& scratchpad = tileset.scratchpad;

        _data->selectedTilesetTiles.clear();
        for (const upoint& p : _data->selectedTiles) {
            if (p.x < scratchpad.width() && p.y < scratchpad.height()) {
                _data->selectedTilesetTiles.insert(scratchpad.at(p));
            }
        }

        _data->tilePropertiesWindowValid = false;
    }
}

MetaTileTilesetEditorGui::MetaTileTilesetEditorGui()
    : AbstractMetaTileEditorGui("##MT Tileset editor")
    , _data(nullptr)
    , _scratchpadSize()
    , _tileProperties(std::nullopt)
    , _sidebar{ 300, 200, 400 }
    , _minimapRight_sidebar{ 320, 280, 400 }
    , _minimapBottom_sidebar{ 320, 280, 400 }
    , _minimapOnRight(true)
    , _tilesetShaderImageFilenamesValid(false)
    , _tileCollisionsValid(false)
    , _interactiveTilesValid(false)
{
}

bool MetaTileTilesetEditorGui::setEditorData(const std::shared_ptr<AbstractEditorData>& data)
{
    AbstractMetaTileEditorGui::setEditorData(data);
    _data = std::dynamic_pointer_cast<MetaTileTilesetEditorData>(data);

    return _data != nullptr;
}

void MetaTileTilesetEditorGui::resetState()
{
    AbstractMetaTileEditorGui::resetState();

    resetTileProperties();

    setEditMode(EditMode::SelectTiles);

    _tilesetShaderImageFilenamesValid = false;
    _tileCollisionsValid = false;
    _interactiveTilesValid = false;
}

void MetaTileTilesetEditorGui::editorClosed()
{
    AbstractMetaTileEditorGui::editorClosed();
}

grid<uint8_t>& MetaTileTilesetEditorGui::map()
{
    assert(_data);
    return _data->data.scratchpad;
}

void MetaTileTilesetEditorGui::mapTilesPlaced(const urect r)
{
    assert(_data);
    const auto& tileset = _data->data;

    assert(tileset.scratchpad.size().contains(r));

    GridActions<AP::Scratchpad>::gridTilesPlaced(_data, r);
}

void MetaTileTilesetEditorGui::selectedTilesetTilesChanged()
{
    assert(_data);
    _data->tilePropertiesWindowValid = false;
}

void MetaTileTilesetEditorGui::propertiesGui(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& tileset = _data->data;

    ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.5f);

    if (Cell("Name", &tileset.name)) {
        EditorActions<AP::MtTileset>::fieldEdited<
            &MetaTileTilesetInput::name>(_data);
    }

    if (Cell("Scratchpad Size", &_scratchpadSize, AP::Scratchpad::MAX_SIZE)) {
        GridActions<AP::Scratchpad>::resizeGrid(_data, _scratchpadSize);
    }

    {
        bool edited = false;

        edited |= Cell("Bit Depth", &tileset.animationFrames.bitDepth);
        edited |= Cell("Animation Delay", &tileset.animationFrames.animationDelay);
        edited |= Cell("Conversion Palette", &tileset.animationFrames.conversionPalette, projectFile.palettes);
        edited |= Cell("Add Transparent Tile", &tileset.animationFrames.addTransparentTile);

        if (edited) {
            EditorActions<AP::MtTileset>::fieldEdited<
                &MetaTileTilesetInput::animationFrames>(_data);
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    {
        ImGui::TextUnformatted(u8"Palettes:");

        ImGui::Indent();

        apTable<AP::Palettes>(
            "Palettes", _data,
            std::to_array({ "Palette" }),
            ImVec2(0, 150),

            [&](auto& p) { return Cell("##Palette", &p, projectFile.palettes); });

        ImGui::Unindent();
    }

    ImGui::Spacing();
    {
        ImGui::TextUnformatted(u8"Frame Images:");

        ImGui::Indent();

        apTable<AP::FrameImages>(
            "Images", _data,
            std::to_array({ "Image Filename" }),
            ImVec2(0, 150),

            [&](auto& fn) { return ImGui::InputPngImageFilename("##Image", &fn); });

        ImGui::Unindent();
    }

    {
        static constexpr std::array<const char*, 2> labels = {
            "Crumbling Tiles Chain A:",
            "Crumbling Tiles Chain B:",
        };

        bool edited = false;

        for (auto [i, ct] : enumerate(tileset.crumblingTiles)) {
            bool thirdTransition = ct.hasThirdTransition();

            ImGui::PushID(labels.at(i));

            ImGui::Spacing();

            ImGui::TextUnformatted(labels.at(i));
            ImGui::Indent();

            edited |= Cell("First Tile Id", &ct.firstTileId);
            edited |= Cell("First Delay", &ct.firstDelay);
            edited |= Cell("Second Tile Id", &ct.secondTileId);

            if (Cell("Third Transition", &thirdTransition)) {
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
                edited |= Cell("Second Delay", &ct.secondDelay);
                edited |= Cell("Third Tile Id", &ct.thirdTileId);
            }

            ImGui::Unindent();
            ImGui::PopID();
        }

        if (edited) {
            static_assert(sizeof(tileset.crumblingTiles) < 100);
            EditorActions<AP::MtTileset>::fieldEdited<
                &MetaTileTilesetInput::crumblingTiles>(_data);
        }
    }

    ImGui::Spacing();
    {
        ImGui::TextUnformatted(u8"Compiled Data:");

        ImGui::Indent();

        if (const auto& td = _tilesetShader.tilesetData()) {
            ImGui::LabelText("Static Tiles", "%u", unsigned(td->animatedTileset.staticTiles.size()));
            ImGui::LabelText("Animated Tiles", "%u", unsigned(td->animatedTileset.nAnimatedTiles()));
        }

        ImGui::Unindent();
    }
}

void MetaTileTilesetEditorGui::selectionChanged()
{
    resetTileProperties();
}

const std::array<idstring, 256>& MetaTileTilesetEditorGui::tileFunctionTables() const
{
    assert(_data);
    return _data->data.tileFunctionTables;
}

void MetaTileTilesetEditorGui::resetTileProperties()
{
    // Update _tileProperties on next `tilePropertiesWindow` call.
    // `_data` may be invalidated sometime after this call
    // but before the Tile Properties window is shown.
    _tileProperties = std::nullopt;
}

void MetaTileTilesetEditorGui::updateTileProperties()
{
    assert(_data);
    auto& tileset = _data->data;
    const auto& selected = _data->selectedTilesetTiles;

    if (selected.empty()) {
        _tileProperties = std::nullopt;
        return;
    }

    if (_tileProperties && _data->tilePropertiesWindowValid) {
        return;
    }

    TileProperties tp;
    tp.tileCollision = tileset.tileCollisions.at(selected.front());
    tp.tileCollisionSame = true;

    tp.functionTable = tileset.tileFunctionTables.at(selected.front());
    tp.functionTableSame = true;

    const auto firstTile = selected.front();
    for (const auto subTile : range(tp.tilePriorities.size())) {
        tp.tilePriorities.at(subTile) = tileset.tilePriorities.getTilePriority(firstTile, subTile);
    }
    tp.tilePrioritiesSame.fill(true);

    for (const auto& tile : skip_first_element(selected)) {
        const auto& tc = tileset.tileCollisions.at(tile);
        if (tc != tp.tileCollision) {
            tp.tileCollisionSame = false;
        }

        for (const auto subTile : range(tp.tilePriorities.size())) {
            const auto c = tileset.tilePriorities.getTilePriority(tile, subTile);
            if (tp.tilePriorities.at(subTile) != c) {
                tp.tilePrioritiesSame.at(subTile) = false;
            }
        }

        const auto& ft = tileset.tileFunctionTables.at(tile);
        if (ft != tp.functionTable) {
            tp.functionTableSame = false;
        }
    }

    _tileProperties = tp;
}

void MetaTileTilesetEditorGui::tileCollisionClicked(const MetaTiles::TileCollisionType tct)
{
    assert(_data);
    auto& tileCollisions = _data->data.tileCollisions;

    if (_data->selectedTilesetTiles.empty()) {
        return;
    }

    for (auto& i : _data->selectedTilesetTiles) {
        tileCollisions.at(i) = tct;
    }

    EditorFieldActions<AP::TileCollisions>::fieldEdited(_data);
}

void MetaTileTilesetEditorGui::tilePriorityClicked(const unsigned subTile, const bool v)
{
    assert(_data);
    auto& tilePriorities = _data->data.tilePriorities;

    if (_data->selectedTilesetTiles.empty()) {
        return;
    }

    for (auto& i : _data->selectedTilesetTiles) {
        tilePriorities.setTilePriority(i, subTile, v);
    }

    EditorActions<AP::MtTileset>::fieldEdited<
        &MetaTileTilesetInput::tilePriorities>(_data);
}

void MetaTileTilesetEditorGui::tileFunctionTableSelected(const idstring& ft)
{
    assert(_data);
    auto& tft = _data->data.tileFunctionTables;

    if (_data->selectedTilesetTiles.empty()) {
        return;
    }

    for (auto& i : _data->selectedTilesetTiles) {
        tft.at(i) = ft;
    }

    EditorFieldActions<AP::TileFunctionTables>::fieldEdited(_data);
}

void MetaTileTilesetEditorGui::tilePropertiesGui(const Project::ProjectFile& projectFile)
{
    assert(_data);

    static const ImVec2 buttonSize = ImVec2(32.0f, 32.0f);

    if (_data->selectedTilesetTiles.empty()) {
        return;
    }

    const auto& style = ImGui::GetStyle();
    const float tileCollisionButtonsWidth = (buttonSize.x + style.FramePadding.x * 2) * 6 + style.ItemSpacing.x * 5;

    updateTileProperties();

    assert(_tileProperties);

    bool edited = false;

    {
        ImGui::TextUnformatted(u8"Tile Collision:");
        ImGui::Indent();
        ImGui::PushID("TC");

        using TC = MetaTiles::TileCollisionType;

        const auto textureId = tileCollisionTypeTexture().imguiTextureId();

        // Image Button background is unused (0 alpha)
        const ImVec4 bgCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        static_assert(TILE_COLLISION_IMAGE_WIDTH == METATILE_SIZE_PX);
        const ImVec2 uvSize = ImVec2(1.0f, 1.0f / int(TILE_COLLISION_IMAGE_HEIGHT / METATILE_SIZE_PX));

        auto button = [&](const char* toolTip, const TC tct) {
            const ImVec2 uv(0.0f, unsigned(tct) * uvSize.y);

            const bool isSelected = _tileProperties->tileCollisionSame && _tileProperties->tileCollision == tct;
            const auto& tint = isSelected ? Style::tilePropertiesButtonTint : Style::tilePropertiesButtonSelectedTint;

            if (ImGui::ToggledImageButton(toolTip, textureId, isSelected, buttonSize, uv, uv + uvSize, bgCol, tint)) {
                tileCollisionClicked(tct);
                edited = true;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::ShowTooltip(toolTip);
            }
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
        ImGui::TextUnformatted(u8"Tile Priority:");
        ImGui::Indent();

        constexpr std::array<const char*, 4> labels = { "##TP_TL", "##TP_TR", "##TP_BL", "##TP_BR" };
        constexpr std::array<const char*, 4> toolTips = { "Top Left", "Top Right", "Bottom Left", "Bottom Right" };

        for (auto [i, sel] : const_enumerate(_tileProperties->tilePriorities)) {
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
                ImGui::ShowTooltip(toolTips.at(i));
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
        ImGui::TextUnformatted(u8"Interactive Tile Functions:");
        ImGui::Indent();

        // Use a different label if the selected tiles have different functions
        const char8_t* comboText = _tileProperties->functionTableSame ? _tileProperties->functionTable.c_str() : u8"------------";

        ImGui::SetNextItemWidth(tileCollisionButtonsWidth);
        if (ImGui::BeginCombo("##ITFT", u8Cast(comboText))) {
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

void MetaTileTilesetEditorGui::tilesetGui()
{
    assert(_data);

    {
        showLayerButtons();
        ImGui::SameLine();

        Style::metaTileTilesetZoom.zoomCombo("##zoom");
        ImGui::SameLine();

        ImGui::ToggledButtonWithTooltip("MR", &_minimapOnRight, "Minimap on Right");
    }

    ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    const ImVec2& zoom = Style::metaTileTilesetZoom.zoom();

    const ImVec2 offset = drawTileset("##Tileset", zoom);

    {
        auto* drawList = ImGui::GetWindowDrawList();

        _invalidTilesCommon.draw(drawList, zoom, offset);
        if (_data->tilesetFrameSel.selectedIndex() < _invalidTilesFrame.size()) {
            _invalidTilesFrame.at(_data->tilesetFrameSel.selectedIndex()).draw(drawList, zoom, offset);
        }
    }

    Style::metaTileTilesetZoom.processMouseWheel();

    ImGui::EndChild();
}

void MetaTileTilesetEditorGui::scratchpadGui()
{
    assert(_data);

    {
        animationButtons();
        ImGui::SameLine(0.0f, 12.0f);

        undoStackButtons();
        ImGui::SameLine(0.0f, 12.0f);

        editModeButtons();
        ImGui::SameLine(0.0f, 12.0f);

        showLayerButtons();
        ImGui::SameLine();

        Style::metaTileScratchpadZoom.zoomCombo("##zoom");
        ImGui::SameLine();

        ImGui::ToggledButtonWithTooltip("MR", &_minimapOnRight, "Minimap on Right");
    }

    ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    drawAndEditMap("##Scratchpad", Style::metaTileScratchpadZoom.zoom());

    Style::metaTileScratchpadZoom.processMouseWheel();

    ImGui::EndChild();
}

void MetaTileTilesetEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData)
{
    if (_data == nullptr) {
        return;
    }

    // Update scratchpadSize if the scratchpad changed
    if (!_tilemapValid) {
        _scratchpadSize = _data->data.scratchpad.size();
    }

    _tilesetShader.setTilesetFrame(_data->tilesetFrameSel.selectedIndex());

    updateMtTilesetShader(projectFile, projectData);
    updateMapAndProcessAnimations();

    auto editorTabs = [&] {
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Tileset")) {
                tilesetGui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Scratchpad")) {
                scratchpadGui();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    };

    splitterSidebarRight(
        "##splitter", &_sidebar, "##Content", [&] {
                if (_minimapOnRight) {
                    splitterSidebarRight(
                        "##mmapsp Right", &_minimapRight_sidebar,
                        "##Editor", editorTabs,
                        "##minimaps",
                        [&] {
                            ImGui::BeginChild("##tileProperties", ImVec2(0, 300), false);
                            tilePropertiesGui(projectFile);
                            ImGui::EndChild();

                            ImGui::Separator();

                            ImGui::BeginChild("##tsMinimap", ImVec2(0, 270), false);
                            tilesetMinimapGui("##ts minimap");
                            ImGui::EndChild();

                            ImGui::Separator();

                            ImGui::BeginChild("##Minimap", ImVec2(0, 0), false);
                            minimapGui("##sp minimap");
                            ImGui::EndChild();
                        });
                }
                else {
                    splitterBottombar(
                        "##mmapsp Bottom", &_minimapBottom_sidebar,
                        "##Editor", editorTabs,
                        "##minimaps",
                        [&] {
                            ImGui::BeginChild("##tileProperties", ImVec2(300, 0), false);
                            tilePropertiesGui(projectFile);
                            ImGui::EndChild();

                            ImGui::SameLineWithSeparator();

                            ImGui::BeginChild("##tsMinimap", ImVec2(270, 0), false);
                            tilesetMinimapGui("##ts minimap");
                            ImGui::EndChild();

                            ImGui::SameLineWithSeparator();

                            ImGui::BeginChild("##Minimap", ImVec2(0, 0), false);
                            minimapGui("##sp minimap");
                            ImGui::EndChild();
                        });
                    } },
        "Sidebar",
        [&] {
            propertiesGui(projectFile);
        });

    if (!_data->tilesetFrameSel.isSelectionChanging()) {
        // Number of frames in the compiled data might not equal frameImageFilenames.size()
        if (_data->data.animationFrames.frameImageFilenames.size() == _tilesetShader.nTilesetFrames()) {
            _data->tilesetFrameSel.setSelected(_tilesetShader.tilesetFrame());
        }
    }
}

void MetaTileTilesetEditorGui::viewMenu()
{
    AbstractMetaTileEditorGui::viewMenu();

    ImGui::Separator();
    ImGui::MenuItem("Minimap on Right", nullptr, &_minimapOnRight);
}

void MetaTileTilesetEditorGui::updateMtTilesetShader(const Project::ProjectFile& projectFile,
                                                     const Project::ProjectData& projectData)
{
    assert(_data);
    auto& mtTileset = _data->data;

    if (_data->paletteSel.selectedIndex() < mtTileset.palettes.size()) {
        const auto& paletteName = mtTileset.palettes.at(_data->paletteSel.selectedIndex());
        const auto palIndexAndData = projectData.palettes.indexAndDataFor(paletteName);

        if (palIndexAndData) {
            _tilesetShader.setPaletteData(palIndexAndData->second);
        }
        else {
            _tilesetShader.setPaletteData(nullptr);
        }
    }

    const auto mtData = projectData.metaTileTilesets.at(_data->itemIndex().index);
    if (mtData != _tilesetShader.tilesetData() || !_tilesetShaderImageFilenamesValid) {
        _tilesetShader.setTilesetData(mtTileset, mtData);
        _tilesetShaderImageFilenamesValid = true;
    }

    if (!_tileCollisionsValid) {
        _tilesetShader.setTileCollisions(mtTileset.tileCollisions);

        _tileCollisionsValid = true;
    }

    if (!_interactiveTilesValid) {
        _tilesetShader.setInteractiveTilesData(mtTileset, projectFile, projectData);

        _interactiveTilesValid = true;
    }
}

void MetaTileTilesetEditorGui::resourceCompiled(const ErrorList& errors)
{
    assert(_data);

    using InvalidImageError = UnTech::Resources::InvalidImageError;

    _invalidTilesCommon.clear();
    _invalidTilesFrame.resize(_data->data.animationFrames.frameImageFilenames.size());
    for (auto& invalidTiles : _invalidTilesFrame) {
        invalidTiles.clear();
    }

    for (const auto& errorItem : errors.list()) {
        if (auto* imgErr = dynamic_cast<const InvalidImageError*>(errorItem.get())) {
            if (imgErr->frameId) {
                const auto fid = imgErr->frameId.value();
                if (fid < _invalidTilesFrame.size()) {
                    _invalidTilesFrame.at(fid).append(*imgErr);
                }
            }
            else {
                _invalidTilesCommon.append(*imgErr);
            }
        }
    }
}

}
