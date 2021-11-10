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

class TilesetError : public GenericListError {
public:
    const TilesetErrorType type;

    TilesetError(const TilesetErrorType type, unsigned pIndex, std::u8string&& message)
        : GenericListError(pIndex, std::move(message))
        , type(type)
    {
    }

    TilesetError(const TilesetErrorType type, unsigned pIndex, unsigned cIndex, std::u8string&& message)
        : GenericListError(pIndex, cIndex, std::move(message))
        , type(type)
    {
    }
};

enum class InteractiveTilesErrorType {
    FUNCTION_TABLE,
};

class InteractiveTilesError : public GenericListError {
public:
    const InteractiveTilesErrorType type;

    InteractiveTilesError(const InteractiveTilesErrorType type, unsigned pIndex, std::u8string&& message)
        : GenericListError(pIndex, std::move(message))
        , type(type)
    {
    }

    InteractiveTilesError(const InteractiveTilesErrorType type, unsigned pIndex, unsigned cIndex, std::u8string&& message)
        : GenericListError(pIndex, cIndex, std::move(message))
        , type(type)
    {
    }
};

}
