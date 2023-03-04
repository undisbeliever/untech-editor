/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"

namespace UnTech::MetaTiles {

enum class TilesetErrorType {
    TILE,
};
using TilesetError = GenericListError<TilesetErrorType>;

enum class InteractiveTilesErrorType {
    FUNCTION_TABLE,
};
using InteractiveTilesError = GenericListError<InteractiveTilesErrorType>;

}
