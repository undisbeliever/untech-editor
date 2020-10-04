/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "scenes.h"
#include "background-image.h"
#include "scene-bgmode.hpp"
#include "models/common/errorlist.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/project/project-data.h"
#include <numeric>

namespace UnTech {
namespace Resources {

// Scene Settings
// --------------

static constexpr uint8_t bgModeByte(BgMode mode)
{
    // All tiles are 8x8 pixels in size

    switch (mode) {
    case BgMode::MODE_0:
        return 0;

    case BgMode::MODE_1:
        return 1;

    case BgMode::MODE_1_BG3_PRIOTITY:
        return 1 | 0b1000;

    case BgMode::MODE_2:
        return 2;

    case BgMode::MODE_3:
        return 3;

    case BgMode::MODE_4:
        return 4;
    }

    return 0xff;
}

bool SceneSettingsInput::validate(ErrorList& err) const
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        err.addError(std::make_unique<ListItemError>(this, msg...));
        valid = false;
    };

    if (name.isValid() == false) {
        addError("Missing name");
    }

    std::array<unsigned, N_LAYER_TYPES> layerTypeCount;
    layerTypeCount.fill(0);

    for (const LayerType& lt : layerTypes) {
        unsigned i = unsigned(lt);
        layerTypeCount.at(i)++;
    }

    if (layerTypeCount.at(unsigned(LayerType::TextConsole)) > 1) {
        addError("Invalid setting ", name, ": Cannot have more than one text layer");
    }

    unsigned nAnimatedLayers = layerTypeCount.at(unsigned(LayerType::BackgroundImage));
    if (nAnimatedLayers > 1) {
        addError("Invalid setting ", name, ": Cannot have more than one animated layer (MetaTiles are animated)");
    }

    return valid;
}

void writeSceneIncData(const ResourceScenes& resourceScenes, std::ostream& out)
{
    out << "code()\n"
        << SceneSettingsData::FUNCTION_TABLE_LABEL << ":\n";
    for (const SceneSettingsInput& ssi : resourceScenes.settings) {
        out << "\tdw\t"
            << "Scenes." << ssi.name << ".SetupPpu_dp2100, "
            << "Scenes." << ssi.name << ".Process, "
            << "Scenes." << ssi.name << ".VBlank_dp2100\n";
    }
    out << "constant Project.SceneSettingsFunctionTable.size = pc() - Project.SceneSettingsFunctionTable\n"
           "\n\n"

           "namespace Project.Scenes {\n";
    unsigned i = 0;
    for (const SceneInput& si : resourceScenes.scenes) {
        out << "\tconstant " << si.name << " = " << i++ << '\n';
    }
    assert(i == resourceScenes.scenes.size());
    out << "}\n\n";
}
constexpr unsigned FUNCTION_TABLE_ELEMENT_SIZE = 6;
constexpr unsigned SCENE_SETTINGS_DATA_ELEMENT_SIZE = 3;
static_assert(MAX_N_SCENE_SETTINGS == 255 / std::max(FUNCTION_TABLE_ELEMENT_SIZE, SCENE_SETTINGS_DATA_ELEMENT_SIZE));

std::shared_ptr<const SceneSettingsData>
compileSceneSettingsData(const NamedList<SceneSettingsInput>& settings, ErrorList& err)
{
    auto out = std::make_unique<SceneSettingsData>();

    out->valid = true;
    auto addError = [&](const SceneSettingsInput& ssi, const auto&... msg) {
        err.addError(std::make_unique<ListItemError>(&ssi, msg...));
        out->valid = false;
    };

    if (settings.size() > MAX_N_SCENE_SETTINGS) {
        err.addErrorString("Too many settings (", settings.size(), "max = ", MAX_N_SCENE_SETTINGS);
        out->valid = false;
    }

    for (const SceneSettingsInput& ssi : settings) {
        out->valid &= ssi.validate(err);
    }

    for (unsigned id = 0; id < settings.size(); id++) {
        const SceneSettingsInput& ssi = settings.at(id);

        if (ssi.name.isValid()) {
            const auto r = out->nameIndexMap.try_emplace(ssi.name, id);
            if (r.second == false) {
                addError(ssi, "Duplicate Scene Settings name:", ssi.name);
            }
        }
    }
    if (!out->valid) {
        return out;
    }

    out->sceneSettings.resize(settings.size() * SCENE_SETTINGS_DATA_ELEMENT_SIZE);

    auto ssDataIt = out->sceneSettings.begin();
    for (const SceneSettingsInput& ssi : settings) {
        const uint8_t bgMode = bgModeByte(ssi.bgMode);
        if (bgMode >= 0xff) {
            addError(ssi, "Invalid bgMode");
        }

        unsigned layerTypes = 0;
        for (unsigned i = 0; i < N_LAYERS; i++) {
            const auto& lt = ssi.layerTypes.at(i);
            layerTypes |= (unsigned(lt) & 0x7) << (i * 4 + 1);
        }
        if (layerTypes >= 0xffff) {
            addError(ssi, "Invalid layerTypes");
        }

        // Must update CompiledScenesData::SCENE_FORMAT_VERSION if data format changes
        *ssDataIt++ = bgMode;
        *ssDataIt++ = layerTypes & 0xff;
        *ssDataIt++ = layerTypes >> 8;
    }
    assert(ssDataIt == out->sceneSettings.end());

    out->nSceneSettings = settings.size();

    if (!out->valid) {
        out->sceneSettings.clear();
    }

    return out;
}

// SceneLayoutData
// ---------------

// thows std::logic_error if layout is not valid
static void confirmLayoutIsValid(const std::array<SceneLayoutsData::LayerLayout, N_LAYERS>& layers)
{
    std::array<bool, SceneLayoutsData::N_VRAM_BLOCKS> usedBlocks;
    usedBlocks.fill(false);

    auto testBlock = [&](const unsigned start, const unsigned size) {
        for (unsigned i = start; i < (start + size); i++) {
            if (usedBlocks.at(i)) {
                throw std::logic_error("Layout invalid");
            }
            usedBlocks.at(i) = true;
        }
    };

    for (const auto& l : layers) {
        testBlock(l.tileStart, l.nTileBlocks);
        testBlock(l.mapStart, l.nMapBlocks);
    }
}

inline SceneLayoutsData::LayerInput::LayerInput(const SceneLayerData& lds)
    : nTileBlocks(0)
    , nMapBlocks(lds.nMaps * BLOCKS_PER_MAP)
    , mapSizeBits(lds.mapHorizontalMirroring | (lds.mapVerticalMirroring << 1))
{
    if (lds.tileSize > 0) {
        nTileBlocks = (lds.tileSize - 1) / BLOCK_SIZE + 1;
    }
}

constexpr unsigned SCENE_LAYOUT_DATA_ENTRY_SIZE = 8;

inline std::optional<uint8_t> SceneLayoutsData::addLayout(const std::array<SceneLayoutsData::LayerInput, N_LAYERS>& input)
{
    constexpr unsigned UNUSED_BLOCK_ID = UINT8_MAX;
    static_assert(N_VRAM_BLOCKS < UNUSED_BLOCK_ID);

    // Use a First-fit algorithm to fill the VRAM and calculate the base for each layer
    // Returns with std::nullopt if a layout could not be found
    struct Base {
        unsigned map;
        unsigned tiles;
    };
    std::array<Base, N_LAYERS> layerBases;

    std::array<bool, N_VRAM_BLOCKS> freeBlocks;
    freeBlocks.fill(true);

    auto findFreeSpaceForwards = [&](const unsigned nBlocks, const unsigned align) -> std::optional<unsigned> {
        if (nBlocks == 0) {
            return UNUSED_BLOCK_ID;
        }

        if (nBlocks > freeBlocks.size()) {
            throw std::runtime_error("nBlocks is too large");
        }

        for (unsigned b = 0; b < freeBlocks.size(); b += align) {
            auto it = freeBlocks.begin() + b;
            auto endIt = it + nBlocks;

            unsigned count = std::count(it, endIt, true);
            if (count == nBlocks) {
                std::fill(it, endIt, false);
                return b;
            }
        }
        return std::nullopt;
    };

    auto findFreeSpaceBackwards = [&](const unsigned nBlocks, const unsigned align) -> std::optional<unsigned> {
        if (nBlocks == 0) {
            return UNUSED_BLOCK_ID;
        }

        if (nBlocks > freeBlocks.size()) {
            throw std::runtime_error("nBlocks is too large");
        }

        unsigned b = freeBlocks.size() - align;
        while (b + nBlocks >= freeBlocks.size()) {
            b -= align;
        }

        while (b > nBlocks) {
            auto it = freeBlocks.begin() + b;
            auto endIt = it + nBlocks;

            unsigned count = std::count(it, endIt, true);
            if (count == nBlocks) {
                std::fill(it, endIt, false);
                return b;
            }

            b -= nBlocks;
        }
        return std::nullopt;
    };

    for (unsigned layerId = 0; layerId < N_LAYERS; layerId++) {
        auto b = findFreeSpaceForwards(input.at(layerId).nTileBlocks, TILE_ALIGN);
        if (b == std::nullopt) {
            return std::nullopt;
        }
        layerBases.at(layerId).tiles = *b;
    }

    // ::TODO sort layers by size of maps (insertation sort)::

    for (unsigned layerId = 0; layerId < N_LAYERS; layerId++) {
        // Search backwards through vram to increase the amount of free tiles available for BG 1 and BG 2
        auto b = findFreeSpaceBackwards(input.at(layerId).nTileBlocks, MAP_ALIGN);
        if (b == std::nullopt) {
            return std::nullopt;
        }
        layerBases.at(layerId).map = *b;
    }

    // Create LayerLayout and Scene Layout Data for the calculated bases
    std::array<LayerLayout, N_LAYERS> layout;
    std::array<uint8_t, SCENE_LAYOUT_DATA_ENTRY_SIZE> layoutData;
    layoutData.fill(0);

    // Must update CompiledScenesData::SCENE_FORMAT_VERSION if data format changes
    auto layoutDataIt = layoutData.begin();

    static_assert(SCENE_LAYOUT_DATA_ENTRY_SIZE == 2 * N_LAYERS);

    for (unsigned layerId = 0; layerId < N_LAYERS; layerId++) {
        const auto& source = input.at(layerId);
        const auto& base = layerBases.at(layerId);
        auto& dest = layout.at(layerId);

        if (base.map != UNUSED_BLOCK_ID) {
            dest.mapStart = base.map;
            dest.nMapBlocks = source.nMapBlocks;
            dest.mapSizeBits = source.mapSizeBits;

            assert(base.map % MAP_ALIGN == 0);
            assert(base.map + source.nMapBlocks <= N_VRAM_BLOCKS);

            const unsigned sc = (base.map / MAP_ALIGN) << 2
                                | (source.mapSizeBits & 3);
            assert(sc < 256);

            *layoutDataIt++ = sc;
        }
        else {
            assert(source.nMapBlocks == 0);

            dest.mapStart = 0;
            dest.nMapBlocks = 0;
            dest.mapSizeBits = 0xf0; // bottom 2 bits clear, single map mode on SNES and prevents matches against SceneLayerData.

            *layoutDataIt++ = 0;
        }

        if (base.tiles != UNUSED_BLOCK_ID) {
            unsigned nTileBlocks = source.nTileBlocks;

            assert(base.tiles % TILE_ALIGN == 0);
            assert(base.tiles + nTileBlocks <= N_VRAM_BLOCKS);

            // Find amount of free space after the tiles data.
            auto it = std::find(freeBlocks.begin() + base.tiles + nTileBlocks, freeBlocks.end(), false);
            if (it != freeBlocks.end()) {
                const int d = std::distance(freeBlocks.begin(), it);
                assert(d > 0 && unsigned(d) > base.tiles);
                const unsigned newNBlocks = d - base.tiles;
                assert(newNBlocks >= nTileBlocks);
                nTileBlocks = newNBlocks;
            }

            dest.tileStart = base.tiles;
            dest.nTileBlocks = nTileBlocks;

            const unsigned bgNba = base.tiles / TILE_ALIGN;
            assert(bgNba <= 0x0f);

            *layoutDataIt++ = bgNba;
        }
        else {
            assert(source.nTileBlocks == 0);

            dest.tileStart = 0;
            dest.nTileBlocks = 0;

            *layoutDataIt++ = 0;
        }
    }
    assert(layoutDataIt == layoutData.end());

    confirmLayoutIsValid(layout);

    if (_sceneLayouts.size() >= UINT8_MAX) {
        throw std::runtime_error("Too many scene layouts");
    }
    const uint8_t layoutId = _sceneLayouts.size();

    _sceneLayouts.emplace_back(layout);
    _sceneLayoutData.insert(_sceneLayoutData.end(), layoutData.begin(), layoutData.end());

    return layoutId;
}

inline std::optional<uint8_t> SceneLayoutsData::findOrAdd(const std::array<SceneLayoutsData::LayerInput, N_LAYERS>& input)
{
    for (unsigned layoutId = 0; layoutId < _sceneLayouts.size(); layoutId++) {
        const auto& layout = _sceneLayouts.at(layoutId);

        bool match = true;
        for (unsigned i = 0; i < N_LAYERS; i++) {
            match &= layout.at(i).mapSizeBits == input.at(i).mapSizeBits;
            match &= layout.at(i).nMapBlocks >= input.at(i).nMapBlocks;
            match &= layout.at(i).nTileBlocks >= input.at(i).nTileBlocks;
        }
        if (match) {
            return layoutId;
        }
    }

    return addLayout(input);
}

inline void SceneLayoutsData::reserve(unsigned cap)
{
    _sceneLayouts.reserve(cap);
    _sceneLayoutData.reserve(cap);
}

// Scene
// -----

optional<const SceneData&> CompiledScenesData::findScene(const idstring& name) const
{
    auto it = nameIndexMap.find(name);
    if (it == nameIndexMap.end()) {
        return {};
    }
    return scenes.at(it->second);
}

std::optional<unsigned> CompiledScenesData::indexForScene(const idstring& name) const
{
    auto it = nameIndexMap.find(name);
    if (it == nameIndexMap.end()) {
        return std::nullopt;
    }
    return it->second;
}

static SceneLayerData getLayerSize(const unsigned layerIndex,
                                   const SceneInput& sceneInput, const SceneSettingsInput& sceneSettings,
                                   const Project::ProjectData& projectData, ErrorList& err)
{
    auto addError = [&](const auto&... msg) {
        err.addError(std::make_unique<ListItemError>(&sceneInput, "Scene", sceneInput.name, ", layer ", layerIndex, ": ", msg...));
    };

    const unsigned bitDepth = bitDepthForLayer(sceneSettings.bgMode, layerIndex);
    const idstring& layer = sceneInput.layers.at(layerIndex);

    SceneLayerData out{ 0, 0, 0, false, false };

    switch (sceneSettings.layerTypes.at(layerIndex)) {
    case LayerType::None: {
        if (layer.isValid()) {
            addError("Layer must be blank");
        }
        break;
    }

    case LayerType::BackgroundImage: {
        const auto index = projectData.backgroundImages().indexOf(layer);
        const auto bi = projectData.backgroundImages().at(index);

        if (!bi) {
            addError("Cannot find background image", layer);
            break;
        }

        // ::SHOULDDO add warning if palette conversion colours do not match::

        if (bi->tiles.bitDepthInt() != int(bitDepth)) {
            addError("Invalid bit depth, expected ", bitDepth, " got ", bi->tiles.bitDepthInt());
            break;
        }

        out.layerIndex = *index;
        out.tileSize = bi->tiles.snesDataSize();
        out.nMaps = bi->nTilemaps();
        out.mapHorizontalMirroring = bi->tilemapHorizontalMirroring();
        out.mapVerticalMirroring = bi->tilemapVerticalMirroring();
        break;
    }

    case LayerType::MetaTileTileset: {
        const auto index = projectData.metaTileTilesets().indexOf(layer);
        const auto mt = projectData.metaTileTilesets().at(layer);

        if (!mt) {
            addError("Cannot find MetaTile Tileset", layer);
            break;
        }

        out.layerIndex = *index;
        out.tileSize = mt->animatedTileset.vramTileSize();

        // The tilemap is fixed for MetaTile Tilesets
        out.nMaps = 2;
        out.mapHorizontalMirroring = true;
        out.mapVerticalMirroring = false;
        break;
    }

    case LayerType::TextConsole: {
        if (layer.isValid()) {
            addError("Text Console layer must be blank");
        }

        out.tileSize = 256 * (bitDepth * 8 * 8 / 8);

        // The tilemap is fixed for MetaTile Tilesets
        out.nMaps = 1;
        out.mapHorizontalMirroring = false;
        out.mapVerticalMirroring = false;
        break;
    }
    }

    return out;
}

static SceneData readSceneData(const SceneInput& scene, const ResourceScenes& resourceScenes,
                               const SceneSettingsData& sceneSettingsData, const Project::ProjectData& projectData,
                               ErrorList& err)
{
    SceneData out{};

    const unsigned oldErrorCount = err.errorCount();

    out.valid = true;
    auto addError = [&](const auto&... msg) {
        err.addError(std::make_unique<ListItemError>(&scene, "Scene ", scene.name, ": ", msg...));
        out.valid = false;
    };

    if (scene.name.isValid() == false) {
        addError("Missing name");
    }

    const auto nimIt = sceneSettingsData.nameIndexMap.find(scene.sceneSettings);
    if (nimIt == sceneSettingsData.nameIndexMap.end()) {
        addError("Cannot find scene setting ", scene.sceneSettings);
        return out;
    }
    const SceneSettingsInput& sceneSettings = resourceScenes.settings.at(nimIt->second);
    out.sceneSettings = nimIt->second;

    out.palette = projectData.palettes().indexOf(scene.palette);
    if (!out.palette) {
        addError("Cannot find palette ", scene.palette);
    }

    out.mtTileset = {};
    out.vramUsed = 0;
    for (unsigned layerId = 0; layerId < N_LAYERS; layerId++) {
        auto& layer = out.layers.at(layerId);
        const auto& sceneLayer = sceneSettings.layerTypes.at(layerId);

        layer = getLayerSize(layerId, scene, sceneSettings, projectData, err);
        out.vramUsed += layer.tileSize;
        out.vramUsed += layer.tilemapSize();

        if (sceneLayer == LayerType::MetaTileTileset) {
            out.mtTileset = layer.layerIndex;
        }
    }

    out.valid &= err.errorCount() == oldErrorCount;

    return out;
}

std::shared_ptr<const CompiledScenesData>
compileScenesData(const ResourceScenes& resourceScenes, const Project::ProjectData& projectData,
                  ErrorList& err)

{
    constexpr unsigned SCENE_DATA_ENTRY_SIZE = 7;

    const unsigned oldErrorCount = err.errorCount();

    auto sceneSettingsData = projectData.sceneSettings();
    if (!sceneSettingsData) {
        err.addErrorString("Missing Compiled Scene Settings Data");
        return nullptr;
    }

    auto out = std::make_unique<CompiledScenesData>();
    out->sceneLayouts.reserve(resourceScenes.scenes.size());
    out->scenes.reserve(resourceScenes.scenes.size());
    out->nameIndexMap.reserve(resourceScenes.scenes.size());
    out->sceneSnesData.resize(resourceScenes.scenes.size() * SCENE_DATA_ENTRY_SIZE, 0);

    out->valid = true;
    for (unsigned sceneIndex = 0; sceneIndex < resourceScenes.scenes.size(); sceneIndex++) {
        const SceneInput& scene = resourceScenes.scenes.at(sceneIndex);

        out->scenes.emplace_back(
            readSceneData(scene, resourceScenes, *sceneSettingsData, projectData, err));

        const auto r = out->nameIndexMap.try_emplace(scene.name, sceneIndex);
        if (r.second == false) {
            err.addError(std::make_unique<ListItemError>(&scene, "Duplicate scene name detected: ", scene.name));
            out->valid = false;
        }
    }

    out->valid &= err.errorCount() == oldErrorCount;

    if (out->valid == false) {
        return out;
    }

    // Order the scenes by vram usage in descending order.
    // This will increase the probability of scene layout reuse.
    // Using stable_sort to ensure layout order is the same on different compilers/systems.
    std::vector<unsigned> sortedSceneIndexes(out->scenes.size());
    std::iota(sortedSceneIndexes.begin(), sortedSceneIndexes.end(), 0);
    std::stable_sort(sortedSceneIndexes.begin(), sortedSceneIndexes.end(),
                     [&](const unsigned a, const unsigned b) { return out->scenes.at(a).vramUsed > out->scenes.at(b).vramUsed; });

    if (sortedSceneIndexes.size() >= 2) {
        // Confirm we are sorting in the right direction
        assert(out->scenes.at(sortedSceneIndexes.front()).vramUsed >= out->scenes.at(sortedSceneIndexes.back()).vramUsed);
    }

    for (unsigned sceneIndex : sortedSceneIndexes) {
        auto& scene = out->scenes.at(sceneIndex);
        const auto& sc = scene.layers;

        scene.vramLayout = out->sceneLayouts.findOrAdd({ sc.at(0), sc.at(1), sc.at(2), sc.at(3) });
        if (!scene.vramLayout) {
            const SceneInput& sceneInput = resourceScenes.scenes.at(sceneIndex);
            err.addError(std::make_unique<ListItemError>(&sceneInput, "Scene", sceneInput.name, ": Cannot generate VRAM layout"));
            scene.valid = false;
        }

        if (scene.valid) {
            assert(*scene.sceneSettings < UINT8_MAX);
            assert(*scene.vramLayout < UINT8_MAX);
            assert(*scene.palette < UINT8_MAX);
            for (const auto& l : scene.layers) {
                assert(l.layerIndex < UINT8_MAX);
            }

            const unsigned dataOffset = sceneIndex * SCENE_DATA_ENTRY_SIZE;
            assert(dataOffset + SCENE_DATA_ENTRY_SIZE <= out->sceneSnesData.size());

            const auto sDataStart = out->sceneSnesData.begin() + dataOffset;
            auto sDataIt = sDataStart;

            // Must update CompiledScenesData::SCENE_FORMAT_VERSION if data format changes
            *sDataIt++ = *scene.sceneSettings;   // Scene.settings
            *sDataIt++ = *scene.vramLayout;      // Scene.vramLayout
            *sDataIt++ = *scene.palette;         // Scene.palette
            for (const auto& l : scene.layers) { // Scene.layers
                *sDataIt++ = l.layerIndex;
            }
            assert(sDataIt == sDataStart + SCENE_DATA_ENTRY_SIZE);
        }

        if (scene.valid == false) {
            out->valid = false;
        }
    }

    out->valid &= err.errorCount() == oldErrorCount;

    assert(out->scenes.size() * SCENE_DATA_ENTRY_SIZE == out->sceneSnesData.size());

    return out;
}
const int CompiledScenesData::SCENE_FORMAT_VERSION = 1;
const std::string SceneSettingsData::DATA_LABEL("Project.SceneSettingsData");
const std::string SceneSettingsData::FUNCTION_TABLE_LABEL("Project.SceneSettingsFunctionTable");
const std::string SceneLayoutsData::DATA_LABEL("Project.SceneLayoutData");
const std::string CompiledScenesData::DATA_LABEL("Project.SceneData");

}
}
