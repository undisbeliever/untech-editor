/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "sigraphicsscene.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

using ObjectSize = UnTech::MetaSprite::ObjectSize;

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

SI::Frame* FrameList::selectedItemEditable()
{
    auto* frames = getList();
    if (selectedIndex() >= frames->size()) {
        return nullptr;
    }
    return &frames->at(selectedIndex());
}

bool FrameList::editSelected_setSpriteOrder(SpriteOrderType spriteOrder)
{
    return UndoHelper(this).editSelectedItemField(
        spriteOrder,
        tr("Edit Sprite Order"),
        [](SI::Frame& f) -> SpriteOrderType& { return f.spriteOrder; });
}

bool FrameList::editSelected_setFrameLocation(SI::FrameLocation& frameLocation)
{
    return UndoHelper(this).editSelectedItemField(
        frameLocation,
        tr("Edit Frame Location"),
        [](SI::Frame& f) -> SI::FrameLocation& { return f.location; },
        [](FrameList* frameList, const size_t i) { emit frameList->frameLocationChanged(i); });
}

bool FrameList::editSelected_setSolid(bool solid)
{
    return UndoHelper(this).editSelectedItemField(
        solid,
        solid ? tr("Enable Tile Hitbox") : tr("Disable Tile Hitbox"),
        [](SI::Frame& f) -> bool& { return f.solid; });
}

bool FrameList::editSelected_toggleTileHitbox()
{
    if (const SI::Frame* frame = selectedItem()) {
        return editSelected_setSolid(!frame->solid);
    }
    else {
        return false;
    }
}

bool FrameList::editSelected_setTileHitbox(const urect& hitbox)
{
    return UndoHelper(this).editSelectedItemMultipleFields(
        std::make_tuple(true, hitbox),
        tr("Edit Tile Hitbox"),
        [](SI::Frame& f) { return std::tie(f.solid, f.tileHitbox); });
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

bool FrameObjectList::editSelectedList_setLocation(unsigned index, const upoint& location)
{
    return UndoHelper(this).editField(
        index, location,
        tr("Edit Object Location"),
        [](SI::FrameObject& obj) -> upoint& { return obj.location; });
}

bool FrameObjectList::editSelectedList_setSize(unsigned index, FrameObjectList::ObjectSize size)
{
    const SI::Frame* frame = resourceItem()->frameList()->selectedItem();
    if (frame == nullptr) {
        return false;
    }

    return UndoHelper(this).editItem(
        index,
        tr("Edit Object Size"),
        [&](SI::FrameObject& obj) {
            obj.size = size;

            if (obj.bottomRight().x >= frame->location.aabb.width) {
                obj.location.x = frame->location.aabb.width - obj.sizePx();
            }
            if (obj.bottomRight().y >= frame->location.aabb.height) {
                obj.location.y = frame->location.aabb.height - obj.sizePx();
            }
        });
}

bool FrameObjectList::editSelected_toggleObjectSize()
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    const SI::Frame* frame = resourceItem()->frameList()->selectedItem();
    Q_ASSERT(frame);

    return UndoHelper(this).editSelectedItems(
        tr("Change Object Size"),
        [&](SI::FrameObject& obj, size_t) {
            obj.size = (obj.size == ObjSize::SMALL) ? ObjSize::LARGE : ObjSize::SMALL;

            if (obj.bottomRight().x >= frame->location.aabb.width) {
                obj.location.x = frame->location.aabb.width - obj.sizePx();
            }
            if (obj.bottomRight().y >= frame->location.aabb.height) {
                obj.location.y = frame->location.aabb.height - obj.sizePx();
            }
        });
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

bool ActionPointList::editSelectedList_setLocation(unsigned index, const upoint& location)
{
    return UndoHelper(this).editField(
        index, location,
        tr("Edit Action Point Location"),
        [](SI::ActionPoint& ap) -> upoint& { return ap.location; });
}

bool ActionPointList::editSelectedList_setType(unsigned index, idstring type)
{
    return UndoHelper(this).editField(
        index, type,
        tr("Edit Action Point Type"),
        [](SI::ActionPoint& ap) -> idstring& { return ap.type; });
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

bool EntityHitboxList::editSelectedList_setAabb(unsigned index, const urect& aabb)
{
    return UndoHelper(this).editField(
        index, aabb,
        tr("Edit Entity Hitbox AABB"),
        [](SI::EntityHitbox& eh) -> urect& { return eh.aabb; });
}

bool EntityHitboxList::editSelectedList_setEntityHitboxType(unsigned index, EntityHitboxList::EntityHitboxType type)
{
    return UndoHelper(this).editField(
        index, type,
        tr("Edit Entity Hitbox Type"),
        [](SI::EntityHitbox& eh) -> EntityHitboxType& { return eh.hitboxType; });
}

bool EntityHitboxList::editSelected_setEntityHitboxType(EntityHitboxType type)
{
    return UndoHelper(this).setFieldInSelectedItems(
        type,
        tr("Change Entity Hitbox Type"),
        [](SI::EntityHitbox& eh) -> EntityHitboxType& { return eh.hitboxType; });
}

void SiGraphicsScene::commitMovedItems()
{
    const size_t frameIndex = _document->frameList()->selectedIndex();
    const SiFrameGraphicsItem* frameItem = _frameItems.value(frameIndex);
    if (frameItem == nullptr) {
        return;
    }

    const auto& objects = frameItem->objects();
    const auto& actionPoints = frameItem->actionPoints();
    const auto& entityHitboxes = frameItem->entityHitboxes();

    QList<QUndoCommand*> commands;
    commands.reserve(4);

    if (_document->frameList()->isTileHitboxSelected()) {
        urect hitbox = frameItem->tileHitbox()->rectUrect();
        auto* c = FrameList::UndoHelper(_document->frameList())
                      .editSelectedItemFieldCommand(hitbox, QString(),
                                                    [](SI::Frame& f) -> urect& { return f.tileHitbox; });
        if (c != nullptr) {
            commands.append(c);
        }
    }

    commands.append(
        FrameObjectList::UndoHelper(_document->frameObjectList())
            .editSelectedItemsCommand(
                QString(),
                [&](SI::FrameObject& obj, size_t i) {
                    obj.location = objects.at(i)->posUpoint();
                }));
    commands.append(
        ActionPointList::UndoHelper(_document->actionPointList())
            .editSelectedItemsCommand(
                QString(),
                [&](SI::ActionPoint& ap, size_t i) {
                    ap.location = actionPoints.at(i)->posUpoint();
                }));
    commands.append(
        EntityHitboxList::UndoHelper(_document->entityHitboxList())
            .editSelectedItemsCommand(
                QString(),
                [&](SI::EntityHitbox& eh, size_t i) {
                    eh.aabb = entityHitboxes.at(i)->rectUrect();
                }));

    commands.removeAll(nullptr);

    if (!commands.empty()) {
        _document->undoStack()->beginMacro(tr("Move Selected"));
        for (QUndoCommand* c : commands) {
            _document->undoStack()->push(c);
        }
        _document->undoStack()->endMacro();
    }
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<SI::Frame, Document>;
template class Accessor::ChildVectorMultipleSelectionAccessor<SI::FrameObject, Document>;
template class Accessor::ChildVectorMultipleSelectionAccessor<SI::ActionPoint, Document>;
template class Accessor::ChildVectorMultipleSelectionAccessor<SI::EntityHitbox, Document>;
