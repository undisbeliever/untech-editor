/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "errorlisthelpers.h"
#include "models/common/errorlist.h"

namespace UnTech {
namespace MetaSprite {

MetaSpriteError::MetaSpriteError(MsErrorType type, const void* ptr, std::string name, std::string errorText)
    : _type(type)
    , _ptr(ptr)
    , _name(std::move(name))
    , _id(UINT_MAX)
    , _errorText(std::move(errorText))
{
}

MetaSpriteError::MetaSpriteError(MsErrorType type, const void* ptr, std::string name, unsigned id, std::string errorText)
    : _type(type)
    , _ptr(ptr)
    , _name(std::move(name))
    , _id(id)
    , _errorText(std::move(errorText))
{
}

MetaSpriteError::~MetaSpriteError() = default;

std::string MetaSpriteError::message() const
{
    switch (_type) {
    case MsErrorType::FRAME:
        return "Frame " + _name + ": " + _errorText;

    // Don't show frame in ANIMATION_FRAME error (that could confuse the user)
    case MsErrorType::ANIMATION_FRAME:
    case MsErrorType::ANIMATION:
        return "Animation " + _name + ": " + _errorText;

    case MsErrorType::FRAME_OBJECT:
        return "Frame " + _name + " (object " + std::to_string(_id) + "): " + _errorText;

    case MsErrorType::ACTION_POINT:
        return "Frame " + _name + " (action point " + std::to_string(_id) + "): " + _errorText;

    case MsErrorType::ENTITY_HITBOX:
        return "Frame " + _name + " (entity hitbox " + std::to_string(_id) + "): " + _errorText;
    }

    return "INVALID_ERROR_TYPE";
}

}
}
