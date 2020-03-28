/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include "models/common/optional.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace UnTech {
class ErrorList;

namespace Resources {
struct PaletteData;
struct BackgroundImageData;
struct SceneSettingsData;
struct CompiledScenesData;
}
namespace MetaTiles {
struct MetaTileTilesetData;
}
namespace Entity {
struct CompiledEntityRomData;
}

namespace GuiQt {
class ProjectDataSlots;
}
}

namespace UnTech {
namespace Project {
struct ProjectFile;

template <typename T>
class DataStore {
    std::unordered_map<idstring, size_t> _mapping;
    std::vector<std::unique_ptr<const T>> _data;

public:
    inline std::optional<unsigned> indexOf(const idstring& id) const
    {
        auto it = _mapping.find(id);
        if (it != _mapping.end()) {
            return it->second;
        }
        else {
            return std::nullopt;
        }
    }

    inline optional<const T&> at(unsigned index) const
    {
        if (index < _data.size()) {
            return _data.at(index);
        }
        else {
            return {};
        }
    }
    inline optional<const T&> at(std::optional<unsigned> index) const
    {
        if (index) {
            return at(index.value());
        }
        else {
            return {};
        }
    }
    inline optional<const T&> at(const idstring& id) const
    {
        auto it = _mapping.find(id);
        if (it != _mapping.end()) {
            return at(unsigned(it->second));
        }
        else {
            return {};
        }
    }

    size_t size() const { return _data.size(); }

    void clear(const size_t index)
    {
        _data.at(index) = nullptr;
    }

    void removeName(const idstring& name)
    {
        _mapping.erase(name);
    }

    void store(std::unique_ptr<const T>&& data, const size_t index)
    {
        const idstring& name = data->name;

        auto it = _mapping.find(name);
        if (it == _mapping.end()) {
            _mapping.emplace(name, index);
            _data.at(index) = std::move(data);
        }
        else {
            if (it->second != index) {
                throw std::logic_error("idstring/index does not match mapping");
            }
            _data.at(index) = std::move(data);
        }
    }

    void clearAllAndResize(size_t size)
    {
        _mapping.clear();
        _mapping.reserve(size);
        _data.clear();
        _data.resize(size);
    }
};

class ProjectData {
    friend class UnTech::GuiQt::ProjectDataSlots;

    const ProjectFile& _project;

    // NOTE:
    // If I add another DataStore here, I must also update `UnTech::GuiQt::ProjectDataSlots`.

    DataStore<Resources::PaletteData> _palettes;
    DataStore<Resources::BackgroundImageData> _backgroundImages;
    DataStore<MetaTiles::MetaTileTilesetData> _metaTileTilesets;

    std::unique_ptr<const Resources::SceneSettingsData> _sceneSettings;
    std::unique_ptr<const Resources::CompiledScenesData> _scenes;
    std::unique_ptr<const Entity::CompiledEntityRomData> _entityRomData;

public:
    ProjectData(const ProjectFile& project);

    // Must be called when item added/removed from projects
    void resizeDataStores();

    const DataStore<Resources::PaletteData>& palettes() const { return _palettes; }
    const DataStore<Resources::BackgroundImageData>& backgroundImages() const { return _backgroundImages; }
    const DataStore<MetaTiles::MetaTileTilesetData>& metaTileTilesets() const { return _metaTileTilesets; }

    const optional<const Resources::SceneSettingsData&> sceneSettings() const { return _sceneSettings; }
    const optional<const Resources::CompiledScenesData&> scenes() const { return _scenes; }

    const optional<const Entity::CompiledEntityRomData&> entityRomData() const { return _entityRomData; }

    bool compilePalette(size_t index, ErrorList& err);
    bool compileBackgroundImage(size_t index, ErrorList& err);
    bool compileMetaTiles(size_t index, ErrorList& err);

    bool compileSceneSettings(ErrorList& err);
    bool compileScenes(ErrorList& err);

    bool compileEntityRomData(ErrorList& err);
};

}
}
