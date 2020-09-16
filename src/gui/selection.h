/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/bit.h"
#include <array>
#include <climits>
#include <cstdint>
#include <tuple>

namespace UnTech::Gui {

/*
 * Changes to the selection state are delayed until the after the GUI has been processed.
 * This simplifies the `AbstractEditor::processGUI()` methods and prevents visual
 * inconsistencies when the selection is changed.
 */

class SingleSelection final {
public:
    constexpr static unsigned MAX_SIZE = UINT_MAX - 1;
    constexpr static unsigned NO_SELECTION = UINT_MAX;

private:
    unsigned _selected = NO_SELECTION;
    unsigned _pending = NO_SELECTION;

public:
    std::tuple<> listArgs() const { return std::make_tuple(); }

    bool hasSelection() const { return _selected != NO_SELECTION; }
    bool hasSingleSelection() const { return _selected != NO_SELECTION; }

    bool isSelected(unsigned index) const { return index == _selected; }

    unsigned selectedIndex() const { return _selected; }

    void clearSelection() { _pending = NO_SELECTION; }

    void setSelected(unsigned s) { _pending = s; }
    void appendSelection(unsigned s) { _pending = s; }

    void selectionClicked(unsigned s, bool ctrlClick)
    {
        if (ctrlClick) {
            _pending = _pending != s ? s : NO_SELECTION;
        }
        else {
            _pending = s;
        }
    }

    bool isSelectionChanging() { return _selected != _pending; }

    // Must be called after the GUI has been processed.
    void update()
    {
        _selected = _pending;
    }

    void itemMoved(unsigned from, unsigned to);
};

class MultipleSelection final {
public:
    constexpr static unsigned MAX_SIZE = 64;
    constexpr static uint64_t NO_SELECTION = 0;

private:
    uint64_t _selected = NO_SELECTION;
    uint64_t _pending = NO_SELECTION;

public:
    std::tuple<> listArgs() const { return std::make_tuple(); }

    bool hasSelection() const { return _selected != NO_SELECTION; }
    bool hasSingleSelection() const { return isPowerOfTwo(_selected); }

    bool isSelected(unsigned index) const { return _selected & (uint64_t(1) << index); }

    void clearSelection() { _pending = NO_SELECTION; }

    void setSelected(unsigned s) { _pending = uint64_t(1) << s; }
    void appendSelection(unsigned s) { _pending |= uint64_t(1) << s; }

    void selectionClicked(unsigned s, bool ctrlClick)
    {
        const uint64_t b = uint64_t(1) << s;

        if (ctrlClick) {
            _pending ^= b;
        }
        else {
            _pending = b;
        }
    }

    // Must be called after the GUI has been processed.
    void update()
    {
        _selected = _pending;
    }

    void itemMoved(unsigned from, unsigned to);
};

// Not a child class of MultipleSelection.
// Forces the use of parent in `update()`.
class MultipleChildSelection final {
public:
    constexpr static unsigned MAX_SIZE = 64;
    constexpr static uint64_t NO_SELECTION = 0;

private:
    unsigned _parent = SingleSelection::NO_SELECTION;
    unsigned _pendingParent = SingleSelection::NO_SELECTION;

    uint64_t _selected = NO_SELECTION;
    uint64_t _pending = NO_SELECTION;

public:
    std::tuple<unsigned> listArgs() const { return { _parent }; }

    unsigned parentIndex() const { return _parent; }

    bool hasSelection() const { return _selected != NO_SELECTION; }
    bool hasSingleSelection() const { return isPowerOfTwo(_selected); }

    bool isSelected(unsigned index) const { return _selected & (uint64_t(1) << index); }
    bool isSelected(unsigned parent, unsigned index) const { return parent == _parent && isSelected(index); }

    void clearSelection() { _pending = NO_SELECTION; }

    void appendSelection(unsigned s) { _pending |= uint64_t(1) << s; }

    void setSelected(unsigned p, unsigned s)
    {
        _pendingParent = p;
        _pending = uint64_t(1) << s;
    }

    void selectionClicked(unsigned p, unsigned s, bool ctrlClick)
    {
        if (_parent != p) {
            _pendingParent = p;
            _pending = 0;
        }

        const uint64_t b = uint64_t(1) << s;

        if (ctrlClick) {
            _pending ^= b;
        }
        else {
            _pending = b;
        }
    }

    void selectionClicked(unsigned s, bool ctrlClick)
    {
        const uint64_t b = uint64_t(1) << s;

        if (ctrlClick) {
            _pending ^= b;
        }
        else {
            _pending = b;
        }
    }

    bool isSelectionChanging(const SingleSelection& parentSel) const
    {
        return _pendingParent != _parent
               || _parent != parentSel.selectedIndex()
               || _pending != _selected;
    }

    // Must be called after the GUI has been processed.
    void update(const SingleSelection& parentSel)
    {
        _parent = _pendingParent;

        if (_parent != parentSel.selectedIndex()) {
            _parent = parentSel.selectedIndex();
            _pendingParent = _parent;
            _selected = NO_SELECTION;
            _pending = NO_SELECTION;
        }
        else {
            _selected = _pending;
        }
    }

    void itemMoved(unsigned pIndex, unsigned from, unsigned to);
};

// Only intended groups with a small MAX_SIZE value
class GroupMultipleSelection {
public:
    constexpr static unsigned MAX_GROUP_SIZE = 8;
    constexpr static unsigned MAX_SIZE = MultipleSelection::MAX_SIZE;

public:
    std::array<MultipleSelection, MAX_GROUP_SIZE> childSelections;

public:
    MultipleSelection& childSel(unsigned groupIndex) { return childSelections.at(groupIndex); }
    const MultipleSelection& childSel(unsigned groupIndex) const { return childSelections.at(groupIndex); }

    bool isSelected(unsigned groupIndex, unsigned index) const { return groupIndex < MAX_GROUP_SIZE && childSelections.at(groupIndex).isSelected(index); }

    bool hasSingleSelection() const
    {
        int s = 0;
        for (const MultipleSelection& g : childSelections) {
            if (g.hasSelection()) {
                s++;
            }
        }
        return s == 1;
    }

    void clearSelection()
    {
        for (MultipleSelection& g : childSelections) {
            g.clearSelection();
        }
    }

    void setSelected(unsigned groupIndex, unsigned index)
    {
        clearSelection();
        if (groupIndex < childSelections.size()) {
            childSelections.at(groupIndex).setSelected(index);
        }
    }

    void selectionClicked(unsigned groupIndex, unsigned index, bool ctrlClicked)
    {
        if (not ctrlClicked) {
            setSelected(groupIndex, index);
        }
        else {
            if (groupIndex < childSelections.size()) {
                childSelections.at(groupIndex).selectionClicked(index, true);
            }
        }
    }

    // Must be called after the GUI has been processed.
    void update()
    {
        for (MultipleSelection& g : childSelections) {
            g.update();
        }
    }

    void itemMoved(unsigned pIndex, unsigned from, unsigned to);
};

// A simple true/false selection that uses the same API as MultipleSelection
class ToggleSelection final {
public:
    constexpr static unsigned MAX_SIZE = UINT_MAX - 1;
    constexpr static bool NO_SELECTION = false;

private:
    bool _selected = NO_SELECTION;
    bool _pending = NO_SELECTION;

public:
    bool hasSingleSelection() const { return _selected; }

    bool isSelected() const { return _selected; }

    bool isSelected(bool s) const { return s && _selected; }

    void clearSelection() { _pending = NO_SELECTION; }

    void setSelected(bool s) { _pending = s; }
    void appendSelection(bool s) { _pending |= s; }

    void selectionClicked(unsigned)
    {
        _pending = true;
    }

    void selectionClicked(unsigned, bool ctrlClick)
    {
        if (ctrlClick) {
            _pending = !_selected;
        }
        else {
            _pending = true;
        }
    }

    // Must be called after the GUI has been processed.
    void update()
    {
        _selected = _pending;
    }
};

}
