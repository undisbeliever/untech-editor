/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "sigraphicsscene.h"
#include "gui-qt/accessor/listandmultipleselectionundohelper.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

using ObjectSize = UnTech::MetaSprite::ObjectSize;

using FrameSetUndoHelper = ResourceItemUndoHelper<Document>;

bool Document::editFrameSet_setName(const idstring& name)
{
    if (name.isValid() == false) {
        return false;
    }

    return FrameSetUndoHelper(this).editName(name);
}

bool Document::editFrameSet_setTilesetType(TilesetType ts)
{
    return FrameSetUndoHelper(this).editField(
        ts,
        tr("Edit Tileset Type"),
        [](SI::FrameSet& fs) -> TilesetType& { return fs.tilesetType; },
        [](Document& d) { emit d.frameSetDataChanged(); });
}

bool Document::editFrameSet_setExportOrder(const UnTech::idstring& exportOrder)
{
    return FrameSetUndoHelper(this).editField(
        exportOrder,
        tr("Edit Export Order"),
        [](SI::FrameSet& fs) -> idstring& { return fs.exportOrder; },
        [](Document& d) { emit d.frameSetDataChanged();
                          emit d.frameSetExportOrderChanged(); });
}

bool Document::editFrameSet_setImageFilename(const std::string& filename)
{
    return FrameSetUndoHelper(this).editField(
        filename,
        tr("Change Image"),
        [](SI::FrameSet& fs) -> std::string& { return fs.imageFilename; },
        [](Document& d) {
            emit d.frameSetImageFilenameChanged();
            emit d.frameSetDataChanged();
        });
}

bool Document::editFrameSet_setTransparentColor(const UnTech::rgba& color)
{
    return FrameSetUndoHelper(this).editField(
        color,
        tr("Edit Transparent Color"),
        [](SI::FrameSet& fs) -> rgba& { return fs.transparentColor; },
        [](Document& d) { emit d.frameSetDataChanged(); });
}

bool Document::editFrameSet_setGrid(const SI::FrameSetGrid& grid)
{
    return FrameSetUndoHelper(this).editField(
        grid,
        tr("Edit FrameSet Grid"),
        [](SI::FrameSet& fs) -> SI::FrameSetGrid& { return fs.grid; },
        [](Document& d) { d.frameSet()->updateFrameLocations();
                          emit d.frameSetGridChanged();
                          emit d.frameSetDataChanged(); });
}

bool Document::editFrameSet_setPalette(const SI::UserSuppliedPalette& palette)
{
    return FrameSetUndoHelper(this).editField(
        palette,
        tr("Edit FrameSet Palette"),
        [](SI::FrameSet& fs) -> SI::UserSuppliedPalette& { return fs.palette; },
        [](Document& d) { emit d.frameSetPaletteChanged();
                          emit d.frameSetDataChanged(); });
}

using FrameListUndoHelper = ListAndSelectionUndoHelper<FrameList>;

bool FrameList::editSelected_setSpriteOrder(SpriteOrderType spriteOrder)
{
    return FrameListUndoHelper(this).editSelectedItemField(
        spriteOrder,
        tr("Edit Sprite Order"),
        [](SI::Frame& f) -> SpriteOrderType& { return f.spriteOrder; });
}

bool FrameList::editSelected_setFrameLocation(SI::FrameLocation& frameLocation)
{
    return FrameListUndoHelper(this).editSelectedItemField(
        frameLocation,
        tr("Edit Frame Location"),
        [](SI::Frame& f) -> SI::FrameLocation& { return f.location; },
        [](FrameList* frameList, const size_t i) { emit frameList->frameLocationChanged(i); });
}

bool FrameList::editSelected_setSolid(bool solid)
{
    return FrameListUndoHelper(this).editSelectedItemField(
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
    return FrameListUndoHelper(this).editSelectedItemMultipleFields(
        std::make_tuple(true, hitbox),
        tr("Edit Tile Hitbox"),
        [](SI::Frame& f) { return std::tie(f.solid, f.tileHitbox); });
}

using FrameObjectListUndoHelper = ListAndMultipleSelectionUndoHelper<FrameObjectList>;
template class UnTech::GuiQt::Accessor::ListAndMultipleSelectionUndoHelper<FrameObjectList>;
// Remember SiGraphicsScene::commitMovedItems

bool FrameObjectList::editSelectedList_setLocation(unsigned index, const upoint& location)
{
    return FrameObjectListUndoHelper(this).editFieldInSelectedList(
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

    return FrameObjectListUndoHelper(this).editItemInSelectedList(
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

    return FrameObjectListUndoHelper(this).editSelectedItems(
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

using ActionPointListUndoHelper = ListAndMultipleSelectionUndoHelper<ActionPointList>;
// Remember SiGraphicsScene::commitMovedItems

bool ActionPointList::editSelectedList_setLocation(unsigned index, const upoint& location)
{
    return ActionPointListUndoHelper(this).editFieldInSelectedList(
        index, location,
        tr("Edit Action Point Location"),
        [](SI::ActionPoint& ap) -> upoint& { return ap.location; });
}

bool ActionPointList::editSelectedList_setParameter(unsigned index, ActionPointList::ParameterType parameter)
{
    return ActionPointListUndoHelper(this).editFieldInSelectedList(
        index, parameter,
        tr("Edit Action Point Parameter"),
        [](SI::ActionPoint& ap) -> ParameterType& { return ap.parameter; });
}

using EntityHitboxListUndoHelper = ListAndMultipleSelectionUndoHelper<EntityHitboxList>;
// Remember SiGraphicsScene::commitMovedItems

bool EntityHitboxList::editSelectedList_setAabb(unsigned index, const urect& aabb)
{
    return EntityHitboxListUndoHelper(this).editFieldInSelectedList(
        index, aabb,
        tr("Edit Entity Hitbox AABB"),
        [](SI::EntityHitbox& eh) -> urect& { return eh.aabb; });
}

bool EntityHitboxList::editSelectedList_setEntityHitboxType(unsigned index, EntityHitboxList::EntityHitboxType type)
{
    return EntityHitboxListUndoHelper(this).editFieldInSelectedList(
        index, type,
        tr("Edit Entity Hitbox Type"),
        [](SI::EntityHitbox& eh) -> EntityHitboxType& { return eh.hitboxType; });
}

bool EntityHitboxList::editSelected_setEntityHitboxType(EntityHitboxType type)
{
    return EntityHitboxListUndoHelper(this).setSelectedFields(
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
        auto* c = FrameListUndoHelper(_document->frameList())
                      .editSelectedItemFieldCommand(hitbox, QString(),
                                                    [](SI::Frame& f) -> urect& { return f.tileHitbox; });
        if (c != nullptr) {
            commands.append(c);
        }
    }

    commands.append(
        FrameObjectListUndoHelper(_document->frameObjectList())
            .editSelectedCommand(
                [&](SI::FrameObject& obj, size_t i) {
                    obj.location = objects.at(i)->posUpoint();
                }));
    commands.append(
        ActionPointListUndoHelper(_document->actionPointList())
            .editSelectedCommand(
                [&](SI::ActionPoint& ap, size_t i) {
                    ap.location = actionPoints.at(i)->posUpoint();
                }));
    commands.append(
        EntityHitboxListUndoHelper(_document->entityHitboxList())
            .editSelectedCommand(
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
