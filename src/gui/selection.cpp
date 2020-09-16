/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "selection.h"
#include <cassert>

namespace UnTech::Gui {

void SingleSelection::itemMoved(unsigned from, unsigned to)
{
    if (_pending == from) {
        _pending = to;
    }
    else if (_pending > from && _pending <= to) {
        _pending--;
    }
    else if (_pending >= to && _pending < from) {
        _pending++;
    }
}

static uint64_t multipleSelectionItemMoved(const uint64_t sel, const unsigned from, const unsigned to)
{
    const uint64_t fromBit = uint64_t(1) << from;
    const uint64_t toBit = uint64_t(1) << to;

    if (from == to) {
        return sel;
    }
    else if (from < to) {
        const uint64_t mask = ((uint64_t(1) << (from + 1)) - 1) ^ ((uint64_t(1) << (to + 1)) - 1);

        uint64_t s = sel & mask;
        s >>= 1;

        if (sel & fromBit) {
            s |= toBit;
        }

        s |= sel & ~(mask | fromBit | toBit);

        return s;
    }
    else {
        const uint64_t mask = ((uint64_t(1) << from) - 1) ^ ((uint64_t(1) << to) - 1);

        uint64_t s = sel & mask;
        s <<= 1;

        if (sel & fromBit) {
            s |= toBit;
        }

        s |= sel & ~(mask | fromBit | toBit);

        return s;
    }
}

void MultipleSelection::itemMoved(unsigned from, unsigned to)
{
    _pending = multipleSelectionItemMoved(_pending, from, to);
}

void MultipleChildSelection::itemMoved(unsigned pIndex, unsigned from, unsigned to)
{
    if (pIndex == _pendingParent) {
        _pending = multipleSelectionItemMoved(_pending, from, to);
    }
}

void GroupMultipleSelection::itemMoved(unsigned pIndex, unsigned from, unsigned to)
{
    if (pIndex < childSelections.size()) {
        childSelections.at(pIndex).itemMoved(from, to);
    }
}

}
