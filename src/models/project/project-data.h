/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include "models/common/mutex_wrapper.h"
#include <gsl/pointers>
#include <memory>
#include <unordered_map>
#include <vector>

namespace UnTech::MetaSprite {
using ActionPointMapping = std::unordered_map<idstring, uint8_t>;
}
namespace UnTech::MetaSprite::Compiler {
struct FrameSetData;
}

namespace UnTech {
class ErrorList;

template <typename T>
class ExternalFileList;
}

namespace UnTech::Scripting {
struct GameStateData;
struct BytecodeMapping;
}

namespace UnTech::Resources {
struct PaletteData;
struct BackgroundImageData;
struct SceneSettingsData;
struct CompiledScenesData;
}

namespace UnTech::MetaTiles {
struct MetaTileTilesetData;
struct InteractiveTilesData;
}

namespace UnTech::Entity {
struct CompiledEntityRomData;
}

namespace UnTech::Rooms {
struct RoomData;
}

namespace UnTech::Project {

struct ProjectFile;

// This class is thread safe
template <typename T>
class DataStore final {
    struct Data {
        std::unordered_map<idstring, size_t> mapping;
        std::vector<std::pair<idstring, std::shared_ptr<const T>>> data;
    };
    shared_mutex<Data> data;

public:
    void clearAllAndResize(size_t size);

    // Returns true if name is unique
    [[nodiscard]] bool store(const size_t index, const idstring& name, std::shared_ptr<const T>&& res_data);

    [[nodiscard]] size_t size() const;

    // May return nullptr
    [[nodiscard]] std::optional<std::pair<size_t, gsl::not_null<std::shared_ptr<const T>>>> indexAndDataFor(const idstring& id) const;
    [[nodiscard]] std::shared_ptr<const T> at(unsigned index) const;
};

// This class is thread safe
class ProjectSettingsData {
private:
    struct Data {
        std::shared_ptr<const Scripting::GameStateData> gameState;
        std::shared_ptr<const Scripting::BytecodeMapping> bytecode;
        std::shared_ptr<const MetaSprite::ActionPointMapping> actionPointMapping;
        std::shared_ptr<const MetaTiles::InteractiveTilesData> interactiveTiles;
        std::shared_ptr<const Resources::CompiledScenesData> scenes;
        std::shared_ptr<const Entity::CompiledEntityRomData> entityRomData;
    };
    shared_mutex<Data> data;

public:
    ProjectSettingsData() = default;

    void store(std::shared_ptr<const Scripting::GameStateData>&& gameState);
    void store(std::shared_ptr<const Scripting::BytecodeMapping>&& bytecode);
    void store(std::shared_ptr<const MetaSprite::ActionPointMapping>&& actionPointMapping);
    void store(std::shared_ptr<const MetaTiles::InteractiveTilesData>&& interactiveTiles);
    void store(std::shared_ptr<const Resources::CompiledScenesData>&& scenes);
    void store(std::shared_ptr<const Entity::CompiledEntityRomData>&& entityRomData);

    [[nodiscard]] std::shared_ptr<const Scripting::GameStateData> gameState() const;
    [[nodiscard]] std::shared_ptr<const Scripting::BytecodeMapping> bytecodeData() const;
    [[nodiscard]] std::shared_ptr<const MetaSprite::ActionPointMapping> actionPointMapping() const;
    [[nodiscard]] std::shared_ptr<const MetaTiles::InteractiveTilesData> interactiveTiles() const;
    [[nodiscard]] std::shared_ptr<const Resources::CompiledScenesData> scenes() const;
    [[nodiscard]] std::shared_ptr<const Entity::CompiledEntityRomData> entityRomData() const;
};

// This struct is thread safe
struct ProjectData {
    // ALL FIELDS in this class MUST BE thread safe

    ProjectSettingsData projectSettingsData;

    DataStore<UnTech::MetaSprite::Compiler::FrameSetData> frameSets;
    DataStore<Resources::PaletteData> palettes;
    DataStore<Resources::BackgroundImageData> backgroundImages;
    DataStore<MetaTiles::MetaTileTilesetData> metaTileTilesets;
    DataStore<Rooms::RoomData> rooms;
};

}
