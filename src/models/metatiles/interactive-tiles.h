/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include "models/common/rgba.h"
#include <sstream>

namespace UnTech {
class ErrorList;

namespace MetaTiles {

struct InteractiveTileFunctionTable {
    idstring name;
    rgba tint = rgba(255, 200, 0);

    bool operator==(const InteractiveTileFunctionTable& o) const
    {
        return name == o.name
               && tint == o.tint;
    }
};

struct InteractiveTiles {
    constexpr static unsigned N_FIXED_FUNCTION_TABLES = 1;
    const static std::array<InteractiveTileFunctionTable, N_FIXED_FUNCTION_TABLES> FIXED_FUNCTION_TABLES;

    constexpr static unsigned MAX_PROJECT_FUNCTION_TABLES = MAX_INTERACTIVE_TILE_FUNCTION_TABLES - N_FIXED_FUNCTION_TABLES;

    NamedList<InteractiveTileFunctionTable> functionTables;

    // Gets the function table used by the interactive tile with the id of `i`.
    const InteractiveTileFunctionTable& getFunctionTable(const unsigned i) const
    {
        if (i < FIXED_FUNCTION_TABLES.size()) {
            return FIXED_FUNCTION_TABLES.at(i);
        }
        else {
            return functionTables.at(i - FIXED_FUNCTION_TABLES.size());
        }
    }

    bool operator==(const InteractiveTiles& o) const
    {
        return functionTables == o.functionTables;
    }
};

struct InteractiveTilesData {
    std::unordered_map<idstring, unsigned> tileFunctionMap;
};
extern const int INTERACTIVE_TILES_FORMAT_VERSION;

// Also validates InteractiveTiles
std::shared_ptr<const InteractiveTilesData>
convertInteractiveTiles(const InteractiveTiles& input, ErrorList& err);

// Assumes InteractiveTiles is valid
void writeFunctionTables(std::stringstream& incData, const InteractiveTiles& input);

}
}
