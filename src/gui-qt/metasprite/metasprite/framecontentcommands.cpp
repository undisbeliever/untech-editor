/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentcommands.h"
#include "document.h"
#include "framecontentsmodel.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

#define CHANGE_COMMAND(CLS, FIELD, TEXT)                                 \
    Change##CLS::Change##CLS(                                            \
        Document* document, MS::Frame* frame,                            \
        unsigned index, const MS::CLS& value)                            \
        : QUndoCommand(QCoreApplication::tr(TEXT))                       \
        , _document(document)                                            \
        , _frame(frame)                                                  \
        , _index(index)                                                  \
        , _old(frame->FIELD.at(index))                                   \
        , _new(value)                                                    \
    {                                                                    \
        Q_ASSERT(_old != _new);                                          \
    }                                                                    \
                                                                         \
    void Change##CLS::undo()                                             \
    {                                                                    \
        _document->frameContentsModel()->set##CLS(_frame, _index, _old); \
    }                                                                    \
                                                                         \
    void Change##CLS::redo()                                             \
    {                                                                    \
        _document->frameContentsModel()->set##CLS(_frame, _index, _new); \
    }

CHANGE_COMMAND(FrameObject, objects, "Change Frame Object");
CHANGE_COMMAND(ActionPoint, actionPoints, "Change Action Point");
CHANGE_COMMAND(EntityHitbox, entityHitboxes, "Change Entity Hitbox");

#define ADD_REMOVE_COMMAND(CLS)                                               \
    AddRemove##CLS::AddRemove##CLS(Document* document, MS::Frame* frame,      \
                                   unsigned index, const MS::CLS& value,      \
                                   const QString& text)                       \
        : QUndoCommand(text)                                                  \
        , _document(document)                                                 \
        , _frame(frame)                                                       \
        , _index(index)                                                       \
        , _value(value)                                                       \
    {                                                                         \
        Q_ASSERT(_frame != nullptr);                                          \
    }                                                                         \
                                                                              \
    void AddRemove##CLS::add##CLS()                                           \
    {                                                                         \
        _document->frameContentsModel()->insert##CLS(_frame, _index, _value); \
    }                                                                         \
                                                                              \
    void AddRemove##CLS::remove##CLS()                                        \
    {                                                                         \
        _document->frameContentsModel()->remove##CLS(_frame, _index);         \
    }
ADD_REMOVE_COMMAND(FrameObject);
ADD_REMOVE_COMMAND(ActionPoint);
ADD_REMOVE_COMMAND(EntityHitbox);

#define ADD_COMMAND(CLS, CONTAINER, TEXT)                    \
    Add##CLS::Add##CLS(Document* document, MS::Frame* frame) \
        : AddRemove##CLS(document, frame,                    \
                         frame->CONTAINER.size(),            \
                         MS::CLS(),                          \
                         QCoreApplication::tr(TEXT))         \
    {                                                        \
    }                                                        \
                                                             \
    void Add##CLS::undo()                                    \
    {                                                        \
        remove##CLS();                                       \
    }                                                        \
    void Add##CLS::redo()                                    \
    {                                                        \
        add##CLS();                                          \
    }
ADD_COMMAND(FrameObject, objects, "Add Frame Object");
ADD_COMMAND(ActionPoint, actionPoints, "Add Action Point");
ADD_COMMAND(EntityHitbox, entityHitboxes, "Add Entity Hitbox");

#define CLONE_COMMAND(CLS, CONTAINER, TEXT)                      \
    Clone##CLS::Clone##CLS(Document* document, MS::Frame* frame, \
                           unsigned index)                       \
        : AddRemove##CLS(document, frame,                        \
                         frame->CONTAINER.size(),                \
                         frame->CONTAINER.at(index),             \
                         QCoreApplication::tr(TEXT))             \
    {                                                            \
    }                                                            \
                                                                 \
    void Clone##CLS::undo()                                      \
    {                                                            \
        remove##CLS();                                           \
    }                                                            \
    void Clone##CLS::redo()                                      \
    {                                                            \
        add##CLS();                                              \
    }
CLONE_COMMAND(FrameObject, objects, "Clone Frame Object");
CLONE_COMMAND(ActionPoint, actionPoints, "Clone Action Point");
CLONE_COMMAND(EntityHitbox, entityHitboxes, "Clone Entity Hitbox");

#define REMOVE_COMMAND(CLS, CONTAINER, TEXT)                       \
    Remove##CLS::Remove##CLS(Document* document, MS::Frame* frame, \
                             unsigned index)                       \
        : AddRemove##CLS(document, frame,                          \
                         index,                                    \
                         frame->CONTAINER.at(index),               \
                         QCoreApplication::tr(TEXT))               \
    {                                                              \
    }                                                              \
                                                                   \
    void Remove##CLS::undo()                                       \
    {                                                              \
        add##CLS();                                                \
    }                                                              \
    void Remove##CLS::redo()                                       \
    {                                                              \
        remove##CLS();                                             \
    }
REMOVE_COMMAND(FrameObject, objects, "Remove Frame Object");
REMOVE_COMMAND(ActionPoint, actionPoints, "Remove Action Point");
REMOVE_COMMAND(EntityHitbox, entityHitboxes, "Remove Entity Hitbox");

// RaiseFrameContents
// ==================

RaiseFrameContents::RaiseFrameContents(Document* document, MS::Frame* frame,
                                       const std::set<SelectedItem>& items)
    : QUndoCommand(QCoreApplication::tr("Raise"))
    , _document(document)
    , _frame(frame)
    , _undoItems(moveSelectedItems(items, -1))
    , _redoItems(items)
{
}

void RaiseFrameContents::undo()
{
    _document->frameContentsModel()->lowerSelectedItems(_frame, _undoItems);
}

void RaiseFrameContents::redo()
{
    _document->frameContentsModel()->raiseSelectedItems(_frame, _redoItems);
}

// LowerFrameContents
// ==================

LowerFrameContents::LowerFrameContents(Document* document, MS::Frame* frame,
                                       const std::set<SelectedItem>& items)
    : QUndoCommand(QCoreApplication::tr("Lower"))
    , _document(document)
    , _frame(frame)
    , _undoItems(moveSelectedItems(items, 1))
    , _redoItems(items)
{
}

void LowerFrameContents::undo()
{
    _document->frameContentsModel()->raiseSelectedItems(_frame, _undoItems);
}

void LowerFrameContents::redo()
{
    _document->frameContentsModel()->lowerSelectedItems(_frame, _redoItems);
}
