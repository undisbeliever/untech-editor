/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "msgraphicsscene.h"
#include "gui-qt/accessor/idmapundohelper.h"
#include "gui-qt/accessor/listandmultipleselectionundohelper.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/graphics/aabbgraphicsitem.h"
#include "gui-qt/common/graphics/pixmapgraphicsitem.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"
#include "gui-qt/snes/snescolordialog.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

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
        [](MS::FrameSet& fs) -> TilesetType& { return fs.tilesetType; },
        [](Document& d) { emit d.frameSetDataChanged(); });
}

bool Document::editFrameSet_setExportOrder(const UnTech::idstring& exportOrder)
{
    return FrameSetUndoHelper(this).editField(
        exportOrder,
        tr("Edit Export Order"),
        [](MS::FrameSet& fs) -> idstring& { return fs.exportOrder; },
        [](Document& d) { emit d.frameSetDataChanged();
                          emit d.frameSetExportOrderChanged(); });
}

using SmallTileTilesetUndoHelper = ListUndoHelper<SmallTileTileset>;

bool SmallTileTileset::editTileset_addTile()
{
    if (auto* tileset = getList()) {
        return editTileset_addTile(tileset->size());
    }
    return false;
}

bool SmallTileTileset::editTileset_addTile(unsigned index)
{
    QUndoCommand* c = SmallTileTilesetUndoHelper(this).addCommand(std::make_tuple(), index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Add Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editSelectedList_shiftTiles(ObjectSize::SMALL, index, 1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool SmallTileTileset::editTileset_cloneTile(unsigned index)
{
    QUndoCommand* c = SmallTileTilesetUndoHelper(this).cloneCommand(std::make_tuple(), index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Clone Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editSelectedList_shiftTiles(ObjectSize::SMALL, index, 1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool SmallTileTileset::editTileset_removeTile(unsigned index)
{
    QUndoCommand* c = SmallTileTilesetUndoHelper(this).removeCommand(std::make_tuple(), index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Remove Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editSelectedList_shiftTiles(ObjectSize::SMALL, index, -1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool SmallTileTileset::editTile_setPixel(unsigned tileId, const QPoint& p, unsigned c, bool first)
{
    c &= 15;

    if (auto* tileset = getList()) {
        if (tileId >= tileset->size()) {
            return false;
        }
        const auto& tile = tileset->tile(tileId);

        if (tile.pixel(p.x(), p.y()) != c) {
            auto newTile = tile;
            newTile.setPixel(p.x(), p.y(), c);

            return SmallTileTilesetUndoHelper(this).editItemInSelectedListMerge(tileId, newTile, first);
        }
    }

    return false;
}

bool SmallTileTileset::editTile_paintPixel(unsigned tileId, const QPoint& p, bool first)
{
    if (_document->paletteList()->isSelectedColorValid()) {
        return editTile_setPixel(tileId, p, _document->paletteList()->selectedColor(), first);
    }
    else {
        return false;
    }
}

using LargeTileTilesetUndoHelper = ListUndoHelper<LargeTileTileset>;

bool LargeTileTileset::editTileset_addTile()
{
    if (auto* tileset = getList()) {
        return editTileset_addTile(tileset->size());
    }
    return false;
}

bool LargeTileTileset::editTileset_addTile(unsigned index)
{
    QUndoCommand* c = LargeTileTilesetUndoHelper(this).addCommand(std::make_tuple(), index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Add Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editSelectedList_shiftTiles(ObjectSize::LARGE, index, 1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool LargeTileTileset::editTileset_cloneTile(unsigned index)
{
    QUndoCommand* c = LargeTileTilesetUndoHelper(this).cloneCommand(std::make_tuple(), index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Clone Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editSelectedList_shiftTiles(ObjectSize::LARGE, index, 1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool LargeTileTileset::editTileset_removeTile(unsigned index)
{
    QUndoCommand* c = LargeTileTilesetUndoHelper(this).removeCommand(std::make_tuple(), index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Remove Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editSelectedList_shiftTiles(ObjectSize::LARGE, index, -1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool LargeTileTileset::editTile_setPixel(unsigned tileId, const QPoint& p, unsigned c, bool first)
{
    c &= 15;

    if (auto* tileset = getList()) {
        if (tileId >= tileset->size()) {
            return false;
        }
        const auto& tile = tileset->tile(tileId);

        if (tile.pixel(p.x(), p.y()) != c) {
            auto newTile = tile;
            newTile.setPixel(p.x(), p.y(), c);

            return LargeTileTilesetUndoHelper(this).editItemInSelectedListMerge(tileId, newTile, first);
        }
    }

    return false;
}

bool LargeTileTileset::editTile_paintPixel(unsigned tileId, const QPoint& p, bool first)
{
    if (_document->paletteList()->isSelectedColorValid()) {
        return editTile_setPixel(tileId, p, _document->paletteList()->selectedColor(), first);
    }
    else {
        return false;
    }
}

using PaletteListUndoHelper = ListAndSelectionUndoHelper<PaletteList>;

void PaletteList::editSelected_setColorDialog(unsigned colorIndex, QWidget* widget)
{
    using namespace UnTech::Snes;
    using namespace UnTech::GuiQt::Snes;

    if (colorIndex >= Palette4bpp::N_COLORS) {
        return;
    }

    const Palette4bpp* palette = _document->paletteList()->selectedPalette();
    if (palette == nullptr) {
        return;
    }
    const SnesColor color = palette->color(colorIndex);

    PaletteListUndoHelper helper(_document->paletteList());
    auto command = helper.editSelectedFieldIncompleteCommand<SnesColor>(
        tr("Edit Palette Color"),
        [=](auto& pal) -> SnesColor& { return pal.color(colorIndex); });

    Q_ASSERT(command);

    SnesColorDialog dialog(widget);
    dialog.setColor(color);

    connect(&dialog, &SnesColorDialog::colorChanged,
            [&](auto& newColor) {
                command->setValue(newColor);
                command->redo();
            });

    dialog.exec();

    if (dialog.result() == QDialog::Accepted && command->hasValueChanged()) {
        _document->undoStack()->push(command.release());
    }
    else {
        command->undo();
    }
}

bool PaletteList::editSelectedList_addItem()
{
    return PaletteListUndoHelper(this).addItemToSelectedList();
}

bool PaletteList::editSelectedList_cloneSelected()
{
    return PaletteListUndoHelper(this).cloneSelectedItem();
}

bool PaletteList::editSelectedList_raiseSelected()
{
    return PaletteListUndoHelper(this).raiseSelectedItem();
}

bool PaletteList::editSelectedList_lowerSelected()
{
    return PaletteListUndoHelper(this).lowerSelectedItem();
}

bool PaletteList::editSelectedList_removeSelected()
{
    return PaletteListUndoHelper(this).removeSelectedItem();
}

using FrameMapUndoHelper = IdmapAndSelectionUndoHelper<FrameMap>;

bool FrameMap::editSelected_setSpriteOrder(SpriteOrderType spriteOrder)
{
    return FrameMapUndoHelper(this).editSelectedItemField(
        spriteOrder,
        tr("Edit Sprite Order"),
        [](MS::Frame& f) -> SpriteOrderType& { return f.spriteOrder; });
}

bool FrameMap::editSelected_setSolid(bool solid)
{
    return FrameMapUndoHelper(this).editSelectedItemField(
        solid,
        solid ? tr("Enable Tile Hitbox") : tr("Disable Tile Hitbox"),
        [](MS::Frame& f) -> bool& { return f.solid; });
}

bool FrameMap::editSelected_setTileHitbox(const UnTech::ms8rect& hitbox)
{
    return FrameMapUndoHelper(this).editSelectedItemField(
        hitbox,
        tr("Edit Tile Hitbox"),
        [](MS::Frame& f) -> ms8rect& { return f.tileHitbox; });
}

bool FrameMap::editSelected_toggleTileHitbox()
{
    const MS::Frame* frame = selectedFrame();
    if (frame) {
        return FrameMapUndoHelper(this).editSelectedItemField(
            !frame->solid,
            !frame->solid ? tr("Enable Tile Hitbox") : tr("Disable Tile Hitbox"),
            [](MS::Frame& f) -> bool& { return f.solid; });
    }
    return false;
}

using FrameObjectListUndoHelper = ListAndMultipleSelectionUndoHelper<FrameObjectList>;
template class UnTech::GuiQt::Accessor::ListAndMultipleSelectionUndoHelper<FrameObjectList>;
// Remember MsGraphicsScene::commitMovedItems

bool FrameObjectList::editSelectedList_setData(index_type index, const MS::FrameObject& data)
{
    return FrameObjectListUndoHelper(this).editItemInSelectedList(index, data);
}

bool FrameObjectList::editSelected_setTileIdAndSize(unsigned tileId, FrameObjectList::ObjectSize size)
{
    return FrameObjectListUndoHelper(this).editSelectedCommand(
        [&](MS::FrameObject& obj, size_t) {
            obj.tileId = tileId;
            obj.size = size;
        });
}

bool FrameObjectList::editSelected_toggleObjectSize()
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    return FrameObjectListUndoHelper(this).editSelectedItems(
        tr("Change Object Size"),
        [](MS::FrameObject& obj, size_t) {
            obj.size = (obj.size == ObjSize::SMALL) ? ObjSize::LARGE : ObjSize::SMALL;
        });
}

bool FrameObjectList::editSelected_flipObjectHorizontally()
{
    return FrameObjectListUndoHelper(this).editSelectedItems(
        tr("Flip Horizontally"),
        [](MS::FrameObject& obj, size_t) {
            obj.hFlip = !obj.hFlip;
        });
}

bool FrameObjectList::editSelected_flipObjectVertically()
{
    return FrameObjectListUndoHelper(this).editSelectedItems(
        tr("Flip Vertically"),
        [](MS::FrameObject& obj, size_t) {
            obj.vFlip = !obj.vFlip;
        });
}

inline bool FrameObjectList::editSelectedList_shiftTiles(ObjectSize size, unsigned tileId, int offset)
{
    return FrameObjectListUndoHelper(this).editAllItemsInSelectedList(
        QString(), [&](MS::FrameObject& obj, size_t) {
            if (obj.size == size && obj.tileId >= tileId) {
                obj.tileId += offset;
            }
        });
}

using ActionPointListUndoHelper = ListAndMultipleSelectionUndoHelper<ActionPointList>;
// Remember MsGraphicsScene::commitMovedItems

bool ActionPointList::editSelectedList_setData(index_type index, const MS::ActionPoint& data)
{
    return ActionPointListUndoHelper(this).editItemInSelectedList(index, data);
}

using EntityHitboxListUndoHelper = ListAndMultipleSelectionUndoHelper<EntityHitboxList>;
// Remember MsGraphicsScene::commitMovedItems

bool EntityHitboxList::editSelectedList_setData(index_type index, const MS::EntityHitbox& data)
{
    return EntityHitboxListUndoHelper(this).editItemInSelectedList(index, data);
}

bool EntityHitboxList::editSelected_setEntityHitboxType(EntityHitboxType type)
{
    return EntityHitboxListUndoHelper(this).setSelectedFields(
        type,
        tr("Change Entity Hitbox Type"),
        [](MS::EntityHitbox& eh) -> EntityHitboxType& { return eh.hitboxType; });
}

void MsGraphicsScene::commitMovedItems()
{
    if (_frame == nullptr) {
        return;
    }

    QList<QUndoCommand*> commands;
    commands.reserve(4);

    if (_document->frameMap()->isTileHitboxSelected()) {
        ms8rect hitbox = _tileHitbox->rectMs8rect();
        auto* c = FrameMapUndoHelper(_document->frameMap())
                      .editSelectedFieldCommand(hitbox, QString(),
                                                [](MS::Frame& f) -> ms8rect& { return f.tileHitbox; });
        if (c != nullptr) {
            commands.append(c);
        }
    }

    commands.append(
        FrameObjectListUndoHelper(_document->frameObjectList())
            .editSelectedCommand(
                [this](MS::FrameObject& obj, size_t i) {
                    obj.location = _objects.at(i)->posMs8point();
                }));
    commands.append(
        ActionPointListUndoHelper(_document->actionPointList())
            .editSelectedCommand(
                [this](MS::ActionPoint& ap, size_t i) {
                    ap.location = _actionPoints.at(i)->posMs8point();
                }));
    commands.append(
        EntityHitboxListUndoHelper(_document->entityHitboxList())
            .editSelectedCommand(
                [this](MS::EntityHitbox& eh, size_t i) {
                    eh.aabb = _entityHitboxes.at(i)->rectMs8rect();
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
