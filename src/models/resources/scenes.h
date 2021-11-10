/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace UnTech {
class ErrorList;
class StringStream;
}
namespace UnTech::Project {
class ProjectData;
}

namespace UnTech::Resources {

constexpr unsigned N_LAYERS = 4;
constexpr unsigned MAX_N_SCENE_SETTINGS = 42;

enum class BgMode {
    MODE_0,
    MODE_1,
    MODE_1_BG3_PRIOTITY,
    MODE_2,
    MODE_3,
    MODE_4,
    // Modes 5 and 6 not enabled due to 8/16 pixel tiles
    // Mode 7 may be enabled in the future
};
constexpr unsigned N_BG_MODES = 6;

enum class LayerType {
    None,
    BackgroundImage,
    MetaTileTileset,
    TextConsole,
};
constexpr unsigned N_LAYER_TYPES = 4;

struct SceneSettingsInput {
    idstring name;

    BgMode bgMode;

    std::array<LayerType, N_LAYERS> layerTypes;

    bool operator==(const SceneSettingsInput&) const = default;
};

struct SceneInput {
    idstring name;

    idstring sceneSettings;
    idstring palette;

    std::array<idstring, 4> layers;

    bool operator==(const SceneInput&) const = default;
};

struct ResourceScenes {
    NamedList<SceneSettingsInput> settings;
    NamedList<SceneInput> scenes;

    bool operator==(const ResourceScenes&) const = default;
};

struct SceneSettingsData {
    const static std::string DATA_LABEL;
    const static std::string FUNCTION_TABLE_LABEL;

    std::vector<uint8_t> sceneSettings;
    unsigned nSceneSettings;
};

struct SceneLayerData {
    unsigned layerIndex;

    unsigned tileSize;
    unsigned nMaps;
    bool mapHorizontalMirroring;
    bool mapVerticalMirroring;

    unsigned tilemapSize() const { return nMaps * 32 * 32 * 2; }
};

struct SceneData {
    std::optional<unsigned> sceneSettings;
    std::optional<unsigned> vramLayout;
    std::optional<unsigned> palette;
    std::array<SceneLayerData, N_LAYERS> layers;

    // Only set if scene has a metatile tileset layer
    std::optional<unsigned> mtTileset;

    unsigned vramUsed;

    bool valid;
};

struct SceneLayoutsData {
    constexpr static unsigned TOTAL_VRAM_SIZE = 64 * 1024;
    constexpr static unsigned VRAM_RESERVED_OBJ = 512 * 32; // 512 OBJ tiles are at the end of the VRAM

    constexpr static unsigned BLOCK_SIZE = 2048; // Smallest alignment
    constexpr static unsigned N_VRAM_BLOCKS = (TOTAL_VRAM_SIZE - VRAM_RESERVED_OBJ) / BLOCK_SIZE;
    constexpr static unsigned BLOCKS_PER_MAP = 32 * 32 * 2 / BLOCK_SIZE;
    constexpr static unsigned TILE_ALIGN = 8192 / BLOCK_SIZE;
    constexpr static unsigned MAP_ALIGN = 2048 / BLOCK_SIZE;

    const static std::string DATA_LABEL;

    struct LayerInput {
        uint8_t nTileBlocks;
        uint8_t nMapBlocks;
        uint8_t mapSizeBits; // BGxSC tilemap size bits

        LayerInput(const SceneLayerData& lds);
    };

    struct LayerLayout {
        uint8_t nTileBlocks;
        uint8_t nMapBlocks;
        uint8_t mapSizeBits; // BGxSC tilemap size bits

        uint8_t tileStart;
        uint8_t mapStart;
    };

private:
    std::vector<uint8_t> _sceneLayoutData;
    std::vector<std::array<LayerLayout, N_LAYERS>> _sceneLayouts;

public:
    SceneLayoutsData() = default;

    const std::vector<uint8_t>& sceneLayoutData() const { return _sceneLayoutData; }
    const std::array<LayerLayout, N_LAYERS>& layout(unsigned index) const { return _sceneLayouts.at(index); }

    unsigned nLayouts() const { return _sceneLayouts.size(); }

    std::optional<uint8_t> findOrAdd(const std::array<LayerInput, N_LAYERS>& input);

    void reserve(unsigned cap);

private:
    std::optional<uint8_t> addLayout(const std::array<LayerInput, N_LAYERS>& input);
};

struct CompiledScenesData {
    static const int SCENE_FORMAT_VERSION;
    const static std::string DATA_LABEL;

    SceneSettingsData sceneSettings;
    SceneLayoutsData sceneLayouts;
    std::vector<uint8_t> sceneSnesData;

    std::vector<SceneData> scenes;

    std::unordered_map<idstring, unsigned> nameIndexMap;

    optional<const SceneData&> findScene(const idstring& name) const;

    std::optional<unsigned> indexForScene(const idstring& name) const;
};

void writeSceneIncData(const ResourceScenes& resourceScenes, StringStream& out);

std::shared_ptr<const CompiledScenesData>
compileScenesData(const ResourceScenes& resourceScenes, const Project::ProjectData& projectData, ErrorList& err);

}
