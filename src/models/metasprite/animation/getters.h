/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animation.h"
#include "models/common/aabb.h"

namespace UnTech::MetaSprite::Animation {

inline optional<const Animation&> getAnimation(const NamedList<Animation>& animations, const size_t index)
{
    if (index < animations.size()) {
        return animations.at(index);
    }
    return std::nullopt;
}

inline std::pair<optional<const Animation&>, optional<const AnimationFrame&>>
getAnimationAndFrame(const NamedList<Animation>& animations, const size_t aniIndex, const size_t frameIndex)
{
    if (aniIndex < animations.size()) {
        const auto& ani = animations.at(aniIndex);

        if (frameIndex < ani.frames.size()) {
            return { ani, ani.frames.at(frameIndex) };
        }

        return { ani, std::nullopt };
    }

    return { std::nullopt, std::nullopt };
}

}
