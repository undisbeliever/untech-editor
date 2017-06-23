/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentcommands.h"
#include "document.h"
#include "framecontentsmodel.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

#define CHANGE_COMMAND(CLS, FIELD, TEXT)                                 \
    Change##CLS::Change##CLS(                                            \
        Document* document, SI::Frame* frame,                            \
        unsigned index, const SI::CLS& value)                            \
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
