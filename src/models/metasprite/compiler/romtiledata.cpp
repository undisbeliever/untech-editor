/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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

        out << '(' << tilePrefix << t.block
            << " + " << t.offset << ") >> 7";

        if (i + 1 < _tileCount) {
            out << ", ";
        }
    }

    out << "\n\n";
}

void RomDmaTile16Data::writeToIncFile(std::ostream& out) const
{
    out << _label << ":\n";

    for (const auto& e : _entries) {
        e.writeToIncFile(out, _tilePrefix);
    }
}

void RomTileData::writeAssertsToIncFile(std::ostream& out) const
{
    for (unsigned i = 0; i < nBlocks(); i++) {
        out << "assert(" << _blockPrefix << i << " & 0x7f == 0)\n";
    }
}

std::vector<std::vector<uint8_t>> RomTileData::data() const
{
    std::vector<std::vector<uint8_t>> ret;
    ret.reserve(nBlocks());

    for (auto& tileset : _tileBlocks) {
        ret.emplace_back(tileset.snesData());
    }

    return ret;
}
