/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metasprite-error.h"
#include "metasprite.h"
#include "spriteimporter.h"

namespace UnTech {
namespace MetaSprite {

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameError(const SpriteImporter::Frame& frame, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, &frame, frame.name,
                                             stringBuilder("SI Frame ", frame.name, ": ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameObjectError(const SpriteImporter::Frame& frame, unsigned objectId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, &frame, frame.name, objectId,
                                             stringBuilder("SI Frame ", frame.name, " (object ", objectId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> actionPointError(const SpriteImporter::Frame& frame, unsigned apId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, &frame, frame.name, apId,
                                             stringBuilder("SI Frame ", frame.name, " (action point ", apId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> entityHitboxError(const SpriteImporter::Frame& frame, unsigned ehId,
                                                          const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ENTITY_HITBOX, &frame, frame.name, ehId,
                                             stringBuilder("SI Frame ", frame.name, " (entity hitbox ", ehId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameError(const MetaSprite::Frame& frame, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, &frame, frame.name,
                                             stringBuilder("MS Frame ", frame.name, ": ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> frameObjectError(const MetaSprite::Frame& frame, unsigned objectId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, &frame, frame.name, objectId,
                                             stringBuilder("MS Frame ", frame.name, " (object ", objectId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> actionPointError(const MetaSprite::Frame& frame, unsigned apId,
                                                         const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, &frame, frame.name, apId,
                                             stringBuilder("MS Frame ", frame.name, " (action point ", apId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> entityHitboxError(const MetaSprite::Frame& frame, unsigned ehId,
                                                          const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ENTITY_HITBOX, &frame, frame.name, ehId,
                                             stringBuilder("MS Frame ", frame.name, " (entity hitbox ", ehId, "): ", message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> animationError(const Animation::Animation& ani, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION, &ani, ani.name,
                                             stringBuilder("Animation ", ani.name, message...));
}

template <typename... Args>
inline std::unique_ptr<MetaSpriteError> animationFrameError(const Animation::Animation& ani, unsigned index, const Args... message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION_FRAME, &ani, ani.name, index,
                                             stringBuilder("Animation ", ani.name, message...));
}

}
}
