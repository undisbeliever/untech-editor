/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"

namespace UnTech {
namespace MetaSprite {

enum class MsErrorType {
    FRAME,
    ANIMATION,
    ANIMATION_FRAME,
    FRAME_OBJECT,
    ACTION_POINT,
    ENTITY_HITBOX,
};

class MetaSpriteError : public AbstractSpecializedError {
private:
    const MsErrorType _type;
    const void* const _ptr;
    const std::string _name;
    const unsigned _id;
    const std::string _errorText;

public:
    MetaSpriteError(MsErrorType type, const void* ptr, std::string name, std::string errorText);
    MetaSpriteError(MsErrorType type, const void* ptr, std::string name, unsigned id, std::string errorText);
    virtual ~MetaSpriteError() final;

    MsErrorType type() const { return _type; }
    const void* ptr() const { return _ptr; }
    unsigned id() const { return _id; }

    virtual std::string message() const final;
};

}
}