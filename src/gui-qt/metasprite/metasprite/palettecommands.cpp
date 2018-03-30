/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettecommands.h"
#include "document.h"
#include "models/common/vector-helpers.h"

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
    auto& palettes = _document->frameSet()->palettes;
    Q_ASSERT(_index <= palettes.size());

    palettes.insert(palettes.begin() + _index, _palette);

    emit _document->paletteAdded(_index);
}

void AddRemovePalette::removePalette()
{
    auto& palettes = _document->frameSet()->palettes;
    Q_ASSERT(_index < palettes.size());

    emit _document->paletteAboutToBeRemoved(_index);

    palettes.erase(palettes.begin() + _index);
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

// MovePalette
// ===========

MovePalette::MovePalette(Document* document, unsigned fromIndex, unsigned toIndex)
    : MovePalette(document, fromIndex, toIndex,
                  QCoreApplication::tr("Move Palette"))
{
}

MovePalette::MovePalette(Document* document, unsigned fromIndex, unsigned toIndex,
                         const QString& text)
    : QUndoCommand(text)
    , _document(document)
    , _fromIndex(fromIndex)
    , _toIndex(toIndex)
{
    Q_ASSERT(_document);
    Q_ASSERT(_fromIndex != _toIndex);
}

void MovePalette::undo()
{
    auto& palettes = _document->frameSet()->palettes;

    Q_ASSERT(_fromIndex < palettes.size());
    Q_ASSERT(_toIndex < palettes.size());

    moveVectorItem(_toIndex, _fromIndex, palettes);

    emit _document->paletteMoved(_toIndex, _fromIndex);
}

void MovePalette::redo()
{
    auto& palettes = _document->frameSet()->palettes;

    Q_ASSERT(_fromIndex < palettes.size());
    Q_ASSERT(_toIndex < palettes.size());

    moveVectorItem(_fromIndex, _toIndex, palettes);

    emit _document->paletteMoved(_fromIndex, _toIndex);
}

// RaisePalette
// ============

RaisePalette::RaisePalette(Document* document, unsigned index)
    : MovePalette(document, index, index - 1,
                  QCoreApplication::tr("Raise Palette"))
{
}

// LowerPalette
// ============

LowerPalette::LowerPalette(Document* document, unsigned index)
    : MovePalette(document, index, index + 1,
                  QCoreApplication::tr("Lower Palette"))
{
}

// ChangePaletteColor
// ==================

ChangePaletteColor::ChangePaletteColor(Document* document,
                                       unsigned paletteIndex, unsigned colorIndex)
    : QUndoCommand(QCoreApplication::tr("Change Palette Color"))
    , _document(document)
    , _paletteIndex(paletteIndex)
    , _colorIndex(colorIndex)
    , _oldColor(_document->frameSet()->palettes.at(paletteIndex).color(colorIndex))
    , _newColor(_oldColor)
{
}

ChangePaletteColor::ChangePaletteColor(Document* document,
                                       unsigned paletteIndex, unsigned colorIndex,
                                       const Snes::SnesColor& color)
    : QUndoCommand(QCoreApplication::tr("Change Palette Color"))
    , _document(document)
    , _paletteIndex(paletteIndex)
    , _colorIndex(colorIndex)
    , _oldColor(_document->frameSet()->palettes.at(paletteIndex).color(colorIndex))
    , _newColor(color)
{
    Q_ASSERT(_document);
    Q_ASSERT(_oldColor != _newColor);
}

void ChangePaletteColor::setNewColor(const Snes::SnesColor& color)
{
    _newColor = color;
}

void ChangePaletteColor::undo()
{
    doChangePaletteColor(_oldColor);
}

void ChangePaletteColor::redo()
{
    doChangePaletteColor(_newColor);
}

void ChangePaletteColor::doChangePaletteColor(const Snes::SnesColor& color)
{
    auto& palettes = _document->frameSet()->palettes;

    Q_ASSERT(_paletteIndex < palettes.size());
    Q_ASSERT(_colorIndex < 16);

    palettes.at(_paletteIndex).color(_colorIndex) = color;

    emit _document->paletteChanged(_paletteIndex);
}
