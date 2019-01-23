/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "romtiledata.h"
#include <algorithm>
#include <iomanip>

using namespace UnTech::MetaSprite::Compiler;

inline void RomDmaTile16Entry::writeToIncFile(std::ostream& out, const std::string& tilePrefix) const
{
    assert(_tileCount > 0);

    out << "\tdb\t" << _tileCount << "\n\tdw\t";

    for (unsigned i = 0; i < _tileCount; i++) {
        auto& t = _tiles.at(i);

        out << '(' << tilePrefix << '_' << t.block
            << " + " << t.offset << ") >> 7";

        if (i + 1 < _tileCount) {
            out << ", ";
        }
    }

    out << '\n';
}

void RomDmaTile16Data::writeToIncFile(std::ostream& out) const
{
    out << "\nrodata(" << _segmentName << ")\n"
        << _label << ":\n";

    for (const auto& e : _entries) {
        e.writeToIncFile(out, _tilePrefix);
    }
}

void RomTileData::writeToIncFile(std::ostream& out) const
{
    auto oldWidth = out.width();
    auto oldFlags = out.flags();
    auto oldFill = out.fill();

    const std::vector<uint8_t> snesData = _tiles.snesData();

    const unsigned tilesPerBlock = _blockSize / SNES_TILE16_SIZE;
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
            const uint8_t* tData = snesData.data() + t * SNES_TILE16_SIZE;

            static_assert(SNES_TILE16_SIZE % BYTES_PER_LINE == 0, "Bad assumption");

            // NOTE: All strings pushed to out MUST CONTAIN more than 2 characters.

            unsigned bPos = 0;
            unsigned lines = SNES_TILE16_SIZE / BYTES_PER_LINE;
            for (unsigned l = 0; l < lines; l++) {
                out << "\tdb\t$" << std::setw(2) << (short)tData[bPos];
                bPos++;

                for (unsigned j = 1; j < BYTES_PER_LINE; j++) {
                    out << ", $" << std::setw(2) << (short)tData[bPos];
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
