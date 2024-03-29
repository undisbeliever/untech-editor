/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"

namespace UnTech::Resources {

enum class SceneErrorType {
    SCENE_SETTINGS,
    SCENE_LAYER_ERROR,
};
using SceneError = GenericListError<SceneErrorType>;

}
