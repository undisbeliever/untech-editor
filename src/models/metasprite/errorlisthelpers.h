/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metasprite-error.h"
#include "metasprite.h"
#include "spriteimporter.h"

namespace UnTech::MetaSprite {

// ::TODO improve error message if the frame has no name::

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameError(const SpriteImporter::Frame& frame, const unsigned frameIndex, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, frameIndex,
                                             stringBuilder("SI Frame ", frame.name, ": ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameObjectError(const SpriteImporter::Frame& frame, const unsigned frameIndex, const unsigned objectId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, frameIndex, objectId,
                                             stringBuilder("SI Frame ", frame.name, " (object ", objectId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> actionPointError(const SpriteImporter::Frame& frame, const unsigned frameIndex, const unsigned apId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, frameIndex, apId,
                                             stringBuilder("SI Frame ", frame.name, " (action point ", apId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> entityHitboxError(const SpriteImporter::Frame& frame, const unsigned frameIndex, const unsigned ehId,
                                                          const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ENTITY_HITBOX, frameIndex, ehId,
                                             stringBuilder("SI Frame ", frame.name, " (entity hitbox ", ehId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameError(const MetaSprite::Frame& frame, const unsigned frameIndex, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, frameIndex,
                                             stringBuilder("MS Frame ", frame.name, ": ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameObjectError(const MetaSprite::Frame& frame, const unsigned frameIndex, const unsigned objectId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, frameIndex, objectId,
                                             stringBuilder("MS Frame ", frame.name, " (object ", objectId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> actionPointError(const MetaSprite::Frame& frame, const unsigned frameIndex, const unsigned apId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, frameIndex, apId,
                                             stringBuilder("MS Frame ", frame.name, " (action point ", apId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> entityHitboxError(const MetaSprite::Frame& frame, const unsigned frameIndex, const unsigned ehId,
                                                          const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ENTITY_HITBOX, frameIndex, ehId,
                                             stringBuilder("MS Frame ", frame.name, " (entity hitbox ", ehId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> animationError(const Animation::Animation& ani, const unsigned aniIndex, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION, aniIndex,
                                             stringBuilder("Animation ", ani.name, message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> animationFrameError(const Animation::Animation& ani, const unsigned aniIndex, const unsigned frameIndex, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION_FRAME, aniIndex, frameIndex,
                                             stringBuilder("Animation ", ani.name, message...));
}

}
