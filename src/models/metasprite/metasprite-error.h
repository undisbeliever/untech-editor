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
    FRAME_OBJECT
};

class MetaSpriteError : public AbstractSpecializedError {
private:
    const MsErrorType _type;
    const std::string _name;
    const unsigned _id;
    const std::string _errorText;

public:
    MetaSpriteError(MsErrorType type, std::string name, std::string errorText);
    MetaSpriteError(MsErrorType type, std::string name, unsigned id, std::string errorText);
    virtual ~MetaSpriteError() final;

    MsErrorType type() const { return _type; }
    const std::string& name() const { return _name; }
    unsigned id() const { return _id; }

    virtual std::string message() const final;
};

}
}
