/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/undo/selectedindexhelper.h"

using namespace UnTech::GuiQt::Undo;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

PaletteList::PaletteList(Document* document)
    : QObject(document)
    , _document(document)
    , _selectedIndex(0)
    , _selectedColor(INT_MAX)
{
    SelectedIndexHelper::buildAndConnectSlots(this);
}
