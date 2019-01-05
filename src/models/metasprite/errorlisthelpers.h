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

inline std::unique_ptr<MetaSpriteError> frameError(const SpriteImporter::FrameSet& fs, const SpriteImporter::Frame& frame, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, fs.frames.getId(&frame), message);
}

inline std::unique_ptr<MetaSpriteError> animationError(const SpriteImporter::FrameSet& fs, const Animation::Animation& ani, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION, fs.animations.getId(&ani), message);
}

inline std::unique_ptr<MetaSpriteError> animationFrameError(const SpriteImporter::FrameSet& fs, const Animation::Animation& ani, unsigned index, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION_FRAME, fs.animations.getId(&ani), index, message);
}

inline std::unique_ptr<MetaSpriteError> frameObjectError(const SpriteImporter::FrameSet& fs, const SpriteImporter::Frame& frame, unsigned objectId, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, fs.frames.getId(&frame), objectId, message);
}

inline std::unique_ptr<MetaSpriteError> actionPointError(const SpriteImporter::FrameSet& fs, const SpriteImporter::Frame& frame, unsigned apId,
                                                         const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, fs.frames.getId(&frame), apId, message);
}

inline std::unique_ptr<MetaSpriteError> entityHitboxError(const SpriteImporter::FrameSet& fs, const SpriteImporter::Frame& frame, unsigned ehId,
                                                          const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ENTITY_HITBOX, fs.frames.getId(&frame), ehId, message);
}

inline std::unique_ptr<MetaSpriteError> frameError(const MetaSprite::FrameSet& fs, const MetaSprite::Frame& frame, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, fs.frames.getId(&frame), message);
}

inline std::unique_ptr<MetaSpriteError> animationError(const MetaSprite::FrameSet& fs, const Animation::Animation& ani, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION, fs.animations.getId(&ani), message);
}

inline std::unique_ptr<MetaSpriteError> animationFrameError(const MetaSprite::FrameSet& fs, const Animation::Animation& ani, unsigned index, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION_FRAME, fs.animations.getId(&ani), index, message);
}

inline std::unique_ptr<MetaSpriteError> frameObjectError(const MetaSprite::FrameSet& fs, const MetaSprite::Frame& frame, unsigned objectId,
                                                         const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, fs.frames.getId(&frame), objectId, message);
}

inline std::unique_ptr<MetaSpriteError> actionPointError(const MetaSprite::FrameSet& fs, const MetaSprite::Frame& frame, unsigned apId,
                                                         const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ACTION_POINT, fs.frames.getId(&frame), apId, message);
}

inline std::unique_ptr<MetaSpriteError> entityHitboxError(const MetaSprite::FrameSet& fs, const MetaSprite::Frame& frame, unsigned ehId,
                                                          const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ENTITY_HITBOX, fs.frames.getId(&frame), ehId, message);
}

}
}
