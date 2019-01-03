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

inline std::unique_ptr<MetaSpriteError> frameObjectError(const SpriteImporter::FrameSet& fs, const SpriteImporter::Frame& frame, unsigned objectId, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, fs.frames.getId(&frame), objectId, message);
}

inline std::unique_ptr<MetaSpriteError> frameError(const MetaSprite::FrameSet& fs, const MetaSprite::Frame& frame, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME, fs.frames.getId(&frame), message);
}

inline std::unique_ptr<MetaSpriteError> animationError(const MetaSprite::FrameSet& fs, const Animation::Animation& ani, const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION, fs.animations.getId(&ani), message);
}

inline std::unique_ptr<MetaSpriteError> frameObjectError(const MetaSprite::FrameSet& fs, const MetaSprite::Frame& frame, unsigned objectId,
                                                         const std::string& message)
{
    return std::make_unique<MetaSpriteError>(MsErrorType::FRAME_OBJECT, fs.frames.getId(&frame), objectId, message);
}

}
}
