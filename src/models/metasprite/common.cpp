/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "common.h"

using namespace UnTech::MetaSprite;

std::string NameReference::str() const
{
    if (hFlip && vFlip) {
        return name + " (hvFlip)";
    }
    else if (hFlip) {
        return name + " (hFlip)";
    }
    else if (vFlip) {
        return name + " (vFlip)";
    }
    else {
        return name;
    }
}
