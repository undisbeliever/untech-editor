/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "interactive-tiles.h"
#include "models/common/errorlist.h"
#include "models/common/validateunique.h"

namespace UnTech {
namespace MetaTiles {

static const idstring BLANK_TILE_FUNCTION{ "NoTileInteraction" };

const std::array<InteractiveTileFunctionTable, InteractiveTiles::N_FIXED_FUNCTION_TABLES> InteractiveTiles::FIXED_FUNCTION_TABLES{ {
    InteractiveTileFunctionTable{ BLANK_TILE_FUNCTION, rgba(0, 0, 0, 0) },
} };

std::shared_ptr<const InteractiveTilesData>
convertInteractiveTiles(const InteractiveTiles& input, ErrorList& err)
{
    bool valid = true;

    const auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    const auto addFunctionError = [&](const InteractiveTileFunctionTable& ft, const auto... msg) {
        err.addError(std::make_unique<ListItemError>(&ft, "Interactive Tile Function Table ", ft.name, " : ", msg...));
        valid = false;
    };

    auto ret = std::make_shared<InteractiveTilesData>();

    {
        const size_t numberOfFunctions = InteractiveTiles::FIXED_FUNCTION_TABLES.size() + input.functionTables.size();

        if (numberOfFunctions > MAX_INTERACTIVE_TILE_FUNCTION_TABLES) {
            addError("Too many Interactive Tile Function Tables (got: ", numberOfFunctions, ", max: ", MAX_INTERACTIVE_TILE_FUNCTION_TABLES, ")");
        }

        // Map unused interactive tile to index 0
        ret->tileFunctionMap.emplace(idstring{}, 0);

        unsigned index = 0;

        for (const auto& ft : InteractiveTiles::FIXED_FUNCTION_TABLES) {
            ret->tileFunctionMap.emplace(ft.name, index);
            index++;
        }
        for (const auto& ft : input.functionTables) {
            if (ft.name.isValid() == false) {
                addFunctionError(ft, "Missing name");
            }

            const auto [it, added] = ret->tileFunctionMap.emplace(ft.name, index);
            index++;

            if (added == false) {
                addFunctionError(ft, "Duplicate name");
            }
        }
        assert(index == numberOfFunctions);
    }

    if (!valid) {
        return nullptr;
    }

    return ret;
}

static void writeTable(std::stringstream& incData, const InteractiveTiles& input, const std::string& tableName, bool last = false)
{
    incData << "code()\n"
               "Project.InteractiveTiles."
            << tableName << "_FunctionTable:\n";

    unsigned nFunctions = 0;
    auto writeTable = [&](const idstring& name) {
        incData << "\tdw  InteractiveTiles." << name << '.' << tableName << '\n';
        nFunctions++;
    };

    for (auto& ft : input.FIXED_FUNCTION_TABLES) {
        writeTable(ft.name);
    }
    for (auto& ft : input.functionTables) {
        writeTable(ft.name);
    }
    while (nFunctions < MAX_INTERACTIVE_TILE_FUNCTION_TABLES) {
        writeTable(BLANK_TILE_FUNCTION);
    }
    assert(nFunctions == MAX_INTERACTIVE_TILE_FUNCTION_TABLES);

    if (last) {
        incData << "Project.InteractiveTiles.EndFunctionTables:\n";
    }
}

void writeFunctionTables(std::stringstream& incData, const InteractiveTiles& input)
{
    using namespace std::string_literals;

    incData << "\n";

    writeTable(incData, input, "EntityCollision"s);
    writeTable(incData, input, "EntityAirCollision"s);
    writeTable(incData, input, "PlayerOriginCollision"s);
    writeTable(incData, input, "PlayerLeftRightCollision"s);
    writeTable(incData, input, "PlayerAirCollision"s, true);
}

const int INTERACTIVE_TILES_FORMAT_VERSION = 1;

}
}
