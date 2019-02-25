/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metasprite-error.h"
#include "metasprite.h"
#include "spriteimporter.h"

namespace UnTech {
namespace MetaSprite {

inline std::unique_ptr<MetaSpriteError> frameError(const SpriteImporter::Frame& frame, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, frame.name, message);
}

inline std::unique_ptr<MetaSpriteError> animationError(const Animation::Animation& ani, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION, ani.name, message);
}

inline std::unique_ptr<MetaSpriteError> frameObjectError(const SpriteImporter::Frame& frame, unsigned objectId, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, frame.name, objectId, message);
}

inline std::unique_ptr<MetaSpriteError> actionPointError(const SpriteImporter::Frame& frame, unsigned apId,
                                                         const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, frame.name, apId, message);
}

inline std::unique_ptr<MetaSpriteError> entityHitboxError(const SpriteImporter::Frame& frame, unsigned ehId,
                                                          const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ENTITY_HITBOX, frame.name, ehId, message);
}

inline std::unique_ptr<MetaSpriteError> frameError(const MetaSprite::Frame& frame, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, frame.name, message);
}

inline std::unique_ptr<MetaSpriteError> animationFrameError(const Animation::Animation& ani, unsigned index, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION_FRAME, ani.name, index, message);
}

inline std::unique_ptr<MetaSpriteError> frameObjectError(const MetaSprite::Frame& frame, unsigned objectId,
                                                         const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, frame.name, objectId, message);
}

inline std::unique_ptr<MetaSpriteError> actionPointError(const MetaSprite::Frame& frame, unsigned apId,
                                                         const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, frame.name, apId, message);
}

inline std::unique_ptr<MetaSpriteError> entityHitboxError(const MetaSprite::Frame& frame, unsigned ehId,
                                                          const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ENTITY_HITBOX, frame.name, ehId, message);
}

}
}
