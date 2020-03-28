/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-data.h"
#include "project.h"
#include "models/common/errorlist.h"
#include <cassert>

namespace UnTech {
namespace Project {

template <typename ConvertFunction, class DataT, class InputT, typename... PreresquitesT>
static bool compileData(ConvertFunction convertFunction, DataStore<DataT>& dataStore,
                        const size_t index, const InputT& input, ErrorList& err,
                        const PreresquitesT&... preresquites)
{
    // Ensure dataStore is not a preresquite.
    assert(((static_cast<const void*>(&preresquites) != static_cast<const void*>(&dataStore)) && ...));

    std::unique_ptr<const DataT> data = convertFunction(input, preresquites..., err);

    bool valid = (data != nullptr);

    if (data) {
        valid &= data->validate(err);
    }

    if (valid) {
        dataStore.store(std::move(data), index);
    }
    else {
        dataStore.clear(index);
    }

    return valid;
}

template <typename ConvertFunction, class DataT, class InputListT, typename... PreresquitesT>
static bool compileListItem(ConvertFunction convertFunction, DataStore<DataT>& dataStore,
                            const InputListT& inputList, const size_t index, ErrorList& err,
                            const PreresquitesT&... preresquites)
{
    auto input = inputList.at(index);
    return compileData(convertFunction, dataStore, index, input, err, preresquites...);
}

template <typename ConvertFunction, class DataT, class InputT, typename... PreresquitesT>
static bool compileExternalFileListItem(ConvertFunction convertFunction, DataStore<DataT>& dataStore,
                                        const ExternalFileList<InputT>& inputList, unsigned index, ErrorList& err,
                                        const PreresquitesT&... preresquites)
{
    const auto* input = inputList.at(index);

    if (input) {
        return compileData(convertFunction, dataStore, index, *input, err, preresquites...);
    }
    else {
        err.addErrorString("Unexpected nullptr, maybe the item wasn't loaded");

        dataStore.clear(index);
        return false;
    }
}

ProjectData::ProjectData(const ProjectFile& project)
    : _project(project)
{
    _palettes.clearAllAndResize(_project.palettes.size());
    _backgroundImages.clearAllAndResize(_project.backgroundImages.size());
    _metaTileTilesets.clearAllAndResize(_project.metaTileTilesets.size());
}

bool ProjectData::compilePalette(size_t index, ErrorList& err)
{
    return compileListItem(Resources::convertPalette, _palettes, _project.palettes, index, err);
}

bool ProjectData::compileBackgroundImage(size_t index, ErrorList& err)
{
    return compileListItem(Resources::convertBackgroundImage, _backgroundImages, _project.backgroundImages, index, err, _palettes);
}

bool ProjectData::compileMetaTiles(size_t index, ErrorList& err)
{
    return compileExternalFileListItem(MetaTiles::convertTileset, _metaTileTilesets, _project.metaTileTilesets, index, err, _palettes);
}

}
}
