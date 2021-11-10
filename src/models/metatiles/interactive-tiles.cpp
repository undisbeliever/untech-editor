/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "interactive-tiles.h"
#include "metatiles-error.h"
#include "models/common/stringstream.h"
#include "models/common/validateunique.h"

namespace UnTech::MetaTiles {

static const idstring BLANK_TILE_FUNCTION = u8"NoTileInteraction"_id;

const std::array<InteractiveTileFunctionTable, InteractiveTiles::N_FIXED_FUNCTION_TABLES> InteractiveTiles::FIXED_FUNCTION_TABLES{ {
    InteractiveTileFunctionTable{ BLANK_TILE_FUNCTION, rgba(0, 0, 0, 0) },
} };

template <typename... Args>
std::unique_ptr<InteractiveTilesError> functionTableError(const InteractiveTileFunctionTable& ft, const unsigned ftIndex, const Args... msg)
{
    return std::make_unique<InteractiveTilesError>(InteractiveTilesErrorType::FUNCTION_TABLE, ftIndex,
                                                   stringBuilder(u8"Interactive Tile Function Table ", ft.name, u8" : ", msg...));
}

std::shared_ptr<const InteractiveTilesData>
convertInteractiveTiles(const InteractiveTiles& input, ErrorList& err)
{
    bool valid = true;

    const auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    const auto addFunctionError = [&](const InteractiveTileFunctionTable& ft, const unsigned ftIndex, const auto... msg) {
        err.addError(functionTableError(ft, ftIndex, msg...));
        valid = false;
    };

    auto ret = std::make_shared<InteractiveTilesData>();

    {
        const size_t numberOfFunctions = InteractiveTiles::FIXED_FUNCTION_TABLES.size() + input.functionTables.size();

        if (numberOfFunctions > MAX_INTERACTIVE_TILE_FUNCTION_TABLES) {
            addError(u8"Too many Interactive Tile Function Tables (got: ", numberOfFunctions, u8", max: ", MAX_INTERACTIVE_TILE_FUNCTION_TABLES, u8")");
        }

        // Map unused interactive tile to index 0
        ret->tileFunctionMap.emplace(idstring{}, 0);

        unsigned index = 0;

        for (const auto& ft : InteractiveTiles::FIXED_FUNCTION_TABLES) {
            ret->tileFunctionMap.emplace(ft.name, index);
            index++;
        }
        for (const auto [ftIndex, ft] : enumerate(input.functionTables)) {
            if (ft.name.isValid() == false) {
                addFunctionError(ft, ftIndex, u8"Missing name");
            }

            const auto [it, added] = ret->tileFunctionMap.emplace(ft.name, index);
            index++;

            if (added == false) {
                addFunctionError(ft, ftIndex, u8"Duplicate name");
            }
        }
        assert(index == numberOfFunctions);
    }

    if (!valid) {
        return nullptr;
    }

    return ret;
}

static void writeTable(StringStream& incData, const InteractiveTiles& input, const std::u8string& tableName, bool last = false)
{
    incData.write(u8"code()\n",
                  u8"Project.InteractiveTiles.", tableName, u8"_FunctionTable:\n");

    unsigned nFunctions = 0;
    auto writeTable = [&](const idstring& name) {
        incData.write(u8"\tdw  InteractiveTiles.", name, u8".", tableName, u8"\n");
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
        incData.write(u8"Project.InteractiveTiles.EndFunctionTables:\n");
    }
}

void writeFunctionTables(StringStream& incData, const InteractiveTiles& input)
{
    using namespace std::string_literals;

    incData.write(u8"\n");

    writeTable(incData, input, u8"EntityCollision"s);
    writeTable(incData, input, u8"EntityAirCollision"s);
    writeTable(incData, input, u8"PlayerOriginCollision"s);
    writeTable(incData, input, u8"PlayerLeftRightCollision"s);
    writeTable(incData, input, u8"PlayerAirCollision"s, true);
}

const int INTERACTIVE_TILES_FORMAT_VERSION = 1;

}
