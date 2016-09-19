#include "romtiledata.h"
#include "models/snes/tile.hpp"
#include <algorithm>
#include <iomanip>

using namespace UnTech::MetaSprite::Compiler;

void RomTileData::writeToIncFile(std::ostream& out) const
{
    constexpr static unsigned SNES_DATA_SIZE = Snes::Tile4bpp16px::SNES_DATA_SIZE;

    auto oldWidth = out.width();
    auto oldFlags = out.flags();
    auto oldFill = out.fill();

    uint8_t buffer[SNES_DATA_SIZE] = { 0 };

    const unsigned tilesPerBlock = _blockSize / SNES_DATA_SIZE;
    const unsigned nBlocks = _tiles.size() / tilesPerBlock + 1;

    for (unsigned blockId = 0; blockId < nBlocks; blockId++) {
        out << "\nrodata(" << _segmentPrefix << '_' << blockId << ")\n"
            << "\tassert(pc() & 0x7f == 0)\n"
            << _blockPrefix << '_' << std::dec << blockId << ":\n";

        out << std::hex << std::setfill('0');

        const unsigned begin = tilesPerBlock * blockId;
        const unsigned end = std::min<unsigned>(begin + tilesPerBlock,
                                                _tiles.size());

        for (unsigned t = begin; t < end; t++) {
            _tiles[t].writeSnesData(buffer);

            static_assert(SNES_DATA_SIZE % BYTES_PER_LINE == 0, "Bad assumption");

            // NOTE: All strings pushed to out MUST CONTAIN more than 2 characters.

            unsigned bPos = 0;
            unsigned lines = SNES_DATA_SIZE / BYTES_PER_LINE;
            for (unsigned l = 0; l < lines; l++) {
                out << "\tdb\t$" << std::setw(2) << (short)buffer[bPos];
                bPos++;

                for (unsigned j = 1; j < BYTES_PER_LINE; j++) {
                    out << ", $" << std::setw(2) << (short)buffer[bPos];
                    bPos++;
                }

                out << '\n';
            }
        }
    }

    out.width(oldWidth);
    out.flags(oldFlags);
    out.fill(oldFill);
}
