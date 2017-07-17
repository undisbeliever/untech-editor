/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettecommands.h"
#include "document.h"
#include "palettesmodel.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

// AddRemovePalette
// ================

AddRemovePalette::AddRemovePalette(Document* document,
                                   unsigned index, const Snes::Palette4bpp& palette,
                                   const QString& text)
    : QUndoCommand(text)
    , _document(document)
    , _index(index)
    , _palette(palette)
{
}

void AddRemovePalette::addPalette()
{
    _document->palettesModel()->insertPalette(_index, _palette);
}

void AddRemovePalette::removePalette()
{
    _document->palettesModel()->removePalette(_index);
}

// AddPalette
// ==========

AddPalette::AddPalette(Document* document)
    : AddRemovePalette(document,
                       document->frameSet()->palettes.size(),
                       Snes::Palette4bpp(),
                       QCoreApplication::tr("Add Palette"))
{
}

void AddPalette::undo()
{
    removePalette();
}
void AddPalette::redo()
{
    addPalette();
}

// ClonePalette
// ============

ClonePalette::ClonePalette(Document* document, unsigned index)
    : AddRemovePalette(document,
                       document->frameSet()->palettes.size(),
                       document->frameSet()->palettes.at(index),
                       QCoreApplication::tr("Clone Palette"))
{
}

void ClonePalette::undo()
{
    removePalette();
}
void ClonePalette::redo()
{
    addPalette();
}

// RemovePalette
// =============

RemovePalette::RemovePalette(Document* document, unsigned index)
    : AddRemovePalette(document,
                       index,
                       document->frameSet()->palettes.at(index),
                       QCoreApplication::tr("Remove Palette"))
{
}

void RemovePalette::undo()
{
    addPalette();
}
void RemovePalette::redo()
{
    removePalette();
}

// RaisePalette
// ============

RaisePalette::RaisePalette(Document* document, unsigned index)
    : QUndoCommand(QCoreApplication::tr("Raise Palette"))
    , _document(document)
    , _index(index)
{
}

void RaisePalette::undo()
{
    _document->palettesModel()->lowerPalette(_index - 1);
}

void RaisePalette::redo()
{
    _document->palettesModel()->raisePalette(_index);
}

// LowerPalette
// ============

LowerPalette::LowerPalette(Document* document, unsigned index)
    : QUndoCommand(QCoreApplication::tr("Lower Palette"))
    , _document(document)
    , _index(index)
{
}

void LowerPalette::undo()
{
    _document->palettesModel()->raisePalette(_index + 1);
}

void LowerPalette::redo()
{
    _document->palettesModel()->lowerPalette(_index);
}
