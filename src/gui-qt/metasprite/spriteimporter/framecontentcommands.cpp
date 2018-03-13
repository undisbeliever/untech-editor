/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentcommands.h"
#include "document.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

#define CHANGE_COMMAND(CLS, FIELD, SIGNAL, TEXT)                    \
    Change##CLS::Change##CLS(                                       \
        Document* document, SI::Frame* frame,                       \
        unsigned index, const SI::CLS& value, QUndoCommand* parent) \
        : QUndoCommand(QCoreApplication::tr(TEXT), parent)          \
        , _document(document)                                       \
        , _frame(frame)                                             \
        , _index(index)                                             \
        , _old(frame->FIELD.at(index))                              \
        , _new(value)                                               \
    {                                                               \
        Q_ASSERT(_old != _new);                                     \
    }                                                               \
                                                                    \
    void Change##CLS::undo()                                        \
    {                                                               \
        _frame->FIELD.at(_index) = _old;                            \
        emit _document->SIGNAL(_frame, _index);                     \
    }                                                               \
                                                                    \
    void Change##CLS::redo()                                        \
    {                                                               \
        _frame->FIELD.at(_index) = _new;                            \
        emit _document->SIGNAL(_frame, _index);                     \
    }
CHANGE_COMMAND(FrameObject, objects, frameObjectChanged, "Change Frame Object");
CHANGE_COMMAND(ActionPoint, actionPoints, actionPointChanged, "Change Action Point");
CHANGE_COMMAND(EntityHitbox, entityHitboxes, entityHitboxChanged, "Change Entity Hitbox");

#define ADD_REMOVE_COMMAND(CLS, FIELD, SIGNAL)                           \
    AddRemove##CLS::AddRemove##CLS(Document* document, SI::Frame* frame, \
                                   unsigned index, const SI::CLS& value, \
                                   const QString& text)                  \
        : QUndoCommand(text)                                             \
        , _document(document)                                            \
        , _frame(frame)                                                  \
        , _index(index)                                                  \
        , _value(value)                                                  \
    {                                                                    \
        Q_ASSERT(_frame != nullptr);                                     \
    }                                                                    \
                                                                         \
    void AddRemove##CLS::add##CLS()                                      \
    {                                                                    \
        auto it = _frame->FIELD.begin() + _index;                        \
        _frame->FIELD.insert(it, _value);                                \
                                                                         \
        emit _document->SIGNAL##Added(_frame, _index);                   \
        emit _document->SIGNAL##ListChanged(_frame);                     \
    }                                                                    \
                                                                         \
    void AddRemove##CLS::remove##CLS()                                   \
    {                                                                    \
        emit _document->SIGNAL##AboutToBeRemoved(_frame, _index);        \
                                                                         \
        auto it = _frame->FIELD.begin() + _index;                        \
        _frame->FIELD.erase(it);                                         \
                                                                         \
        emit _document->SIGNAL##ListChanged(_frame);                     \
    }
ADD_REMOVE_COMMAND(FrameObject, objects, frameObject);
ADD_REMOVE_COMMAND(ActionPoint, actionPoints, actionPoint);
ADD_REMOVE_COMMAND(EntityHitbox, entityHitboxes, entityHitbox);

#define ADD_COMMAND(CLS, CONTAINER, TEXT)                    \
    Add##CLS::Add##CLS(Document* document, SI::Frame* frame) \
        : AddRemove##CLS(document, frame,                    \
                         frame->CONTAINER.size(),            \
                         SI::CLS(),                          \
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
    Clone##CLS::Clone##CLS(Document* document, SI::Frame* frame, \
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
    Remove##CLS::Remove##CLS(Document* document, SI::Frame* frame, \
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
