/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "bit-depth.h"
#include <stdexcept>

namespace UnTech::Snes {

// cppcheck-suppress unusedFunction
BitDepth toBitDepth(unsigned bd)
{
    switch (bd) {
    case 2:
        return BitDepth::BD_2BPP;
    case 4:
        return BitDepth::BD_4BPP;
    case 8:
        return BitDepth::BD_8BPP;
    }

    throw std::runtime_error("Invalid bit depth");
}

BitDepthSpecial toBitDepthSpecial(unsigned bd)
{

    switch (bd) {
    case 1:
        return BitDepthSpecial::BD_1BPP;
    case 2:
        return BitDepthSpecial::BD_2BPP;
    case 3:
        return BitDepthSpecial::BD_3BPP;
    case 4:
        return BitDepthSpecial::BD_4BPP;
    case 8:
        return BitDepthSpecial::BD_8BPP;
    }

    throw std::runtime_error("Invalid bit depth");
}

}
