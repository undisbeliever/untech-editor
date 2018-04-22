/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "actions.h"
#include "accessors.h"
#include "document.h"
#include "mainwindow.h"
#include "gui-qt/accessor/idmapundohelper.h"
#include "gui-qt/accessor/listactionhelper.h"
#include "gui-qt/accessor/listandmultipleselectionundohelper.h"
#include "gui-qt/accessor/listundohelper.h"
#include "gui-qt/common/idstringdialog.h"

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

Actions::Actions(MainWindow* mainWindow)
    : QObject(mainWindow)
    , _mainWindow(mainWindow)
    , _document(nullptr)
    , _addRemoveTileHitbox(new QAction(tr("Add Tile Hitbox"), this))
    , _addPalette(new QAction(QIcon(":/icons/add.svg"), tr("New Palette"), this))
    , _clonePalette(new QAction(QIcon(":/icons/clone.svg"), tr("Clone Palette"), this))
    , _raisePalette(new QAction(QIcon(":/icons/raise.svg"), tr("Raise Palette"), this))
    , _lowerPalette(new QAction(QIcon(":/icons/lower.svg"), tr("Lower Palette"), this))
    , _removePalette(new QAction(QIcon(":/icons/remove.svg"), tr("Remove Palette"), this))
    , _addFrameObject(new QAction(QIcon(":/icons/add-frame-object.svg"), tr("Add Frame Object"), this))
    , _addActionPoint(new QAction(QIcon(":/icons/add-action-point.svg"), tr("Add Action Point"), this))
    , _addEntityHitbox(new QAction(QIcon(":/icons/add-entity-hitbox.svg"), tr("Add Entity Hitbox"), this))
    , _raiseSelected(new QAction(QIcon(":/icons/raise.svg"), tr("Raise Selected"), this))
    , _lowerSelected(new QAction(QIcon(":/icons/lower.svg"), tr("Lower Selected"), this))
    , _cloneSelected(new QAction(QIcon(":/icons/clone.svg"), tr("Clone Selected"), this))
    , _removeSelected(new QAction(QIcon(":/icons/remove.svg"), tr("Remove Selected"), this))
    , _toggleObjSize(new QAction(QIcon(":/icons/toggle-obj-size.svg"), tr("Toggle Object Size"), this))
    , _flipObjHorizontally(new QAction(QIcon(":/icons/flip-horizontally.svg"), tr("Flip Object Horizontally"), this))
    , _flipObjVertically(new QAction(QIcon(":/icons/flip-vertically.svg"), tr("Flip Object Vertically"), this))
    , _entityHitboxTypeMenu(std::make_unique<QMenu>(tr("Set Entity Hitbox Type")))
{
    for (auto& it : UnTech::MetaSprite::EntityHitboxType::enumMap) {
        QString s = QString::fromStdString(it.first);
        _entityHitboxTypeMenu->addAction(s)->setData(int(it.second));
    }

    _raiseSelected->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
    _lowerSelected->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
    _cloneSelected->setShortcut(Qt::CTRL + Qt::Key_D);
    _removeSelected->setShortcut(Qt::Key_Delete);

    updateSelectionActions();
    updatePaletteActions();

    connect(_addRemoveTileHitbox, &QAction::triggered, this, &Actions::onAddRemoveTileHitbox);

    connect(_addPalette, &QAction::triggered, this, &Actions::onAddPalette);
    connect(_clonePalette, &QAction::triggered, this, &Actions::onClonePalette);
    connect(_removePalette, &QAction::triggered, this, &Actions::onRemovePalette);
    connect(_raisePalette, &QAction::triggered, this, &Actions::onRaisePalette);
    connect(_lowerPalette, &QAction::triggered, this, &Actions::onLowerPalette);

    connect(_addFrameObject, &QAction::triggered, this, &Actions::onAddFrameObject);
    connect(_addActionPoint, &QAction::triggered, this, &Actions::onAddActionPoint);
    connect(_addEntityHitbox, &QAction::triggered, this, &Actions::onAddEntityHitbox);
    connect(_raiseSelected, &QAction::triggered, this, &Actions::onRaiseSelected);
    connect(_lowerSelected, &QAction::triggered, this, &Actions::onLowerSelected);
    connect(_cloneSelected, &QAction::triggered, this, &Actions::onCloneSelected);
    connect(_removeSelected, &QAction::triggered, this, &Actions::onRemoveSelected);

    connect(_toggleObjSize, &QAction::triggered, this, &Actions::onToggleObjSize);
    connect(_flipObjHorizontally, &QAction::triggered, this, &Actions::onFlipObjHorizontally);
    connect(_flipObjVertically, &QAction::triggered, this, &Actions::onFlipObjVertically);

    connect(_entityHitboxTypeMenu.get(), &QMenu::triggered, this, &Actions::onEntityHitboxTypeMenu);
}

void Actions::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->paletteList()->disconnect(this);
        _document->frameMap()->disconnect(this);
        _document->frameObjectList()->disconnect(this);
        _document->actionPointList()->disconnect(this);
        _document->entityHitboxList()->disconnect(this);
    }
    _document = document;

    if (document) {
        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->frameObjectList(), &FrameObjectList::listChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->actionPointList(), &ActionPointList::listChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->entityHitboxList(), &EntityHitboxList::listChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->actionPointList(), &ActionPointList::selectedIndexesChanged,
                this, &Actions::updateSelectionActions);
        connect(_document->entityHitboxList(), &EntityHitboxList::selectedIndexesChanged,
                this, &Actions::updateSelectionActions);

        connect(_document->paletteList(), &PaletteList::selectedIndexChanged,
                this, &Actions::updatePaletteActions);
        connect(_document->paletteList(), &PaletteList::listChanged,
                this, &Actions::updatePaletteActions);
    }

    updateSelectionActions();
    updatePaletteActions();
}

void Actions::updateSelectionActions()
{
    using namespace UnTech::GuiQt::Accessor;

    ListActionStatus obj;
    ListActionStatus ap;
    ListActionStatus eh;

    if (_document) {
        obj = ListActionHelper::status(_document->frameObjectList());
        ap = ListActionHelper::status(_document->actionPointList());
        eh = ListActionHelper::status(_document->entityHitboxList());
    }

    ListActionStatus selected(obj, ap, eh);

    _addFrameObject->setEnabled(obj.canAdd);
    _addActionPoint->setEnabled(ap.canAdd);
    _addEntityHitbox->setEnabled(eh.canAdd);

    _raiseSelected->setEnabled(selected.canRaise);
    _lowerSelected->setEnabled(selected.canLower);
    _cloneSelected->setEnabled(selected.canClone);
    _removeSelected->setEnabled(selected.canRemove);

    _toggleObjSize->setEnabled(obj.selectionValid);
    _flipObjVertically->setEnabled(obj.selectionValid);
    _flipObjHorizontally->setEnabled(obj.selectionValid);

    _entityHitboxTypeMenu->setEnabled(eh.selectionValid);
}

void Actions::updatePaletteActions()
{
    using namespace UnTech::GuiQt::Accessor;

    ListActionStatus status;

    if (_document) {
        status = ListActionHelper::status(_document->paletteList());
    }

    _addPalette->setEnabled(status.canAdd);
    _clonePalette->setEnabled(status.canClone);
    _removePalette->setEnabled(status.canRemove);
    _raisePalette->setEnabled(status.canRaise);
    _lowerPalette->setEnabled(status.canLower);
}

void Actions::onAddRemoveTileHitbox()
{
    const MS::Frame* frame = _document->frameMap()->selectedFrame();
    if (frame) {
        QString text = !frame->solid ? tr("Enable Tile Hitbox")
                                     : tr("Disable Tile Hitbox");

        FrameMapUndoHelper h(_document->frameMap());
        h.editSelectedItemField(!frame->solid, text,
                                [](MS::Frame& f) -> bool& { return f.solid; });
    }
}

void Actions::onAddPalette()
{
    PaletteListUndoHelper(_document->paletteList()).addItemToSelectedList();
}

void Actions::onClonePalette()
{
    PaletteListUndoHelper(_document->paletteList()).cloneSelectedItem();
}

void Actions::onRemovePalette()
{
    PaletteListUndoHelper(_document->paletteList()).removeSelectedItem();
}

void Actions::onRaisePalette()
{
    PaletteListUndoHelper(_document->paletteList()).raiseSelectedItem();
}

void Actions::onLowerPalette()
{
    PaletteListUndoHelper(_document->paletteList()).lowerSelectedItem();
}

void Actions::onAddFrameObject()
{
    FrameObjectListUndoHelper(_document->frameObjectList()).addItemToSelectedList();
}

void Actions::onAddActionPoint()
{
    ActionPointListUndoHelper(_document->actionPointList()).addItemToSelectedList();
}

void Actions::onAddEntityHitbox()
{
    EntityHitboxListUndoHelper(_document->entityHitboxList()).addItemToSelectedList();
}

void Actions::onRaiseSelected()
{
    // ::TODO helper class that combines these and checks a command exists::
    _document->undoStack()->beginMacro(tr("Raise Selected"));

    FrameObjectListUndoHelper(_document->frameObjectList()).raiseSelectedItems();
    ActionPointListUndoHelper(_document->actionPointList()).raiseSelectedItems();
    EntityHitboxListUndoHelper(_document->entityHitboxList()).raiseSelectedItems();

    _document->undoStack()->endMacro();
}

void Actions::onLowerSelected()
{
    // ::TODO helper class::
    _document->undoStack()->beginMacro(tr("Lower Selected"));

    FrameObjectListUndoHelper(_document->frameObjectList()).lowerSelectedItems();
    ActionPointListUndoHelper(_document->actionPointList()).lowerSelectedItems();
    EntityHitboxListUndoHelper(_document->entityHitboxList()).lowerSelectedItems();

    _document->undoStack()->endMacro();
}

void Actions::onCloneSelected()
{
    // ::TODO helper class::
    _document->undoStack()->beginMacro(tr("Clone Selected"));

    FrameObjectListUndoHelper(_document->frameObjectList()).cloneSelectedItems();
    ActionPointListUndoHelper(_document->actionPointList()).cloneSelectedItems();
    EntityHitboxListUndoHelper(_document->entityHitboxList()).cloneSelectedItems();

    _document->undoStack()->endMacro();
}

void Actions::onRemoveSelected()
{
    // ::TODO helper class::
    _document->undoStack()->beginMacro(tr("Remove Selected"));

    FrameObjectListUndoHelper(_document->frameObjectList()).removeSelectedItems();
    ActionPointListUndoHelper(_document->actionPointList()).removeSelectedItems();
    EntityHitboxListUndoHelper(_document->entityHitboxList()).removeSelectedItems();

    _document->undoStack()->endMacro();
}

void Actions::onToggleObjSize()
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    FrameObjectListUndoHelper h(_document->frameObjectList());
    h.editSelectedItems(tr("Change Object Size"),
                        [](MS::FrameObject& obj, size_t) {
                            obj.size = (obj.size == ObjSize::SMALL) ? ObjSize::LARGE : ObjSize::SMALL;
                        });
}

void Actions::onFlipObjHorizontally()
{
    FrameObjectListUndoHelper h(_document->frameObjectList());
    h.editSelectedItems(tr("Flip Horizontally"),
                        [](MS::FrameObject& obj, size_t) {
                            obj.hFlip = !obj.hFlip;
                        });
}

void Actions::onFlipObjVertically()
{
    FrameObjectListUndoHelper h(_document->frameObjectList());
    h.editSelectedItems(tr("Flip Vertically"),
                        [](MS::FrameObject& obj, size_t) {
                            obj.vFlip = !obj.vFlip;
                        });
}

void Actions::onEntityHitboxTypeMenu(QAction* action)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;

    EHT ehType = EHT::Enum(action->data().toInt());

    EntityHitboxListUndoHelper h(_document->entityHitboxList());
    h.setSelectedFields(ehType, tr("Change Entity Hitbox Type"),
                        [](MS::EntityHitbox& eh) -> EHT& { return eh.hitboxType; });
}
