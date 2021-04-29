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

class MetaSpriteError : public GenericListError {
public:
    const MsErrorType type;

    MetaSpriteError(const MsErrorType type, unsigned pIndex, std::string&& message)
        : GenericListError(pIndex, std::move(message))
        , type(type)
    {
    }

    MetaSpriteError(const MsErrorType type, unsigned pIndex, unsigned cIndex, std::string&& message)
        : GenericListError(pIndex, cIndex, std::move(message))
        , type(type)
    {
    }
};

enum class EoErrorType {
    STILL_FRAMES,
    STILL_FRAMES_ALT,
    ANIMATIONS,
    ANIMATIONS_ALT,
};

class ExportOrderError : public GenericListError {
public:
    const EoErrorType type;

    ExportOrderError(const EoErrorType type, unsigned pIndex, std::string&& message)
        : GenericListError(pIndex, std::move(message))
        , type(type)
    {
    }

    ExportOrderError(const EoErrorType type, unsigned pIndex, unsigned cIndex, std::string&& message)
        : GenericListError(pIndex, cIndex, std::move(message))
        , type(type)
    {
    }
};

enum class ApfErrorType {
    ACTION_POINT_FUNCTIONS,
};

class ActionPointFunctionError : public GenericListError {
public:
    const ApfErrorType type;

    ActionPointFunctionError(const ApfErrorType type, unsigned pIndex, std::string&& message)
        : GenericListError(pIndex, std::move(message))
        , type(type)
    {
    }
};

}
