/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "romdata.h"
#include <iomanip>

using namespace UnTech::MetaSprite::Compiler;

void RomAddrTable::writeToIncFile(std::ostream& out) const
{
    out << _label << ":";
    if (_nullableType) {
        out << "\tassert(pc() & 0xffff != 0)\n";
    }

    auto oldWidth = out.width();
    auto oldFlags = out.flags();
    auto oldFill = out.fill();

    auto writeOffset = [&](unsigned i) {
        uint32_t v = _offsets[i];
        if (v <= 0xFFFF) {
            out << _dataLabel << " + " << v;
        }
        else {
            out << '0';
        }
    };

    for (unsigned i = 0; i < _offsets.size(); i += ADDR_PER_LINE) {
        out << "\n\tdw\t";
        writeOffset(i);

        const unsigned end = std::min<unsigned>(i + ADDR_PER_LINE,
                                                _offsets.size());

        for (unsigned j = i + 1; j < end; j++) {
            out << ", ";
            writeOffset(j);
        }
    }

    out << "\n\n";

    out.width(oldWidth);
    out.flags(oldFlags);
    out.fill(oldFill);
}
