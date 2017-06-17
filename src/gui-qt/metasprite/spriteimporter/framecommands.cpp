/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecommands.h"
#include "document.h"
#include "framelistmodel.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

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
    emit _document->frameDataChanged(_frame);
}

void ChangeFrameLocation::redo()
{
    _frame->location = _newLocation;
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
    emit _document->frameDataChanged(_frame);
}

void ChangeFrameSolid::redo()
{
    _frame->solid = _newSolid;
    emit _document->frameDataChanged(_frame);
}

// ChangeFrameTileHitbox
// =====================

ChangeFrameTileHitbox::ChangeFrameTileHitbox(
    Document* document, SI::Frame* frame, const urect& hitbox)
    : QUndoCommand(QCoreApplication::tr("Change Tile Hitbox"))
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
    emit _document->frameDataChanged(_frame);
}

void ChangeFrameTileHitbox::redo()
{
    _frame->tileHitbox = _newHitbox;
    emit _document->frameDataChanged(_frame);
}
