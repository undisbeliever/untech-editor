/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "msgraphicsscene.h"
#include "gui-qt/accessor/listundohelper.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/graphics/aabbgraphicsitem.h"
#include "gui-qt/common/graphics/pixmapgraphicsitem.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"
#include "gui-qt/snes/snescolordialog.h"

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

using ObjectSize = UnTech::MetaSprite::ObjectSize;

bool Document::editFrameSet_setName(const idstring& name)
{
    if (name.isValid() == false) {
        return false;
    }

    return UndoHelper(this).editName(name);
}

bool Document::editFrameSet_setTilesetType(TilesetType ts)
{
    return UndoHelper(this).editField(
        ts,
        tr("Edit Tileset Type"),
        [](MS::FrameSet& fs) -> TilesetType& { return fs.tilesetType; },
        [](Document& d) { emit d.frameSetDataChanged(); });
}

bool Document::editFrameSet_setExportOrder(const UnTech::idstring& exportOrder)
{
    return UndoHelper(this).editField(
        exportOrder,
        tr("Edit Export Order"),
        [](MS::FrameSet& fs) -> idstring& { return fs.exportOrder; },
        [](Document& d) { emit d.frameSetDataChanged();
                          emit d.frameSetExportOrderChanged(); });
}

bool SmallTileTileset::addItem()
{
    if (auto* tileset = getList()) {
        return addItem(tileset->size());
    }
    return false;
}

bool SmallTileTileset::addItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).addCommand(index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Add Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editAll_shiftTileIds(ObjectSize::SMALL, index, 1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool SmallTileTileset::cloneItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).cloneCommand(index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Clone Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editAll_shiftTileIds(ObjectSize::SMALL, index, 1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool SmallTileTileset::removeItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).removeCommand(index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Remove Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editAll_shiftTileIds(ObjectSize::SMALL, index, -1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool SmallTileTileset::moveItem(size_t from, size_t to)
{
    // ::SHOULDDO adjust tileIds when a tile is moved::
    return UndoHelper(this).moveItem(from, to);
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

            return UndoHelper(this).editMerge(tileId, newTile, first);
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

bool LargeTileTileset::addItem()
{
    if (auto* tileset = getList()) {
        return addItem(tileset->size());
    }
    return false;
}

bool LargeTileTileset::addItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).addCommand(index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Add Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editAll_shiftTileIds(ObjectSize::LARGE, index, 1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool LargeTileTileset::cloneItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).cloneCommand(index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Clone Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editAll_shiftTileIds(ObjectSize::LARGE, index, 1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool LargeTileTileset::removeItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).removeCommand(index);
    if (c) {
        _document->undoStack()->beginMacro(tr("Remove Small Tile"));
        _document->undoStack()->push(c);
        _document->frameObjectList()->editAll_shiftTileIds(ObjectSize::LARGE, index, -1);
        _document->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool LargeTileTileset::moveItem(size_t from, size_t to)
{
    // ::SHOULDDO adjust tileIds when a tile is moved::
    return UndoHelper(this).moveItem(from, to);
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

            return UndoHelper(this).editMerge(tileId, newTile, first);
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

void PaletteList::editSelected_setColorDialog(unsigned colorIndex, QWidget* widget)
{
    using namespace UnTech::Snes;
    using namespace UnTech::GuiQt::Snes;

    if (colorIndex >= Palette4bpp::N_COLORS) {
        return;
    }

    const Palette4bpp* palette = selectedItem();
    if (palette == nullptr) {
        return;
    }
    const SnesColor color = palette->color(colorIndex);

    UndoHelper helper(this);
    auto command = helper.editSelectedItemFieldIncompleteCommand<SnesColor>(
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
        resourceItem()->undoStack()->push(command.release());
    }
    else {
        command->undo();
    }
}

bool PaletteList::editSelectedList_addItem()
{
    return UndoHelper(this).addItem();
}

bool PaletteList::editSelectedList_cloneSelected()
{
    return UndoHelper(this).cloneItem(selectedIndex());
}

bool PaletteList::editSelectedList_raiseSelected()
{
    return UndoHelper(this).raiseSelectedItem();
}

bool PaletteList::editSelectedList_lowerSelected()
{
    return UndoHelper(this).lowerSelectedItem();
}

bool PaletteList::editSelectedList_removeSelected()
{
    return UndoHelper(this).removeItem(selectedIndex());
}

bool FrameList::editSelected_setSpriteOrder(SpriteOrderType spriteOrder)
{
    return UndoHelper(this).editSelectedItemField(
        spriteOrder,
        tr("Edit Sprite Order"),
        [](MS::Frame& f) -> SpriteOrderType& { return f.spriteOrder; });
}

bool FrameList::editSelected_setSolid(bool solid)
{
    return UndoHelper(this).editSelectedItemField(
        solid,
        solid ? tr("Enable Tile Hitbox") : tr("Disable Tile Hitbox"),
        [](MS::Frame& f) -> bool& { return f.solid; });
}

bool FrameList::editSelected_toggleTileHitbox()
{
    if (const MS::Frame* frame = selectedItem()) {
        return editSelected_setSolid(!frame->solid);
    }
    return false;
}

bool FrameList::editSelected_setTileHitbox(const ms8rect& hitbox)
{
    return UndoHelper(this).editSelectedItemMultipleFields(
        std::make_tuple(true, hitbox),
        tr("Edit Tile Hitbox"),
        [](MS::Frame& f) { return std::tie(f.solid, f.tileHitbox); });
}

bool FrameObjectList::editSelectedList_setLocation(unsigned index, const ms8point& location)
{
    return UndoHelper(this).editField(
        index, location,
        tr("Edit Object Location"),
        [](MS::FrameObject& obj) -> ms8point& { return obj.location; });
}

bool FrameObjectList::editSelectedList_setSize(unsigned index, FrameObjectList::ObjectSize size)
{
    return UndoHelper(this).editField(
        index, size,
        tr("Edit Object Size"),
        [](MS::FrameObject& obj) -> ObjectSize& { return obj.size; });
}

bool FrameObjectList::editSelectedList_setTile(unsigned index, unsigned tileId)
{
    return UndoHelper(this).editField(
        index, tileId,
        tr("Edit Object Tile"),
        [](MS::FrameObject& obj) -> unsigned& { return obj.tileId; });
}

bool FrameObjectList::editSelectedList_setFlips(unsigned index, bool hFlip, bool vFlip)
{
    return UndoHelper(this).editField(
        index, std::make_tuple(hFlip, vFlip),
        tr("Edit Object Flip"),
        [](MS::FrameObject & obj) -> auto { return std::tie(obj.hFlip, obj.vFlip); });
}

bool FrameObjectList::editSelected_setTileIdAndSize(unsigned tileId, FrameObjectList::ObjectSize size)
{
    return UndoHelper(this).editSelectedItems(
        tr("Edit Frame Object"),
        [&](MS::FrameObject& obj, size_t) {
            obj.tileId = tileId;
            obj.size = size;
        });
}

bool FrameObjectList::editSelected_toggleObjectSize()
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    return UndoHelper(this).editSelectedItems(
        tr("Change Object Size"),
        [](MS::FrameObject& obj, size_t) {
            obj.size = (obj.size == ObjSize::SMALL) ? ObjSize::LARGE : ObjSize::SMALL;
        });
}

bool FrameObjectList::editSelected_flipObjectHorizontally()
{
    return UndoHelper(this).editSelectedItems(
        tr("Flip Horizontally"),
        [](MS::FrameObject& obj, size_t) {
            obj.hFlip = !obj.hFlip;
        });
}

bool FrameObjectList::editSelected_flipObjectVertically()
{
    return UndoHelper(this).editSelectedItems(
        tr("Flip Vertically"),
        [](MS::FrameObject& obj, size_t) {
            obj.vFlip = !obj.vFlip;
        });
}

inline bool FrameObjectList::editAll_shiftTileIds(ObjectSize size, unsigned tileId, int offset)
{
    const MS::FrameSet* fs = resourceItem()->frameSet();

    if (fs == nullptr) {
        return false;
    }

    QList<QUndoCommand*> commands;
    commands.reserve(fs->frames.size());

    for (size_t fIndex = 0; fIndex < fs->frames.size(); fIndex++) {
        QUndoCommand* cmd = UndoHelper(this).editAllItemsCommand(
            QString(),
            [&](MS::FrameObject& obj, size_t) {
                if (obj.size == size && obj.tileId >= tileId) {
                    if (offset > 0 || obj.tileId > 0) {
                        obj.tileId += offset;
                    }
                }
            });

        if (cmd != nullptr) {
            commands.append(cmd);
        }
    }

    if (!commands.empty()) {
        auto* undoStack = resourceItem()->undoStack();

        undoStack->beginMacro(tr("Shift TileIds"));
        for (QUndoCommand* c : commands) {
            undoStack->push(c);
        }
        undoStack->endMacro();
    }

    return commands.empty() == false;
}

bool ActionPointList::editSelectedList_setLocation(unsigned index, const ms8point& location)
{
    return UndoHelper(this).editField(
        index, location,
        tr("Edit Action Point Location"),
        [](MS::ActionPoint& ap) -> ms8point& { return ap.location; });
}

bool ActionPointList::editSelectedList_setParameter(unsigned index, ParameterType parameter)
{
    return UndoHelper(this).editField(
        index, parameter,
        tr("Edit Action Point Parameter"),
        [](MS::ActionPoint& ap) -> ParameterType& { return ap.parameter; });
}

bool EntityHitboxList::editSelectedList_setAabb(unsigned index, const ms8rect& aabb)
{
    return UndoHelper(this).editField(
        index, aabb,
        tr("Edit Entity Hitbox AABB"),
        [](MS::EntityHitbox& eh) -> ms8rect& { return eh.aabb; });
}

bool EntityHitboxList::editSelectedList_setEntityHitboxType(unsigned index, EntityHitboxType type)
{
    return UndoHelper(this).editField(
        index, type,
        tr("Edit Entity Hitbox Type"),
        [](MS::EntityHitbox& eh) -> EntityHitboxType& { return eh.hitboxType; });
}

bool EntityHitboxList::editSelected_setEntityHitboxType(EntityHitboxType type)
{
    return UndoHelper(this).setFieldInSelectedItems(
        type,
        tr("Change Entity Hitbox Type"),
        [](MS::EntityHitbox& eh) -> EntityHitboxType& { return eh.hitboxType; });
}

void MsGraphicsScene::commitMovedItems()
{
    auto* frame = selectedFrame();
    if (frame == nullptr) {
        return;
    }

    QList<QUndoCommand*> commands;
    commands.reserve(4);

    if (_document->frameList()->isTileHitboxSelected()) {
        ms8rect hitbox = _tileHitbox->rectMs8rect();
        auto* c = FrameList::UndoHelper(_document->frameList())
                      .editSelectedItemFieldCommand(hitbox, QString(),
                                                    [](MS::Frame& f) -> ms8rect& { return f.tileHitbox; });
        if (c != nullptr) {
            commands.append(c);
        }
    }

    commands.append(
        FrameObjectList::UndoHelper(_document->frameObjectList())
            .editSelectedItemsCommand(
                QString(),
                [this](MS::FrameObject& obj, size_t i) {
                    obj.location = _objects.at(i)->posMs8point();
                }));
    commands.append(
        ActionPointList::UndoHelper(_document->actionPointList())
            .editSelectedItemsCommand(
                QString(),
                [this](MS::ActionPoint& ap, size_t i) {
                    ap.location = _actionPoints.at(i)->posMs8point();
                }));
    commands.append(
        EntityHitboxList::UndoHelper(_document->entityHitboxList())
            .editSelectedItemsCommand(
                QString(),
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
