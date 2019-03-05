/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "gui-qt/accessor/multipleselectedindexeshelper.h"
#include "gui-qt/accessor/selectedindexhelper.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

SmallTileTileset::SmallTileTileset(Document* document)
    : AbstractListAccessor(document, 255)
    , _document(document)
{
}

QString SmallTileTileset::typeName() const
{
    return tr("Small Tile");
}

QString SmallTileTileset::typeNamePlural() const
{
    return tr("Small Tiles");
}

bool SmallTileTileset::listExists() const
{
    return resourceItem()->frameSet() != nullptr;
}

size_t SmallTileTileset::size() const
{
    MS::FrameSet* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return 0;
    }
    return fs->smallTileset.size();
}

LargeTileTileset::LargeTileTileset(Document* document)
    : AbstractListAccessor(document, 255)
    , _document(document)
{
}

bool LargeTileTileset::listExists() const
{
    return resourceItem()->frameSet() != nullptr;
}

size_t LargeTileTileset::size() const
{
    MS::FrameSet* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return 0;
    }
    return fs->smallTileset.size();
}

QString LargeTileTileset::typeName() const
{
    return tr("Large Tile");
}

QString LargeTileTileset::typeNamePlural() const
{
    return tr("Large Tiles");
}

template <>
const std::vector<Snes::Palette4bpp>* VectorSingleSelectionAccessor<Snes::Palette4bpp, Document>::list() const
{
    const MS::FrameSet* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    return &fs->palettes;
}

template <>
std::vector<Snes::Palette4bpp>* VectorSingleSelectionAccessor<Snes::Palette4bpp, Document>::getList()
{
    MS::FrameSet* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    return &fs->palettes;
}

PaletteList::PaletteList(Document* document)
    : VectorSingleSelectionAccessor(document, UnTech::MetaSprite::MAX_PALETTES)
    , _selectedColor(INT_MAX)
{
    setSelectedIndex(0);
}

QString PaletteList::typeName() const
{
    return tr("Palette");
}

QString PaletteList::typeNamePlural() const
{
    return tr("Palettes");
}

void PaletteList::setSelectedColor(unsigned color)
{
    if (_selectedColor != color) {
        _selectedColor = color;
        emit selectedColorChanged();
    }
}

bool PaletteList::isSelectedColorValid() const
{
    return isSelectedIndexValid() && _selectedColor < 16;
}

template <>
const NamedList<MS::Frame>* NamedListAccessor<MS::Frame, Document>::list() const
{
    if (const MS::FrameSet* fs = resourceItem()->frameSet()) {
        return &fs->frames;
    }
    return nullptr;
}

template <>
NamedList<MS::Frame>* NamedListAccessor<MS::Frame, Document>::getList()
{
    if (MS::FrameSet* fs = resourceItem()->frameSet()) {
        return &fs->frames;
    }
    return nullptr;
}

FrameList::FrameList(Document* document)
    : NamedListAccessor(document, UnTech::MetaSprite::MAX_EXPORT_NAMES)
    , _tileHitboxSelected(false)
{
}

QString FrameList::typeName() const
{
    return tr("Frame");
}

QString FrameList::typeNamePlural() const
{
    return tr("Frames");
}

void FrameList::setTileHitboxSelected(bool s)
{
    if (_tileHitboxSelected != s) {
        _tileHitboxSelected = s;
        emit tileHitboxSelectedChanged();
    }
}

template <>
const std::vector<MS::FrameObject>* ChildVectorMultipleSelectionAccessor<MS::FrameObject, Document>::list(size_t pIndex) const
{
    const auto* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    if (pIndex >= fs->frames.size()) {
        return nullptr;
    }
    return &fs->frames.at(pIndex).objects;
}

template <>
std::vector<MS::FrameObject>* ChildVectorMultipleSelectionAccessor<MS::FrameObject, Document>::getList(size_t pIndex)
{
    auto* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    if (pIndex >= fs->frames.size()) {
        return nullptr;
    }
    return &fs->frames.at(pIndex).objects;
}

FrameObjectList::FrameObjectList(Document* document)
    : ChildVectorMultipleSelectionAccessor(document->frameList(), document, UnTech::MetaSprite::MAX_FRAME_OBJECTS)
{
}

QString FrameObjectList::typeName() const
{
    return tr("Frame Object");
}

QString FrameObjectList::typeNamePlural() const
{
    return tr("Frame Objects");
}

template <>
const std::vector<MS::ActionPoint>* ChildVectorMultipleSelectionAccessor<MS::ActionPoint, Document>::list(size_t pIndex) const
{
    const auto* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    if (pIndex >= fs->frames.size()) {
        return nullptr;
    }
    return &fs->frames.at(pIndex).actionPoints;
}

template <>
std::vector<MS::ActionPoint>* ChildVectorMultipleSelectionAccessor<MS::ActionPoint, Document>::getList(size_t pIndex)
{
    auto* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    if (pIndex >= fs->frames.size()) {
        return nullptr;
    }
    return &fs->frames.at(pIndex).actionPoints;
}

ActionPointList::ActionPointList(Document* document)
    : ChildVectorMultipleSelectionAccessor(document->frameList(), document, UnTech::MetaSprite::MAX_ACTION_POINTS)
{
}

QString ActionPointList::typeName() const
{
    return tr("Action Point");
}

QString ActionPointList::typeNamePlural() const
{
    return tr("Action Points");
}

template <>
const std::vector<MS::EntityHitbox>* ChildVectorMultipleSelectionAccessor<MS::EntityHitbox, Document>::list(size_t pIndex) const
{
    const auto* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    if (pIndex >= fs->frames.size()) {
        return nullptr;
    }
    return &fs->frames.at(pIndex).entityHitboxes;
}

template <>
std::vector<MS::EntityHitbox>* ChildVectorMultipleSelectionAccessor<MS::EntityHitbox, Document>::getList(size_t pIndex)
{
    auto* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    if (pIndex >= fs->frames.size()) {
        return nullptr;
    }
    return &fs->frames.at(pIndex).entityHitboxes;
}

EntityHitboxList::EntityHitboxList(Document* document)
    : ChildVectorMultipleSelectionAccessor(document->frameList(), document, UnTech::MetaSprite::MAX_ENTITY_HITBOXES)
{
}

QString EntityHitboxList::typeName() const
{
    return tr("Entity Hitbox");
}

QString EntityHitboxList::typeNamePlural() const
{
    return tr("Entity Hitboxes");
}

using namespace UnTech::GuiQt;
template class Accessor::VectorSingleSelectionAccessor<Snes::Palette4bpp, Document>;
template class Accessor::NamedListAccessor<MS::Frame, Document>;
template class Accessor::ChildVectorMultipleSelectionAccessor<MS::FrameObject, Document>;
template class Accessor::ChildVectorMultipleSelectionAccessor<MS::ActionPoint, Document>;
template class Accessor::ChildVectorMultipleSelectionAccessor<MS::EntityHitbox, Document>;
