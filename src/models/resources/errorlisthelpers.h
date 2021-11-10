/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "scene-error.h"
#include "scenes.h"

namespace UnTech::Resources {

template <typename... Args>
inline std::unique_ptr<SceneError> sceneSettingsError(const SceneSettingsInput& s, const unsigned index, const Args... msg)
{
    return std::make_unique<SceneError>(SceneErrorType::SCENE_SETTINGS, index,
                                        stringBuilder(u8"Scene Setting ", s.name, u8": ", msg...));
}

template <typename... Args>
inline std::unique_ptr<SceneError> sceneError(const SceneInput& s, const unsigned index, const Args... msg)
{
    return std::make_unique<SceneError>(SceneErrorType::SCENE_LAYER_ERROR, index,
                                        stringBuilder(u8"Scene ", s.name, u8": ", msg...));
}

template <typename... Args>
inline std::unique_ptr<SceneError> sceneLayerError(const SceneInput& s, const unsigned sceneIndex, const unsigned layerIndex, const Args... msg)
{
    return std::make_unique<SceneError>(SceneErrorType::SCENE_LAYER_ERROR, sceneIndex, layerIndex,
                                        stringBuilder(u8"Scene", s.name, u8", layer ", layerIndex, u8": ", msg...));
}

}
