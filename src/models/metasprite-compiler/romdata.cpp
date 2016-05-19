#include "romdata.h"
#include <iomanip>

using namespace UnTech::MetaSpriteCompiler;

RomOffsetPtr NULL_OFFSET(0);

void RomIncData::writeToIncFile(std::ostream& out) const
{
    // Skip new line after _stream as the first char of stream is a newline

    out << "\n.segment \"" << _segmentName << "\""
        << "\n\t.assert .loword(*) != 0, lderror, \"" << _label << " must not start on .loword(0)\"\n"
        << _label << ":"
        << _stream.str()
        << '\n';
}

void RomAddrTable::writeToIncFile(std::ostream& out) const
{
    out << "\n.segment \"" << _segmentName << "\""
        << "\n\t.assert .loword(*) != 0, lderror, \"" << _label << " must not start on .loword(0)\"\n"
        << _label << ":";

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
        out << "\n\t.addr\t";
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
    out << "\n.segment \"" << _segmentName << "\""
        << "\n\t.assert .loword(*) != 0, lderror, \"" << _label << " must not start on .loword(0)\"\n"
        << _label << ":\n";

    auto oldWidth = out.width();
    auto oldFlags = out.flags();
    auto oldFill = out.fill();

    out << std::hex << std::setfill('0');

    for (unsigned i = 0; i < _data.size(); i += BYTES_PER_LINE) {
        out << "\t.byte\t$" << std::setw(2) << (short)_data[i];

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
