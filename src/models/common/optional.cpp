/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 201, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "optional.h"
#include "models/resources/palette.h"

namespace UnTech {

// Test template compliles properly
template class UnTech::optional<long>;
template class UnTech::optional<Resources::PaletteInput&>;

const char* bad_optional_access::what() const noexcept
{
    return "Bad optional access";
}

}
