/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"

namespace UnTech::MetaSprite {

enum class MsErrorType {
    FRAME,
    ANIMATION,
    ANIMATION_FRAME,
    FRAME_OBJECT,
    ACTION_POINT,

    TILE_HITBOX,
    SHIELD,
    HIT_BOX,
    HURT_BOX,
};
using MetaSpriteError = GenericListError<MsErrorType>;

enum class EoErrorType {
    STILL_FRAMES,
    STILL_FRAMES_ALT,
    ANIMATIONS,
    ANIMATIONS_ALT,
};
using ExportOrderError = GenericListError<EoErrorType>;

enum class ApfErrorType {
    ACTION_POINT_FUNCTIONS,
};
using ActionPointFunctionError = GenericListError<ApfErrorType>;

}
