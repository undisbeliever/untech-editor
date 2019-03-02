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
    : QObject(document)
    , _document(document)
{
}

LargeTileTileset::LargeTileTileset(Document* document)
    : QObject(document)
    , _document(document)
{
}

PaletteList::PaletteList(Document* document)
    : QObject(document)
    , _document(document)
    , _selectedIndex(0)
    , _selectedColor(INT_MAX)
{
    SelectedIndexHelper::buildAndConnectSlots(this);
}

void PaletteList::setSelectedIndex(PaletteList::index_type index)
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

bool PaletteList::isSelectedIndexValid() const
{
    const MS::FrameSet* fs = _document->frameSet();
    if (fs == nullptr) {
        return false;
    }
    return _selectedIndex < fs->palettes.size();
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
    return isSelectedIndexValid()
           && _selectedColor < 16;
}

const PaletteList::ListT* PaletteList::palettes() const
{
    MS::FrameSet* fs = _document->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    return &fs->palettes;
}

const PaletteList::DataT* PaletteList::selectedPalette() const
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
