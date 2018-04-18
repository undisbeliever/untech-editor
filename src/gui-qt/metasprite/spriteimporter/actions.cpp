/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "actions.h"
#include "accessors.h"
#include "document.h"
#include "framelistmodel.h"
#include "mainwindow.h"
#include "gui-qt/common/idstringdialog.h"
#include "gui-qt/undo/idmapundohelper.h"
#include "gui-qt/undo/listactionhelper.h"
#include "gui-qt/undo/listandmultipleselectionundohelper.h"

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

Actions::Actions(MainWindow* mainWindow)
    : QObject(mainWindow)
    , _mainWindow(mainWindow)
    , _document(nullptr)
    , _addFrame(new QAction(QIcon(":/icons/add.svg"), tr("New Frame"), this))
    , _cloneFrame(new QAction(QIcon(":/icons/clone.svg"), tr("Clone Frame"), this))
    , _renameFrame(new QAction(QIcon(":/icons/rename.svg"), tr("Rename Frame"), this))
    , _removeFrame(new QAction(QIcon(":/icons/remove.svg"), tr("Remove Frame"), this))
    , _addRemoveTileHitbox(new QAction(tr("Add Tile Hitbox"), this))
    , _addFrameObject(new QAction(QIcon(":/icons/add-frame-object.svg"), tr("Add Frame Object"), this))
    , _addActionPoint(new QAction(QIcon(":/icons/add-action-point.svg"), tr("Add Action Point"), this))
    , _addEntityHitbox(new QAction(QIcon(":/icons/add-entity-hitbox.svg"), tr("Add Entity Hitbox"), this))
    , _raiseSelected(new QAction(QIcon(":/icons/raise.svg"), tr("Raise Selected"), this))
    , _lowerSelected(new QAction(QIcon(":/icons/lower.svg"), tr("Lower Selected"), this))
    , _cloneSelected(new QAction(QIcon(":/icons/clone.svg"), tr("Clone Selected"), this))
    , _removeSelected(new QAction(QIcon(":/icons/remove.svg"), tr("Remove Selected"), this))
    , _toggleObjSize(new QAction(QIcon(":/icons/toggle-obj-size.svg"), tr("Toggle Object Size"), this))
    , _entityHitboxTypeMenu(std::make_unique<QMenu>(tr("Set Entity Hitbox Type")))
{
    _raiseSelected->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
    _lowerSelected->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
    _cloneSelected->setShortcut(Qt::CTRL + Qt::Key_D);
    _removeSelected->setShortcut(Qt::Key_Delete);

    for (auto& it : UnTech::MetaSprite::EntityHitboxType::enumMap) {
        QString s = QString::fromStdString(it.first);
        _entityHitboxTypeMenu->addAction(s)->setData(int(it.second));
    }

    updateFrameActions();
    updateSelectionActions();

    connect(_addFrame, &QAction::triggered, this, &Actions::onAddFrame);
    connect(_cloneFrame, &QAction::triggered, this, &Actions::onCloneFrame);
    connect(_renameFrame, &QAction::triggered, this, &Actions::onRenameFrame);
    connect(_removeFrame, &QAction::triggered, this, &Actions::onRemoveFrame);
    connect(_addRemoveTileHitbox, &QAction::triggered, this, &Actions::onAddRemoveTileHitbox);
    connect(_addFrameObject, &QAction::triggered, this, &Actions::onAddFrameObject);
    connect(_addActionPoint, &QAction::triggered, this, &Actions::onAddActionPoint);
    connect(_addEntityHitbox, &QAction::triggered, this, &Actions::onAddEntityHitbox);
    connect(_raiseSelected, &QAction::triggered, this, &Actions::onRaiseSelected);
    connect(_lowerSelected, &QAction::triggered, this, &Actions::onLowerSelected);
    connect(_cloneSelected, &QAction::triggered, this, &Actions::onCloneSelected);
    connect(_removeSelected, &QAction::triggered, this, &Actions::onRemoveSelected);

    connect(_toggleObjSize, &QAction::triggered, this, &Actions::onToggleObjSize);
    connect(_entityHitboxTypeMenu.get(), &QMenu::triggered, this, &Actions::onEntityHitboxTypeMenu);
}

void Actions::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->frameMap()->disconnect(this);
        _document->frameObjectList()->disconnect(this);
        _document->actionPointList()->disconnect(this);
        _document->entityHitboxList()->disconnect(this);
    }
    _document = document;

    if (document) {
        connect(_document->frameMap(), &FrameMap::dataChanged,
                this, &Actions::updateFrameActions);
        connect(_document->frameMap(), &FrameMap::mapChanged,
                this, &Actions::updateFrameActions);
        connect(_document->frameMap(), &FrameMap::selectedItemChanged,
                this, &Actions::updateFrameActions);

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
    }

    updateFrameActions();
    updateSelectionActions();
}

void Actions::updateFrameActions()
{
    // ::TODO IdmapActionHelper::

    bool documentExists = false;
    bool frameSelected = false;

    if (_document) {
        documentExists = true;

        if (const SI::Frame* frame = _document->frameMap()->selectedFrame()) {
            frameSelected = true;

            if (frame->solid) {
                _addRemoveTileHitbox->setText(tr("Remove Tile Hitbox"));
            }
            else {
                _addRemoveTileHitbox->setText(tr("Add Tile Hitbox"));
            }
        }
    }

    _addRemoveTileHitbox->setEnabled(frameSelected);

    _addFrame->setEnabled(documentExists);
    _cloneFrame->setEnabled(frameSelected);
    _renameFrame->setEnabled(frameSelected);
    _removeFrame->setEnabled(frameSelected);
}

void Actions::updateSelectionActions()
{
    using namespace UnTech::GuiQt::Undo;

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

    _entityHitboxTypeMenu->setEnabled(eh.selectionValid);
}

void Actions::onAddFrame()
{
    idstring newId = IdstringDialog::getIdstring(
        _mainWindow,
        tr("Input Frame Name"),
        tr("Input name of the new frame:"),
        idstring(), _document->frameList());

    FrameMapUndoHelper(_document->frameMap()).addItem(newId);
}

void Actions::onCloneFrame()
{
    const idstring& frameId = _document->frameMap()->selectedId();

    idstring newId = IdstringDialog::getIdstring(
        _mainWindow,
        tr("Input Frame Name"),
        tr("Input name of the cloned frame:"),
        frameId, _document->frameList());

    FrameMapUndoHelper(_document->frameMap()).cloneItem(frameId, newId);
}

void Actions::onRenameFrame()
{
    const idstring& frameId = _document->frameMap()->selectedId();

    idstring newId = IdstringDialog::getIdstring(
        _mainWindow,
        tr("Input Frame Name"),
        tr("Rename %1 to:").arg(QString::fromStdString(frameId)),
        frameId, _document->frameList());

    FrameMapUndoHelper(_document->frameMap()).renameItem(frameId, newId);
}

void Actions::onRemoveFrame()
{
    FrameMapUndoHelper(_document->frameMap()).removeSelectedItem();
}

void Actions::onAddRemoveTileHitbox()
{
    const SI::Frame* frame = _document->frameMap()->selectedFrame();
    if (frame) {
        QString text = !frame->solid ? tr("Enable Tile Hitbox")
                                     : tr("Disable Tile Hitbox");

        FrameMapUndoHelper h(_document->frameMap());
        h.editSelectedItemField(!frame->solid, text,
                                [](SI::Frame& f) -> bool& { return f.solid; });
    }
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

    const SI::Frame* frame = _document->frameMap()->selectedFrame();
    Q_ASSERT(frame);

    FrameObjectListUndoHelper h(_document->frameObjectList());
    h.editSelectedItems(tr("Change Object Size"),
                        [&](SI::FrameObject& obj, size_t) {
                            obj.size = (obj.size == ObjSize::SMALL) ? ObjSize::LARGE : ObjSize::SMALL;
                        });
}

void Actions::onEntityHitboxTypeMenu(QAction* action)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;

    EHT ehType = EHT::Enum(action->data().toInt());

    EntityHitboxListUndoHelper h(_document->entityHitboxList());
    h.setSelectedFields(ehType, tr("Change Entity Hitbox Type"),
                        [](SI::EntityHitbox& eh) -> EHT& { return eh.hitboxType; });
}
