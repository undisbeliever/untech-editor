/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "romdata.h"
#include <iomanip>

using namespace UnTech::MetaSprite::Compiler;

const RomOffsetPtr RomOffsetPtr::NULL_PTR;

void RomIncData::writeToIncFile(std::ostream& out) const
{
    // Skip new line after _stream as the first char of stream is a newline

    out << "\nrodata(" << _segmentName << ")\n";

    if (_nullableType) {
        out << "\tassert(pc() & 0xffff != 0)\n";
    }
    out << _label << ":"
        << _stream.str()
        << '\n';
}

void RomAddrTable::writeToIncFile(std::ostream& out) const
{
    out << "\nrodata(" << _segmentName << ")\n";
    if (_nullableType) {
        out << "\tassert(pc() & 0xffff != 0)\n";
    }
    out << _label << ":";

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

    out << '\n';

    out.width(oldWidth);
    out.flags(oldFlags);
    out.fill(oldFill);
}

void RomBinData::writeToIncFile(std::ostream& out) const
{
    out << "\nrodata(" << _segmentName << ")\n";
    if (_nullableType) {
        out << "\tassert(pc() & 0xffff != 0)\n";
    }
    out << _label << ":\n";

    auto oldWidth = out.width();
    auto oldFlags = out.flags();
    auto oldFill = out.fill();

    out << std::hex << std::setfill('0');

    for (unsigned i = 0; i < _data.size(); i += BYTES_PER_LINE) {
        out << "\tdb\t$" << std::setw(2) << (short)_data[i];

        const unsigned end = std::min<unsigned>(i + BYTES_PER_LINE,
                                                _data.size());

        for (unsigned j = i + 1; j < end; j++) {
            out << ", $" << std::setw(2) << (short)_data[j];
        }

        out << '\n';
    }

    out.width(oldWidth);
    out.flags(oldFlags);
    out.fill(oldFill);
}
