/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "msgraphicsscene.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "gui-qt/common/graphics/aabbgraphicsitem.h"
#include "gui-qt/common/graphics/pixmapgraphicsitem.h"
#include "gui-qt/common/graphics/resizableaabbgraphicsitem.h"
#include "gui-qt/snes/snescolordialog.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

using ObjectSize = UnTech::MetaSprite::ObjectSize;

SmallTileTileset::SmallTileTileset(ResourceItem* resourceItem)
    : AbstractListAccessor(resourceItem, 255)
    , _resourceItem(resourceItem)
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
        _resourceItem->undoStack()->beginMacro(tr("Add Small Tile"));
        _resourceItem->undoStack()->push(c);
        _resourceItem->frameObjectList()->editAll_shiftTileIds(ObjectSize::SMALL, index, 1);
        _resourceItem->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool SmallTileTileset::cloneItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).cloneCommand(index);
    if (c) {
        _resourceItem->undoStack()->beginMacro(tr("Clone Small Tile"));
        _resourceItem->undoStack()->push(c);
        _resourceItem->frameObjectList()->editAll_shiftTileIds(ObjectSize::SMALL, index, 1);
        _resourceItem->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool SmallTileTileset::removeItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).removeCommand(index);
    if (c) {
        _resourceItem->undoStack()->beginMacro(tr("Remove Small Tile"));
        _resourceItem->undoStack()->push(c);
        _resourceItem->frameObjectList()->editAll_shiftTileIds(ObjectSize::SMALL, index, -1);
        _resourceItem->undoStack()->endMacro();
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
    if (_resourceItem->paletteList()->isSelectedColorValid()) {
        return editTile_setPixel(tileId, p, _resourceItem->paletteList()->selectedColor(), first);
    }
    else {
        return false;
    }
}

LargeTileTileset::LargeTileTileset(ResourceItem* resourceItem)
    : AbstractListAccessor(resourceItem, 255)
    , _resourceItem(resourceItem)
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
        _resourceItem->undoStack()->beginMacro(tr("Add Small Tile"));
        _resourceItem->undoStack()->push(c);
        _resourceItem->frameObjectList()->editAll_shiftTileIds(ObjectSize::LARGE, index, 1);
        _resourceItem->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool LargeTileTileset::cloneItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).cloneCommand(index);
    if (c) {
        _resourceItem->undoStack()->beginMacro(tr("Clone Small Tile"));
        _resourceItem->undoStack()->push(c);
        _resourceItem->frameObjectList()->editAll_shiftTileIds(ObjectSize::LARGE, index, 1);
        _resourceItem->undoStack()->endMacro();
    }
    return c != nullptr;
}

bool LargeTileTileset::removeItem(size_t index)
{
    QUndoCommand* c = UndoHelper(this).removeCommand(index);
    if (c) {
        _resourceItem->undoStack()->beginMacro(tr("Remove Small Tile"));
        _resourceItem->undoStack()->push(c);
        _resourceItem->frameObjectList()->editAll_shiftTileIds(ObjectSize::LARGE, index, -1);
        _resourceItem->undoStack()->endMacro();
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
    if (_resourceItem->paletteList()->isSelectedColorValid()) {
        return editTile_setPixel(tileId, p, _resourceItem->paletteList()->selectedColor(), first);
    }
    else {
        return false;
    }
}

template <>
const std::vector<Snes::Palette4bpp>* VectorSingleSelectionAccessor<Snes::Palette4bpp, ResourceItem>::list() const
{
    const MS::FrameSet* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    return &fs->palettes;
}

template <>
std::vector<Snes::Palette4bpp>* VectorSingleSelectionAccessor<Snes::Palette4bpp, ResourceItem>::getList()
{
    MS::FrameSet* fs = resourceItem()->frameSet();
    if (fs == nullptr) {
        return nullptr;
    }
    return &fs->palettes;
}

PaletteList::PaletteList(ResourceItem* resourceItem)
    : VectorSingleSelectionAccessor(resourceItem, UnTech::MetaSprite::MAX_PALETTES)
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

template <>
const NamedList<MS::Frame>* NamedListAccessor<MS::Frame, ResourceItem>::list() const
{
    if (const MS::FrameSet* fs = resourceItem()->frameSet()) {
        return &fs->frames;
    }
    return nullptr;
}

template <>
NamedList<MS::Frame>* NamedListAccessor<MS::Frame, ResourceItem>::getList()
{
    if (MS::FrameSet* fs = resourceItem()->frameSet()) {
        return &fs->frames;
    }
    return nullptr;
}

FrameList::FrameList(ResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, UnTech::MetaSprite::MAX_EXPORT_NAMES)
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

template <>
const std::vector<MS::FrameObject>* ChildVectorMultipleSelectionAccessor<MS::FrameObject, ResourceItem>::list(size_t pIndex) const
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
std::vector<MS::FrameObject>* ChildVectorMultipleSelectionAccessor<MS::FrameObject, ResourceItem>::getList(size_t pIndex)
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

FrameObjectList::FrameObjectList(ResourceItem* resourceItem)
    : ChildVectorMultipleSelectionAccessor(resourceItem->frameList(), resourceItem, UnTech::MetaSprite::MAX_FRAME_OBJECTS)
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

template <>
const std::vector<MS::ActionPoint>* ChildVectorMultipleSelectionAccessor<MS::ActionPoint, ResourceItem>::list(size_t pIndex) const
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
std::vector<MS::ActionPoint>* ChildVectorMultipleSelectionAccessor<MS::ActionPoint, ResourceItem>::getList(size_t pIndex)
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

ActionPointList::ActionPointList(ResourceItem* resourceItem)
    : ChildVectorMultipleSelectionAccessor(resourceItem->frameList(), resourceItem, UnTech::MetaSprite::MAX_ACTION_POINTS)
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

bool ActionPointList::editSelectedList_setLocation(unsigned index, const ms8point& location)
{
    return UndoHelper(this).editField(
        index, location,
        tr("Edit Action Point Location"),
        [](MS::ActionPoint& ap) -> ms8point& { return ap.location; });
}

bool ActionPointList::editSelectedList_setType(unsigned index, idstring type)
{
    return UndoHelper(this).editField(
        index, type,
        tr("Edit Action Point Type"),
        [](MS::ActionPoint& ap) -> idstring& { return ap.type; });
}

template <>
const std::vector<MS::EntityHitbox>* ChildVectorMultipleSelectionAccessor<MS::EntityHitbox, ResourceItem>::list(size_t pIndex) const
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
std::vector<MS::EntityHitbox>* ChildVectorMultipleSelectionAccessor<MS::EntityHitbox, ResourceItem>::getList(size_t pIndex)
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

EntityHitboxList::EntityHitboxList(ResourceItem* resourceItem)
    : ChildVectorMultipleSelectionAccessor(resourceItem->frameList(), resourceItem, UnTech::MetaSprite::MAX_ENTITY_HITBOXES)
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

    if (_resourceItem->frameList()->isTileHitboxSelected()) {
        ms8rect hitbox = _tileHitbox->rectMs8rect();
        auto* c = FrameList::UndoHelper(_resourceItem->frameList())
                      .editSelectedItemFieldCommand(hitbox, QString(),
                                                    [](MS::Frame& f) -> ms8rect& { return f.tileHitbox; });
        if (c != nullptr) {
            commands.append(c);
        }
    }

    commands.append(
        FrameObjectList::UndoHelper(_resourceItem->frameObjectList())
            .editSelectedItemsCommand(
                QString(),
                [this](MS::FrameObject& obj, size_t i) {
                    obj.location = _objects.at(i)->posMs8point();
                }));
    commands.append(
        ActionPointList::UndoHelper(_resourceItem->actionPointList())
            .editSelectedItemsCommand(
                QString(),
                [this](MS::ActionPoint& ap, size_t i) {
                    ap.location = _actionPoints.at(i)->posMs8point();
                }));
    commands.append(
        EntityHitboxList::UndoHelper(_resourceItem->entityHitboxList())
            .editSelectedItemsCommand(
                QString(),
                [this](MS::EntityHitbox& eh, size_t i) {
                    eh.aabb = _entityHitboxes.at(i)->rectMs8rect();
                }));

    commands.removeAll(nullptr);

    if (!commands.empty()) {
        _resourceItem->undoStack()->beginMacro(tr("Move Selected"));
        for (QUndoCommand* c : commands) {
            _resourceItem->undoStack()->push(c);
        }
        _resourceItem->undoStack()->endMacro();
    }
}

using namespace UnTech::GuiQt;
template class Accessor::VectorSingleSelectionAccessor<UnTech::Snes::Palette4bpp, ResourceItem>;
template class Accessor::NamedListAccessor<MS::Frame, ResourceItem>;
template class Accessor::ChildVectorMultipleSelectionAccessor<MS::FrameObject, ResourceItem>;
template class Accessor::ChildVectorMultipleSelectionAccessor<MS::ActionPoint, ResourceItem>;
template class Accessor::ChildVectorMultipleSelectionAccessor<MS::EntityHitbox, ResourceItem>;
