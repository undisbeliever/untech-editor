/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "scenes.h"
#include "background-image.h"
#include "errorlisthelpers.h"
#include "scene-bgmode.hpp"
#include "models/common/exceptions.h"
#include "models/common/iterators.h"
#include "models/common/stringstream.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/project/project-data.h"
#include "models/snes/bit-depth.h"
#include <numeric>

namespace UnTech::Resources {

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

static bool validate(const SceneSettingsInput& input, const unsigned index, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        err.addError(sceneSettingsError(input, index, msg...));
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError(u8"Missing name");
    }

    for (const auto l : range(numberOfLayers(input.bgMode), N_LAYERS)) {
        if (input.layerTypes.at(l) != LayerType::None) {
            const unsigned bgModeInt = bgModeByte(input.bgMode) & 7;
            addError(u8"Layer ", l, u8" must be empty in BG Mode ", bgModeInt);
        }
    }

    std::array<unsigned, N_LAYER_TYPES> layerTypeCount;
    layerTypeCount.fill(0);

    for (const LayerType& lt : input.layerTypes) {
        unsigned i = unsigned(lt);
        layerTypeCount.at(i)++;
    }

    if (layerTypeCount.at(unsigned(LayerType::TextConsole)) > 1) {
        addError(u8"Invalid setting ", input.name, u8": Cannot have more than one text layer");
    }

    unsigned nAnimatedLayers = layerTypeCount.at(unsigned(LayerType::BackgroundImage));
    if (nAnimatedLayers > 1) {
        addError(u8"Invalid setting ", input.name, u8": Cannot have more than one animated layer (MetaTiles are animated)");
    }

    return valid;
}

void writeSceneIncData(const ResourceScenes& resourceScenes, StringStream& out)
{
    out.write(u8"code()\n",
              SceneSettingsData::FUNCTION_TABLE_LABEL, u8":\n");

    for (const SceneSettingsInput& ssi : resourceScenes.settings) {
        out.write(u8"\tdw\tScenes.", ssi.name, u8".SetupPpu_dp2100, Scenes.", ssi.name, u8".Process, Scenes.", ssi.name, u8".VBlank_dp2100\n");
    }
    out.write(u8"constant Project.SceneSettingsFunctionTable.size = pc() - Project.SceneSettingsFunctionTable\n"
              u8"\n\n"

              u8"namespace Project.Scenes {\n");

    unsigned i = 0;
    for (const SceneInput& si : resourceScenes.scenes) {
        out.write(u8"\tconstant ", si.name, u8" = ", i++, u8"\n");
    }
    assert(i == resourceScenes.scenes.size());
    out.write(u8"}\n\n");
}
constexpr unsigned FUNCTION_TABLE_ELEMENT_SIZE = 6;
constexpr unsigned SCENE_SETTINGS_DATA_ELEMENT_SIZE = 3;
static_assert(MAX_N_SCENE_SETTINGS == 255 / std::max(FUNCTION_TABLE_ELEMENT_SIZE, SCENE_SETTINGS_DATA_ELEMENT_SIZE));

using NameIndexMap = std::unordered_map<idstring, unsigned>;

static std::optional<NameIndexMap> sceneSettingsIndexMap(const NamedList<SceneSettingsInput>& settings, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const SceneSettingsInput& ssi, const unsigned index, const auto&... msg) {
        err.addError(sceneSettingsError(ssi, index, msg...));
        valid = false;
    };

    NameIndexMap nameIndexMap;

    for (auto [id, ssi] : const_enumerate(settings)) {
        if (ssi.name.isValid()) {
            const auto r = nameIndexMap.try_emplace(ssi.name, id);
            if (r.second == false) {
                addError(ssi, id, u8"Duplicate Scene Settings name:", ssi.name);
            }
        }
    }

    if (!valid) {
        return std::nullopt;
    }

    return nameIndexMap;
}

static SceneSettingsData
compileSceneSettingsData(const NamedList<SceneSettingsInput>& settings, ErrorList& err)
{
    auto addError = [&](const SceneSettingsInput& ssi, const unsigned index, const auto&... msg) {
        err.addError(sceneSettingsError(ssi, index, msg...));
    };

    if (settings.size() > MAX_N_SCENE_SETTINGS) {
        err.addErrorString(u8"Too many settings (", settings.size(), u8"max = ", MAX_N_SCENE_SETTINGS);
    }

    SceneSettingsData out;
    out.sceneSettings.resize(settings.size() * SCENE_SETTINGS_DATA_ELEMENT_SIZE);
    out.nSceneSettings = settings.size();

    auto ssDataIt = out.sceneSettings.begin();
    for (auto [ssIndex, ssi] : const_enumerate(settings)) {
        const uint8_t bgMode = bgModeByte(ssi.bgMode);
        if (bgMode >= 0xff) {
            addError(ssi, ssIndex, u8"Invalid bgMode");
        }

        unsigned layerTypes = 0;
        for (const auto i : range(N_LAYERS)) {
            const auto& lt = ssi.layerTypes.at(i);
            layerTypes |= (unsigned(lt) & 0x7) << (i * 4 + 1);
        }
        if (layerTypes >= 0xffff) {
            addError(ssi, ssIndex, u8"Invalid layerTypes");
        }

        // Must update CompiledScenesData::SCENE_FORMAT_VERSION if data format changes
        *ssDataIt++ = bgMode;
        *ssDataIt++ = layerTypes & 0xff;
        *ssDataIt++ = layerTypes >> 8;
    }
    assert(ssDataIt == out.sceneSettings.end());

    return out;
}

// SceneLayoutData
// ---------------

// thows `logic_error` if layout is not valid
static void confirmLayoutIsValid(const std::array<SceneLayoutsData::LayerLayout, N_LAYERS>& layers)
{
    std::array<bool, SceneLayoutsData::N_VRAM_BLOCKS> usedBlocks;
    usedBlocks.fill(false);

    auto testBlock = [&](const unsigned start, const unsigned size) {
        for (const auto i : range(start, start + size)) {
            if (usedBlocks.at(i)) {
                throw logic_error(u8"Layout invalid");
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
            throw runtime_error(u8"nBlocks is too large");
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
            throw runtime_error(u8"nBlocks is too large");
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

    for (const auto layerId : range(N_LAYERS)) {
        auto b = findFreeSpaceForwards(input.at(layerId).nTileBlocks, TILE_ALIGN);
        if (b == std::nullopt) {
            return std::nullopt;
        }
        layerBases.at(layerId).tiles = *b;
    }

    // ::TODO sort layers by size of maps (insertation sort)::

    for (const auto layerId : range(N_LAYERS)) {
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

    for (const auto layerId : range(N_LAYERS)) {
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
        throw runtime_error(u8"Too many scene layouts");
    }
    const uint8_t layoutId = _sceneLayouts.size();

    _sceneLayouts.emplace_back(layout);
    _sceneLayoutData.insert(_sceneLayoutData.end(), layoutData.begin(), layoutData.end());

    return layoutId;
}

inline std::optional<uint8_t> SceneLayoutsData::findOrAdd(const std::array<SceneLayoutsData::LayerInput, N_LAYERS>& input)
{
    for (const auto [layoutId, layout] : const_enumerate(_sceneLayouts)) {
        bool match = true;
        for (const auto i : range(N_LAYERS)) {
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

optional_ref<const SceneData&> CompiledScenesData::findScene(const idstring& name) const
{
    auto it = nameIndexMap.find(name);
    if (it == nameIndexMap.end()) {
        return std::nullopt;
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
                                   const SceneInput& sceneInput, const unsigned sceneIndex,
                                   const SceneSettingsInput& sceneSettings,
                                   const Project::ProjectData& projectData, ErrorList& err)
{
    auto addError = [&](const auto&... msg) {
        err.addError(sceneLayerError(sceneInput, sceneIndex, layerIndex, msg...));
    };

    const auto bitDepth = bitDepthForLayer(sceneSettings.bgMode, layerIndex);
    const idstring& layer = sceneInput.layers.at(layerIndex);

    // Used for printing error messages
    const unsigned bitDepthUint = bitDepth ? unsigned(*bitDepth) : 0;

    SceneLayerData out{ 0, 0, 0, false, false };

    switch (sceneSettings.layerTypes.at(layerIndex)) {
    case LayerType::None: {
        if (layer.isValid()) {
            addError(u8"Layer must be blank");
        }
        break;
    }

    case LayerType::BackgroundImage: {
        const auto index = projectData.backgroundImages().indexOf(layer);
        const auto bi = projectData.backgroundImages().at(index);

        if (!bi) {
            addError(u8"Cannot find background image", layer);
            break;
        }

        // ::SHOULDDO add warning if palette conversion colours do not match::

        if (bi->bitDepth != bitDepth) {
            addError(u8"Invalid bit depth, expected ", bitDepthUint, u8" got ", unsigned(bi->bitDepth));
            break;
        }

        out.layerIndex = *index;
        out.tileSize = bi->tilesetDataSize();
        out.nMaps = bi->nTilemaps();
        out.mapHorizontalMirroring = bi->tilemapHorizontalMirroring();
        out.mapVerticalMirroring = bi->tilemapVerticalMirroring();
        break;
    }

    case LayerType::MetaTileTileset: {
        const auto index = projectData.metaTileTilesets().indexOf(layer);
        const auto mt = projectData.metaTileTilesets().at(layer);

        if (!mt) {
            addError(u8"Cannot find MetaTile Tileset", layer);
            break;
        }

        if (mt->animatedTileset.bitDepth != bitDepth) {
            addError(u8"Invalid bit depth, expected ", bitDepthUint, u8" got ", unsigned(mt->animatedTileset.bitDepth));
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
            addError(u8"Text Console layer must be blank");
        }

        out.tileSize = 256 * Snes::snesTileSizeForBitdepth(bitDepth.value());

        // The tilemap is fixed for MetaTile Tilesets
        out.nMaps = 1;
        out.mapHorizontalMirroring = false;
        out.mapVerticalMirroring = false;
        break;
    }
    }

    return out;
}

static SceneData readSceneData(const SceneInput& scene, const unsigned sceneIndex, const ResourceScenes& resourceScenes,
                               const NameIndexMap& sceneSettingsMap, const Project::ProjectData& projectData,
                               ErrorList& err)
{
    SceneData out{};

    const unsigned oldErrorCount = err.errorCount();

    out.valid = true;
    auto addError = [&](const auto&... msg) {
        err.addError(sceneError(scene, sceneIndex, msg...));
        out.valid = false;
    };

    if (scene.name.isValid() == false) {
        addError(u8"Missing name");
    }

    const auto nimIt = sceneSettingsMap.find(scene.sceneSettings);
    if (nimIt == sceneSettingsMap.end()) {
        addError(u8"Cannot find scene setting ", scene.sceneSettings);
        return out;
    }
    const SceneSettingsInput& sceneSettings = resourceScenes.settings.at(nimIt->second);
    out.sceneSettings = nimIt->second;

    out.palette = projectData.palettes().indexOf(scene.palette);
    if (!out.palette) {
        addError(u8"Cannot find palette ", scene.palette);
    }

    out.mtTileset = {};
    out.vramUsed = 0;
    for (const auto layerId : range(N_LAYERS)) {
        auto& layer = out.layers.at(layerId);
        const auto& sceneLayer = sceneSettings.layerTypes.at(layerId);

        layer = getLayerSize(layerId, scene, sceneIndex, sceneSettings, projectData, err);
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
    using LayerInput = SceneLayoutsData::LayerInput;

    constexpr unsigned SCENE_DATA_ENTRY_SIZE = 7;

    const unsigned oldErrorCount = err.errorCount();

    bool valid = true;

    for (const auto [i, ssi] : enumerate(resourceScenes.settings)) {
        valid &= validate(ssi, i, err);
    }

    const auto sceneSettingsMap = sceneSettingsIndexMap(resourceScenes.settings, err);
    valid &= sceneSettingsMap.has_value();

    if (valid == false) {
        return nullptr;
    }

    auto out = std::make_shared<CompiledScenesData>();

    out->sceneSettings = compileSceneSettingsData(resourceScenes.settings, err);
    out->sceneLayouts.reserve(resourceScenes.scenes.size());
    out->scenes.reserve(resourceScenes.scenes.size());
    out->nameIndexMap.reserve(resourceScenes.scenes.size());
    out->sceneSnesData.resize(resourceScenes.scenes.size() * SCENE_DATA_ENTRY_SIZE, 0);

    for (auto [sceneIndex, scene] : const_enumerate(resourceScenes.scenes)) {
        out->scenes.emplace_back(
            readSceneData(scene, sceneIndex, resourceScenes, *sceneSettingsMap, projectData, err));

        const auto r = out->nameIndexMap.try_emplace(scene.name, sceneIndex);
        if (r.second == false) {
            err.addError(sceneError(scene, sceneIndex, u8"Duplicate scene name detected"));
            valid = false;
        }
    }

    valid &= err.errorCount() == oldErrorCount;

    if (valid == false) {
        return nullptr;
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

        scene.vramLayout = out->sceneLayouts.findOrAdd({ LayerInput{ sc.at(0) }, LayerInput{ sc.at(1) },
                                                         LayerInput{ sc.at(2) }, LayerInput{ sc.at(3) } });

        if (!scene.vramLayout) {
            const SceneInput& sceneInput = resourceScenes.scenes.at(sceneIndex);
            err.addError(sceneError(sceneInput, sceneIndex, u8"Cannot generate VRAM layout"));
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
            valid = false;
        }
    }

    valid &= err.errorCount() == oldErrorCount;

    assert(out->scenes.size() * SCENE_DATA_ENTRY_SIZE == out->sceneSnesData.size());

    return out;
}
const int CompiledScenesData::SCENE_FORMAT_VERSION = 1;
const std::u8string SceneSettingsData::DATA_LABEL(u8"Project.SceneSettingsData");
const std::u8string SceneSettingsData::FUNCTION_TABLE_LABEL(u8"Project.SceneSettingsFunctionTable");
const std::u8string SceneLayoutsData::DATA_LABEL(u8"Project.SceneLayoutData");
const std::u8string CompiledScenesData::DATA_LABEL(u8"Project.SceneData");

}
