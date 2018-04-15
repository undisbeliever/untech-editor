/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metasprite/metasprite/document.h"
#include "gui-qt/undo/undo.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {

class PaletteList : public QObject {
    Q_OBJECT

public:
    constexpr static size_t max_size = UnTech::MetaSprite::MAX_PALETTES;

    using DataT = UnTech::Snes::Palette4bpp;
    using ListT = capped_vector<Snes::Palette4bpp, max_size>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<>;

private:
    Document* const _document;

    index_type _selectedIndex;
    unsigned _selectedColor;

public:
    PaletteList(Document* document);

    Document* resourceItem() const { return _document; }

    QString typeName() const { return tr("Palette"); }

    index_type selectedIndex() const { return _selectedIndex; }
    void setSelectedIndex(index_type index)
    {
        // always ensure a palette is selected
        const auto* palettes = this->palettes();
        if (palettes == nullptr || index >= palettes->size()) {
            index = 0;
        }

        if (_selectedIndex != index) {
            _selectedIndex = index;
            emit selectedIndexChanged();
        }
    }
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool isSelectedIndexValid() const
    {
        const MS::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return false;
        }
        return _selectedIndex < fs->palettes.size();
    }

    index_type selectedColor() const { return _selectedColor; }
    void setSelectedColor(unsigned color)
    {
        if (_selectedColor != color) {
            _selectedColor = color;
            emit selectedColorChanged();
        }
    }
    void unselectColor() { setSelectedColor(INT_MAX); }

    bool isSelectedColorValid() const
    {
        return isSelectedIndexValid()
               && _selectedColor < 16;
    }

    const ListT* palettes() const
    {
        MS::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->palettes;
    }

    const DataT* selectedPalette() const
    {
        auto* pl = palettes();
        if (pl == nullptr) {
            return nullptr;
        }
        if (_selectedIndex >= pl->size()) {
            return nullptr;
        }
        return &pl->at(_selectedIndex);
    }

protected:
    friend class Undo::ListUndoHelper<PaletteList>;
    friend class Undo::ListActionHelper;
    friend class Undo::SelectedIndexHelper;
    ListT* getList()
    {
        MS::FrameSet* fs = _document->frameSet();
        if (fs == nullptr) {
            return nullptr;
        }
        return &fs->palettes;
    }

    ArgsT selectedListTuple() const
    {
        return std::make_tuple();
    }

signals:
    void dataChanged(index_type index);
    void listChanged();

    void listAboutToChange();
    void itemAdded(index_type index);
    void itemAboutToBeRemoved(index_type index);
    void itemMoved(index_type from, index_type to);

    void selectedIndexChanged();
    void selectedColorChanged();
};

using PaletteListUndoHelper = Undo::ListAndSelectionUndoHelper<PaletteList>;
}
}
}
}
