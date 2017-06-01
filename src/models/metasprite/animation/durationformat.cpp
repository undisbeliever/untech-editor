/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "durationformat.h"
#include <cstdio>
#include <stdexcept>

using namespace UnTech::MetaSprite::Animation;

const std::map<DurationFormat::Enum, std::string> DurationFormat::enumMap = {
    { DurationFormat::Enum::FRAME, "FRAME" },
    { DurationFormat::Enum::TIME, "TIME" },
    { DurationFormat::Enum::DISTANCE_VERTICAL, "DISTANCE_VERTICAL" },
    { DurationFormat::Enum::DISTANCE_HORIZONTAL, "DISTANCE_HORIZONTAL" }
};

const std::map<std::string, DurationFormat::Enum> DurationFormat::stringMap = {
    { "FRAME", DurationFormat::Enum::FRAME },
    { "TIME", DurationFormat::Enum::TIME },
    { "DISTANCE_VERTICAL", DurationFormat::Enum::DISTANCE_VERTICAL },
    { "DISTANCE_HORIZONTAL", DurationFormat::Enum::DISTANCE_HORIZONTAL }
};

std::string DurationFormat::durationToString(uint8_t duration) const
{
    constexpr size_t N = 15;
    char str[N + 1];

    if (duration == 0) {
        duration = 1;
    }

    switch (_value) {
    case Enum::FRAME:
        snprintf(str, N, "%d frames", duration);
        break;

    case Enum::TIME:
        snprintf(str, N, "%d ms", duration * 1000 / 75);
        break;

    case Enum::DISTANCE_HORIZONTAL:
    case Enum::DISTANCE_VERTICAL:
        snprintf(str, N, "%0.3f px", double(duration) / 32);
        break;
    }

    return str;
}
