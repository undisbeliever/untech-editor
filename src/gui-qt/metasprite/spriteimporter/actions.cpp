/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "actions.h"
#include "document.h"
#include "framecommands.h"
#include "mainwindow.h"
#include "selection.h"

#include <QInputDialog>
#include <QMessageBox>

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

    updateActions();

    connect(_addFrame, SIGNAL(triggered()), this, SLOT(onAddFrame()));
    connect(_cloneFrame, SIGNAL(triggered()), this, SLOT(onCloneFrame()));
    connect(_renameFrame, SIGNAL(triggered()), this, SLOT(onRenameFrame()));
    connect(_removeFrame, SIGNAL(triggered()), this, SLOT(onRemoveFrame()));
}

void Actions::setDocument(Document* document)
{
    if (_document) {
        _document->selection()->disconnect(this);
    }
    _document = document;

    if (document) {
        connect(_document->selection(), SIGNAL(selectedFrameChanged()), this, SLOT(updateActions()));
    }

    updateActions();
}

void Actions::updateActions()
{
    bool documentExists = false;
    bool frameSelected = false;

    if (_document) {
        documentExists = true;
        frameSelected = _document->selection()->selectedFrame() != nullptr;
    }

    _addFrame->setEnabled(documentExists);
    _cloneFrame->setEnabled(frameSelected);
    _renameFrame->setEnabled(frameSelected);
    _removeFrame->setEnabled(frameSelected);
}

void Actions::onAddFrame()
{
    const SI::FrameSet& fs = *_document->frameSet();

    QString text = QInputDialog::getText(
        _mainWindow, tr("Input Frame Name"), tr("Input name of the new frame:"));
    idstring newId(text.toStdString());

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

    QString text = QInputDialog::getText(
        _mainWindow, tr("Input Frame Name"), tr("Input name of the cloned frame:"));
    idstring newId(text.toStdString());

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

    QString text = QInputDialog::getText(
        _mainWindow, tr("Input Frame Name"), tr("Rename frame to:"));
    idstring newId(text.toStdString());

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
