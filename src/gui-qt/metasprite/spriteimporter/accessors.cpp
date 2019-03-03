/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "gui-qt/accessor/multipleselectedindexeshelper.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

template <>
const NamedList<SI::Frame>* NamedListAccessor<SI::Frame, Document>::list() const
{
    if (const SI::FrameSet* fs = resourceItem()->frameSet()) {
        return &fs->frames;
    }
    return nullptr;
}

template <>
NamedList<SI::Frame>* NamedListAccessor<SI::Frame, Document>::getList()
{
    if (SI::FrameSet* fs = resourceItem()->frameSet()) {
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

SI::Frame* FrameList::selectedItemEditable()
{
    auto* frames = getList();
    if (selectedIndex() >= frames->size()) {
        return nullptr;
    }
    return &frames->at(selectedIndex());
}

template <>
const std::vector<SI::FrameObject>* ChildVectorMultipleSelectionAccessor<SI::FrameObject, Document>::list(size_t pIndex) const
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
std::vector<SI::FrameObject>* ChildVectorMultipleSelectionAccessor<SI::FrameObject, Document>::getList(size_t pIndex)
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
const std::vector<SI::ActionPoint>* ChildVectorMultipleSelectionAccessor<SI::ActionPoint, Document>::list(size_t pIndex) const
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
std::vector<SI::ActionPoint>* ChildVectorMultipleSelectionAccessor<SI::ActionPoint, Document>::getList(size_t pIndex)
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
const std::vector<SI::EntityHitbox>* ChildVectorMultipleSelectionAccessor<SI::EntityHitbox, Document>::list(size_t pIndex) const
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
std::vector<SI::EntityHitbox>* ChildVectorMultipleSelectionAccessor<SI::EntityHitbox, Document>::getList(size_t pIndex)
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
template class Accessor::NamedListAccessor<SI::Frame, Document>;
template class Accessor::ChildVectorMultipleSelectionAccessor<SI::FrameObject, Document>;
template class Accessor::ChildVectorMultipleSelectionAccessor<SI::ActionPoint, Document>;
template class Accessor::ChildVectorMultipleSelectionAccessor<SI::EntityHitbox, Document>;
