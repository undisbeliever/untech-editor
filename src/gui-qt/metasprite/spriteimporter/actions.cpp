/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "actions.h"
#include "document.h"
#include "framecommands.h"
#include "framecontentcommands.h"
#include "framelistmodel.h"
#include "mainwindow.h"
#include "selection.h"
#include "gui-qt/common/idstringdialog.h"

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

Actions::Actions(MainWindow* mainWindow)
    : QObject(mainWindow)
    , _mainWindow(mainWindow)
    , _document(nullptr)
{
    _addFrame = new QAction(tr("New Frame"), this);
    _cloneFrame = new QAction(tr("Clone Frame"), this);
    _renameFrame = new QAction(tr("Rename Frame"), this);
    _removeFrame = new QAction(tr("Remove Frame"), this);

    _addRemoveTileHitbox = new QAction(tr("Add Tile Hitbox"), this);

    _addFrameObject = new QAction(tr("Add Frame Object"), this);
    _addActionPoint = new QAction(tr("Add Action Point"), this);
    _addEntityHitbox = new QAction(tr("Add Entity Hitbox"), this);

    _raiseSelected = new QAction(tr("Raise Selected"), this);
    _lowerSelected = new QAction(tr("Lower Selected"), this);
    _cloneSelected = new QAction(tr("Clone Selected"), this);
    _removeSelected = new QAction(tr("Remove Selected"), this);

    _raiseSelected->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
    _lowerSelected->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
    _cloneSelected->setShortcut(Qt::CTRL + Qt::Key_D);
    _removeSelected->setShortcut(Qt::Key_Delete);

    updateActions();

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
}

void Actions::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    if (document) {
        connect(_document, &Document::frameDataChanged,
                this, &Actions::updateActions);
        connect(_document->selection(), &Selection::selectedFrameChanged,
                this, &Actions::updateActions);
        connect(_document->selection(), &Selection::selectedItemsChanged,
                this, &Actions::updateActions);
    }

    updateActions();
}

void Actions::updateActions()
{
    bool documentExists = false;
    bool frameSelected = false;
    bool canAddFrameObject = false;
    bool canAddActionPoint = false;
    bool canAddEntityHitbox = false;
    bool canRaiseSelected = false;
    bool canLowerSelected = false;
    bool canCloneSelected = false;
    bool canRemoveSelected = false;

    if (_document) {
        documentExists = true;

        if (const SI::Frame* frame = _document->selection()->selectedFrame()) {
            frameSelected = true;

            if (frame->solid) {
                _addRemoveTileHitbox->setText(tr("Remove Tile Hitbox"));
            }
            else {
                _addRemoveTileHitbox->setText(tr("Add Tile Hitbox"));
            }
            canAddFrameObject = frame->objects.can_insert();
            canAddActionPoint = frame->actionPoints.can_insert();
            canAddEntityHitbox = frame->entityHitboxes.can_insert();
        }

        canRaiseSelected = _document->selection()->canRaiseSelectedItems();
        canLowerSelected = _document->selection()->canLowerSelectedItems();
        canCloneSelected = _document->selection()->canCloneSelectedItems();
        canRemoveSelected = _document->selection()->selectedItems().size() > 0;
    }

    _addFrame->setEnabled(documentExists);
    _cloneFrame->setEnabled(frameSelected);
    _renameFrame->setEnabled(frameSelected);
    _removeFrame->setEnabled(frameSelected);
    _addRemoveTileHitbox->setEnabled(frameSelected);
    _addFrameObject->setEnabled(canAddFrameObject);
    _addActionPoint->setEnabled(canAddActionPoint);
    _addEntityHitbox->setEnabled(canAddEntityHitbox);
    _raiseSelected->setEnabled(canRaiseSelected);
    _lowerSelected->setEnabled(canLowerSelected);
    _cloneSelected->setEnabled(canCloneSelected);
    _removeSelected->setEnabled(canRemoveSelected);
}

void Actions::onAddFrame()
{
    const SI::FrameSet& fs = *_document->frameSet();

    idstring newId = IdstringDialog::getIdstring(
        _mainWindow,
        tr("Input Frame Name"),
        tr("Input name of the new frame:"),
        idstring(), _document->frameListModel());

    if (newId.isValid() && !fs.frames.contains(newId)) {
        _document->undoStack()->push(
            new AddFrame(_document, newId));

        _document->selection()->selectFrame(newId);
    }
}

void Actions::onCloneFrame()
{
    const SI::FrameSet& fs = *_document->frameSet();
    const idstring& frameId = _document->selection()->selectedFrameId();

    idstring newId = IdstringDialog::getIdstring(
        _mainWindow,
        tr("Input Frame Name"),
        tr("Input name of the cloned frame:"),
        frameId, _document->frameListModel());

    if (newId != frameId && newId.isValid() && !fs.frames.contains(newId)) {
        _document->undoStack()->push(
            new CloneFrame(_document, frameId, newId));

        _document->selection()->selectFrame(newId);
    }
}

void Actions::onRenameFrame()
{
    const SI::FrameSet& fs = *_document->frameSet();
    const idstring& frameId = _document->selection()->selectedFrameId();

    idstring newId = IdstringDialog::getIdstring(
        _mainWindow,
        tr("Input Frame Name"),
        tr("Rename %1 to:").arg(QString::fromStdString(frameId)),
        frameId, _document->frameListModel());

    if (newId != frameId && newId.isValid() && !fs.frames.contains(newId)) {
        _document->undoStack()->push(
            new RenameFrame(_document, frameId, newId));
    }
}

void Actions::onRemoveFrame()
{
    idstring frameId = _document->selection()->selectedFrameId();

    _document->undoStack()->push(
        new RemoveFrame(_document, frameId));
}

void Actions::onAddFrameObject()
{
    SI::Frame* frame = _document->selection()->selectedFrame();

    _document->undoStack()->push(
        new AddFrameObject(_document, frame));

    _document->selection()->selectFrameObject(frame->objects.size() - 1);
}

void Actions::onAddRemoveTileHitbox()
{
    SI::Frame* frame = _document->selection()->selectedFrame();
    if (frame) {
        _document->undoStack()->push(
            new ChangeFrameSolid(_document, frame, !frame->solid));
    }
}

void Actions::onAddActionPoint()
{
    SI::Frame* frame = _document->selection()->selectedFrame();

    _document->undoStack()->push(
        new AddActionPoint(_document, frame));

    _document->selection()->selectActionPoint(frame->actionPoints.size() - 1);
}

void Actions::onAddEntityHitbox()
{
    SI::Frame* frame = _document->selection()->selectedFrame();

    _document->undoStack()->push(
        new AddEntityHitbox(_document, frame));

    _document->selection()->selectEntityHitbox(frame->entityHitboxes.size() - 1);
}

void Actions::onRaiseSelected()
{
    SI::Frame* frame = _document->selection()->selectedFrame();
    const auto& selectedItems = _document->selection()->selectedItems();

    _document->undoStack()->push(
        new RaiseFrameContents(_document, frame, selectedItems));
}

void Actions::onLowerSelected()
{
    SI::Frame* frame = _document->selection()->selectedFrame();
    const auto& selectedItems = _document->selection()->selectedItems();

    _document->undoStack()->push(
        new LowerFrameContents(_document, frame, selectedItems));
}

void Actions::onCloneSelected()
{
    QUndoStack* undoStack = _document->undoStack();
    SI::Frame* frame = _document->selection()->selectedFrame();
    const std::set<SelectedItem> items = _document->selection()->selectedItems();
    std::set<SelectedItem> newSel;

    if (items.size() > 1) {
        undoStack->beginMacro(tr("Clone"));
    }

    for (const auto& item : items) {
        switch (item.type) {
        case SelectedItem::NONE:
        case SelectedItem::TILE_HITBOX:
            break;

        case SelectedItem::FRAME_OBJECT:
            undoStack->push(new CloneFrameObject(_document, frame, item.index));
            newSel.insert({ SelectedItem::FRAME_OBJECT, frame->objects.size() - 1 });
            break;

        case SelectedItem::ACTION_POINT:
            undoStack->push(new CloneActionPoint(_document, frame, item.index));
            newSel.insert({ SelectedItem::ACTION_POINT, frame->actionPoints.size() - 1 });
            break;

        case SelectedItem::ENTITY_HITBOX:
            undoStack->push(new CloneEntityHitbox(_document, frame, item.index));
            newSel.insert({ SelectedItem::ENTITY_HITBOX, frame->entityHitboxes.size() - 1 });
            break;
        }
    }

    if (items.size() > 1) {
        undoStack->endMacro();
    }

    _document->selection()->setSelectedItems(newSel);
}

void Actions::onRemoveSelected()
{
    QUndoStack* undoStack = _document->undoStack();
    SI::Frame* frame = _document->selection()->selectedFrame();

    const auto& itemsSet = _document->selection()->selectedItems();
    const std::vector<SelectedItem> items(itemsSet.rbegin(), itemsSet.rend());

    if (items.size() > 1) {
        undoStack->beginMacro(tr("Remove Items"));
    }

    for (const auto& item : items) {
        switch (item.type) {
        case SelectedItem::NONE:
        case SelectedItem::TILE_HITBOX:
            break;

        case SelectedItem::FRAME_OBJECT:
            undoStack->push(new RemoveFrameObject(_document, frame, item.index));
            break;

        case SelectedItem::ACTION_POINT:
            undoStack->push(new RemoveActionPoint(_document, frame, item.index));
            break;

        case SelectedItem::ENTITY_HITBOX:
            undoStack->push(new RemoveEntityHitbox(_document, frame, item.index));
            break;
        }
    }

    if (items.size() > 1) {
        undoStack->endMacro();
    }
}
