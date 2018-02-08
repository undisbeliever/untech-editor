/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecommands.h"
#include "document.h"
#include "framelistmodel.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

// AddRemoveFrame
// ==============

AddRemoveFrame::AddRemoveFrame(Document* document,
                               const idstring& frameId, std::unique_ptr<SI::Frame> frame)
    : QUndoCommand()
    , _document(document)
    , _frameId(frameId)
    , _frame(std::move(frame))
{
    Q_ASSERT(_frameId.isValid());
}

void AddRemoveFrame::addFrame()
{
    Q_ASSERT(_frame != nullptr);
    Q_ASSERT(_document->frameSet()->frames.contains(_frameId) == false);

    _document->frameListModel()->insertFrame(_frameId, std::move(_frame));
}

void AddRemoveFrame::removeFrame()
{
    Q_ASSERT(_frame == nullptr);
    Q_ASSERT(_document->frameSet()->frames.contains(_frameId));

    _frame = _document->frameListModel()->removeFrame(_frameId);
}

// AddFrame
// ========

std::unique_ptr<SI::Frame> AddFrame::createNewFrame(const SI::FrameSet& frameSet)
{
    auto frame = std::make_unique<SI::Frame>();
    frame->location.useGridLocation = true;
    frame->location.useGridOrigin = true;
    frame->location.update(frameSet.grid, *frame);

    return frame;
}

AddFrame::AddFrame(Document* document, const idstring& newId)
    : AddRemoveFrame(document, newId,
                     createNewFrame(*document->frameSet()))
{
    setText(QCoreApplication::tr("Add Frame"));
}

void AddFrame::undo()
{
    removeFrame();
}
void AddFrame::redo()
{
    addFrame();
}

// CloneFrame
// ==========

CloneFrame::CloneFrame(Document* document,
                       const idstring& id, const idstring& newId)
    : AddRemoveFrame(document, newId,
                     std::make_unique<SI::Frame>(document->frameSet()->frames.at(id)))
{
    setText(QCoreApplication::tr("Clone Frame"));
}

void CloneFrame::undo()
{
    removeFrame();
}
void CloneFrame::redo()
{
    addFrame();
}

// RemoveFrame
// ===========

RemoveFrame::RemoveFrame(Document* document, const idstring& frameId)
    : AddRemoveFrame(document, frameId, nullptr)
{
    setText(QCoreApplication::tr("Remove Frame"));
}

void RemoveFrame::undo()
{
    addFrame();
}
void RemoveFrame::redo()
{
    removeFrame();
}

// RenameFrame
// ===========

RenameFrame::RenameFrame(Document* document,
                         const idstring& oldId, const idstring& newId)
    : QUndoCommand(QCoreApplication::tr("Rename Frame"))
    , _document(document)
    , _oldId(oldId)
    , _newId(newId)
{
    Q_ASSERT(_oldId != _newId);
}

void RenameFrame::undo()
{
    _document->frameListModel()->renameFrame(_newId, _oldId);
}

void RenameFrame::redo()
{
    _document->frameListModel()->renameFrame(_oldId, _newId);
}

// ChangeFrameSpriteOrder
// ======================

ChangeFrameSpriteOrder::ChangeFrameSpriteOrder(Document* document,
                                               SI::Frame* frame, unsigned order)
    : QUndoCommand(QCoreApplication::tr("Change Sprite Order"))
    , _document(document)
    , _frame(frame)
    , _oldOrder(frame->spriteOrder)
    , _newOrder(order)
{
    Q_ASSERT(_oldOrder != _newOrder);
}

void ChangeFrameSpriteOrder::undo()
{
    _frame->spriteOrder = _oldOrder;
    emit _document->frameDataChanged(_frame);
}

void ChangeFrameSpriteOrder::redo()
{
    _frame->spriteOrder = _newOrder;
    emit _document->frameDataChanged(_frame);
}

// ChangeFrameLocation
// ===================

ChangeFrameLocation::ChangeFrameLocation(Document* document,
                                         SI::Frame* frame,
                                         const SI::FrameLocation& location)
    : QUndoCommand(QCoreApplication::tr("Change Frame Location"))
    , _document(document)
    , _frame(frame)
    , _oldLocation(frame->location)
    , _newLocation(location)
{
    Q_ASSERT(_oldLocation != _newLocation);
}

void ChangeFrameLocation::undo()
{
    _frame->location = _oldLocation;

    emit _document->frameLocationChanged(_frame);
    emit _document->frameDataChanged(_frame);
}

void ChangeFrameLocation::redo()
{
    _frame->location = _newLocation;

    emit _document->frameLocationChanged(_frame);
    emit _document->frameDataChanged(_frame);
}

// ChangeFrameSolid
// ================

ChangeFrameSolid::ChangeFrameSolid(
    Document* document, SI::Frame* frame, bool solid)
    : QUndoCommand()
    , _document(document)
    , _frame(frame)
    , _oldSolid(frame->solid)
    , _newSolid(solid)
{
    Q_ASSERT(_oldSolid != _newSolid);

    if (_newSolid) {
        setText(QCoreApplication::tr("Enable Tile Hitbox"));
    }
    else {
        setText(QCoreApplication::tr("Disable Tile Hitbox"));
    }
}

void ChangeFrameSolid::undo()
{
    _frame->solid = _oldSolid;

    emit _document->frameTileHitboxChanged(_frame);
    emit _document->frameDataChanged(_frame);
}

void ChangeFrameSolid::redo()
{
    _frame->solid = _newSolid;

    emit _document->frameTileHitboxChanged(_frame);
    emit _document->frameDataChanged(_frame);
}

// ChangeFrameTileHitbox
// =====================

ChangeFrameTileHitbox::ChangeFrameTileHitbox(
    Document* document, SI::Frame* frame, const urect& hitbox,
    QUndoCommand* parent)
    : QUndoCommand(QCoreApplication::tr("Change Tile Hitbox"), parent)
    , _document(document)
    , _frame(frame)
    , _oldHitbox(frame->tileHitbox)
    , _newHitbox(hitbox)
{
    Q_ASSERT(_oldHitbox != _newHitbox);
}

void ChangeFrameTileHitbox::undo()
{
    _frame->tileHitbox = _oldHitbox;

    emit _document->frameTileHitboxChanged(_frame);
    emit _document->frameDataChanged(_frame);
}

void ChangeFrameTileHitbox::redo()
{
    _frame->tileHitbox = _newHitbox;

    emit _document->frameTileHitboxChanged(_frame);
    emit _document->frameDataChanged(_frame);
}
