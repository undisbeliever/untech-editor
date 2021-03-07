/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "selection.h"
#include <cassert>
#include <limits>

namespace UnTech::Gui {

template <typename index_type>
static inline void singleSelectionItemAdded(index_type& pending, const index_type index)
{
    if (pending >= index) {
        pending++;
    }
}

template <typename index_type>
static inline void singleSelectionItemRemoved(index_type& pending, const index_type index)
{
    if (pending == index) {
        pending = std::numeric_limits<index_type>::max();
    }
    else if (pending > index) {
        pending--;
    }
}

template <typename index_type>
static inline void singleSelectionItemMoved(index_type& pending, const index_type from, const index_type to)
{
    if (pending == from) {
        pending = to;
    }
    else if (pending > from && pending <= to) {
        pending--;
    }
    else if (pending >= to && pending < from) {
        pending++;
    }
}

void SingleSelection::itemAdded(unsigned index)
{
    singleSelectionItemAdded(_pending, index);
}

void SingleSelection::itemRemoved(unsigned index)
{
    singleSelectionItemRemoved(_pending, index);
}

void SingleSelection::itemMoved(unsigned from, unsigned to)
{
    singleSelectionItemMoved(_pending, from, to);
}

static uint64_t multipleSelectionItemAdded(const uint64_t sel, const unsigned index)
{
    const uint64_t mask = ~((uint64_t(1) << index) - 1);

    uint64_t s = (sel & mask) << 1;
    s |= sel & ~mask;
    return s;
}

static uint64_t multipleSelectionItemRemoved(const uint64_t sel, const unsigned index)
{
    const uint64_t indexBit = uint64_t(1) << index;
    const uint64_t shiftMask = ~((uint64_t(1) << (index + 1)) - 1);
    const uint64_t keepMask = ~(shiftMask | indexBit);

    uint64_t s = (sel & shiftMask) >> 1;
    s |= sel & keepMask;
    return s;
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

void MultipleSelection::itemAdded(unsigned index)
{
    _pending = multipleSelectionItemAdded(_pending, index);
}

void MultipleSelection::itemRemoved(unsigned index)
{
    _pending = multipleSelectionItemRemoved(_pending, index);
}

void MultipleSelection::itemMoved(unsigned from, unsigned to)
{
    _pending = multipleSelectionItemMoved(_pending, from, to);
}

void MultipleChildSelection::itemAdded(unsigned pIndex, unsigned index)
{
    if (pIndex == _pendingParent) {
        _pending = multipleSelectionItemAdded(_pending, index);
    }
}

void MultipleChildSelection::itemRemoved(unsigned pIndex, unsigned index)
{
    if (pIndex == _pendingParent) {
        _pending = multipleSelectionItemRemoved(_pending, index);
    }
}

void MultipleChildSelection::itemMoved(unsigned pIndex, unsigned from, unsigned to)
{
    if (pIndex == _pendingParent) {
        _pending = multipleSelectionItemMoved(_pending, from, to);
    }
}

void GroupMultipleSelection::itemAdded(unsigned pIndex, unsigned index)
{
    if (pIndex < childSelections.size()) {
        childSelections.at(pIndex).itemAdded(index);
    }
}

void GroupMultipleSelection::itemRemoved(unsigned pIndex, unsigned index)
{
    if (pIndex < childSelections.size()) {
        childSelections.at(pIndex).itemRemoved(index);
    }
}

void GroupMultipleSelection::itemMoved(unsigned pIndex, unsigned from, unsigned to)
{
    if (pIndex < childSelections.size()) {
        childSelections.at(pIndex).itemMoved(from, to);
    }
}

void NodeSelection::itemAdded(const ParentIndexT& pIndex, index_type index)
{
    if (pIndex == _pendingParent) {
        singleSelectionItemAdded(_pending, index);
    }
}

void NodeSelection::itemRemoved(const ParentIndexT& pIndex, index_type index)
{
    if (pIndex == _pendingParent) {
        singleSelectionItemRemoved(_pending, index);
    }
}

void NodeSelection::itemMoved(const ParentIndexT& pIndex, index_type from, index_type to)
{
    if (pIndex == _pendingParent) {
        singleSelectionItemMoved(_pending, from, to);
    }
}

}
