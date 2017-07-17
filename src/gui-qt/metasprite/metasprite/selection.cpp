/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "selection.h"
#include "document.h"
#include <algorithm>

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

Selection::Selection(QObject* parent)
    : AbstractSelection(parent)
    , _document(nullptr)
    , _selectedFrame(nullptr)
    , _selectedPalette(0)
{
}

void Selection::setDocument(Document* document)
{
    AbstractSelection::setDocument(document);

    if (_document != nullptr) {
        _document->disconnect(this);
    }
    _document = document;

    connect(_document, &Document::paletteAdded,
            this, &Selection::onPaletteAdded);
    connect(_document, &Document::paletteAboutToBeRemoved,
            this, &Selection::onPaletteAboutToBeRemoved);
    connect(_document, &Document::paletteMoved,
            this, &Selection::onPaletteMoved);
}

const void* Selection::setSelectedFrame(const idstring& id)
{
    if (_document) {
        _selectedFrame = _document->frameSet()->frames.getPtr(id);
    }
    else {
        _selectedFrame = nullptr;
    }
    return _selectedFrame;
}

unsigned Selection::nObjectsInSelectedFrame() const
{
    if (_selectedFrame) {
        return _selectedFrame->objects.size();
    }
    return 0;
}

unsigned Selection::nActionPointsInSelectedFrame() const
{
    if (_selectedFrame) {
        return _selectedFrame->actionPoints.size();
    }
    return 0;
}

unsigned Selection::nEntityHitboxesInSelectedFrame() const
{
    if (_selectedFrame) {
        return _selectedFrame->entityHitboxes.size();
    }
    return 0;
}

void Selection::selectPalette(unsigned index)
{
    if (_document == nullptr
        || index >= _document->frameSet()->palettes.size()) {
        index = 0;
    }

    if (_selectedPalette != index) {
        _selectedPalette = index;
        emit selectedPaletteChanged();
    }
}

void Selection::onPaletteAdded(unsigned)
{
    if (_document->frameSet()->palettes.size() == 1) {
        _selectedPalette = 0;
        emit selectedPaletteChanged();
    }
}

void Selection::onPaletteAboutToBeRemoved(unsigned index)
{
    if (_selectedPalette == index) {
        _selectedPalette = 0;
        emit selectedPaletteChanged();
    }
}

void Selection::onPaletteMoved(unsigned oldPos, unsigned newPos)
{
    if (_selectedPalette == oldPos) {
        _selectedPalette = newPos;
        emit selectedPaletteChanged();
    }
    else if (_selectedPalette == newPos) {
        _selectedPalette = oldPos;
        emit selectedPaletteChanged();
    }
}
