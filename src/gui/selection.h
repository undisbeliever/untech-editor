/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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

    bool isSelected(unsigned index) const { return index == _selected; }

    unsigned selectedIndex() const { return _selected; }

    void clearSelection() { _pending = NO_SELECTION; }

    void setSelected(unsigned s) { _pending = s; }

    void selectionClicked(unsigned s, bool ctrlClick)
    {
        if (ctrlClick) {
            _pending = _pending != s ? s : NO_SELECTION;
        }
        else {
            _pending = s;
        }
    }

    // Must be called after the GUI has been processed.
    void update()
    {
        _selected = _pending;
    }
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

    bool isSelected(unsigned index) const { return _selected & (uint64_t(1) << index); }

    void clearSelection() { _pending = NO_SELECTION; }

    void setSelected(unsigned s) { _pending = uint64_t(1) << s; }

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

    bool isSelected(unsigned index) const { return _selected & (uint64_t(1) << index); }
    bool isSelected(unsigned parent, unsigned index) const { return parent == _parent && isSelected(index); }

    void clearSelection() { _pending = NO_SELECTION; }

    void setSelected(unsigned s) { _pending = uint64_t(1) << s; }

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

    // Must be called after the GUI has been processed.
    void update(const SingleSelection& parentSel)
    {
        _parent = _pendingParent;

        if (_parent != parentSel.selectedIndex()) {
            _parent = parentSel.selectedIndex();
            _pendingParent = NO_SELECTION;
            _selected = NO_SELECTION;
            _pending = NO_SELECTION;
        }
        else {
            _selected = _pending;
        }
    }
};

}
