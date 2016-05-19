#include "tilesettype.h"
#include <stdexcept>

using namespace UnTech::MetaSpriteFormat;

const std::map<TilesetType::Enum, std::string> TilesetType::enumMap = {
    { TilesetType::Enum::ONE_16_TILE, "ONE_16_TILE" },
    { TilesetType::Enum::TWO_16_TILES, "TWO_16_TILES" },
    { TilesetType::Enum::ONE_VRAM_ROW, "ONE_VRAM_ROW" },
    { TilesetType::Enum::TWO_VRAM_ROWS, "TWO_VRAM_ROWS" },
    { TilesetType::Enum::ONE_16_TILE_FIXED, "ONE_16_TILE_FIXED" },
    { TilesetType::Enum::TWO_16_TILES_FIXED, "TWO_16_TILES_FIXED" },
    { TilesetType::Enum::ONE_VRAM_ROW_FIXED, "ONE_VRAM_ROW_FIXED" },
    { TilesetType::Enum::TWO_VRAM_ROWS_FIXED, "TWO_VRAM_ROWS_FIXED" },
};

const std::map<std::string, TilesetType::Enum> TilesetType::stringMap = {
    { "ONE_16_TILE", TilesetType::Enum::ONE_16_TILE },
    { "TWO_16_TILES", TilesetType::Enum::TWO_16_TILES },
    { "ONE_VRAM_ROW", TilesetType::Enum::ONE_VRAM_ROW },
    { "TWO_VRAM_ROWS", TilesetType::Enum::TWO_VRAM_ROWS },
    { "ONE_16_TILE_FIXED", TilesetType::Enum::ONE_16_TILE_FIXED },
    { "TWO_16_TILES_FIXED", TilesetType::Enum::TWO_16_TILES_FIXED },
    { "ONE_VRAM_ROW_FIXED", TilesetType::Enum::ONE_VRAM_ROW_FIXED },
    { "TWO_VRAM_ROWS_FIXED", TilesetType::Enum::TWO_VRAM_ROWS_FIXED },
};

TilesetType TilesetType::smallestFixedTileset(unsigned tilesetSize)
{
    if (tilesetSize > 16) {
        throw std::invalid_argument("too many tiles");
    }
    else if (tilesetSize > 8) {
        return TilesetType(Enum::TWO_VRAM_ROWS_FIXED);
    }
    else if (tilesetSize > 2) {
        return TilesetType(Enum::ONE_VRAM_ROW_FIXED);
    }
    else if (tilesetSize == 2) {
        return TilesetType(Enum::TWO_16_TILES_FIXED);
    }
    else {
        return TilesetType(Enum::ONE_16_TILE_FIXED);
    }
}

unsigned TilesetType::nTiles() const
{
    switch (_value) {
    case Enum::ONE_16_TILE:
    case Enum::ONE_16_TILE_FIXED:
        return 1;

    case Enum::TWO_16_TILES:
    case Enum::TWO_16_TILES_FIXED:
        return 2;

    case Enum::ONE_VRAM_ROW:
    case Enum::ONE_VRAM_ROW_FIXED:
        return 8;

    case Enum::TWO_VRAM_ROWS:
    case Enum::TWO_VRAM_ROWS_FIXED:
        return 16;

    default:
        return 0;
    }
}

unsigned TilesetType::tilesetSplitPoint() const
{
    switch (_value) {
    case Enum::ONE_16_TILE:
    case Enum::ONE_16_TILE_FIXED:
    case Enum::TWO_16_TILES:
    case Enum::TWO_16_TILES_FIXED:
        return 1;

    case Enum::ONE_VRAM_ROW:
    case Enum::ONE_VRAM_ROW_FIXED:
    case Enum::TWO_VRAM_ROWS:
    case Enum::TWO_VRAM_ROWS_FIXED:
        return 8;

    default:
        return 0;
    }
}
