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
                                             stringBuilder(u8"SI Frame ", frame.name, u8": ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> collisionBoxError(const SpriteImporter::Frame& frame, const unsigned frameIndex, const MsErrorType type, const Args... message)
{
    return std::make_unique<MetaSpriteError>(type, frameIndex,
                                             stringBuilder(u8"SI Frame ", frame.name, u8": ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameObjectError(const SpriteImporter::Frame& frame, const unsigned frameIndex, const unsigned objectId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, frameIndex, objectId,
                                             stringBuilder(u8"SI Frame ", frame.name, u8" (object ", objectId, u8"): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> actionPointError(const SpriteImporter::Frame& frame, const unsigned frameIndex, const unsigned apId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, frameIndex, apId,
                                             stringBuilder(u8"SI Frame ", frame.name, u8" (action point ", apId, u8"): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameError(const MetaSprite::Frame& frame, const unsigned frameIndex, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, frameIndex,
                                             stringBuilder(u8"MS Frame ", frame.name, u8": ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> collisionBoxError(const MetaSprite::Frame& frame, const unsigned frameIndex, const MsErrorType type, const Args... message)
{
    return std::make_unique<MetaSpriteError>(type, frameIndex,
                                             stringBuilder(u8"MS Frame ", frame.name, u8": ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameObjectError(const MetaSprite::Frame& frame, const unsigned frameIndex, const unsigned objectId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, frameIndex, objectId,
                                             stringBuilder(u8"MS Frame ", frame.name, u8" (object ", objectId, u8"): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> actionPointError(const MetaSprite::Frame& frame, const unsigned frameIndex, const unsigned apId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, frameIndex, apId,
                                             stringBuilder(u8"MS Frame ", frame.name, u8" (action point ", apId, u8"): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> animationError(const Animation::Animation& ani, const unsigned aniIndex, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION, aniIndex,
                                             stringBuilder(u8"Animation ", ani.name, message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> animationFrameError(const Animation::Animation& ani, const unsigned aniIndex, const unsigned frameIndex, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION_FRAME, aniIndex, frameIndex,
                                             stringBuilder(u8"Animation ", ani.name, message...));
}

}
