/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "selection.h"
#include "gui-qt/metasprite/framecontentcommands.h"
#include "models/metasprite/metasprite.h"
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;

using RaiseFrameContents = RaiseFrameContents<Document>;
using LowerFrameContents = LowerFrameContents<Document>;

#define CREATE_COMMAND_CLASSES(CLS)                          \
    class Change##CLS : public QUndoCommand {                \
    public:                                                  \
        Change##CLS(Document* document, MS::Frame* frame,    \
                    unsigned index, const MS::CLS& value,    \
                    QUndoCommand* parent = nullptr);         \
        ~Change##CLS() = default;                            \
                                                             \
        virtual void undo() final;                           \
        virtual void redo() final;                           \
                                                             \
    private:                                                 \
        Document* const _document;                           \
        MS::Frame* const _frame;                             \
        const unsigned _index;                               \
        const MS::CLS _old;                                  \
        const MS::CLS _new;                                  \
    };                                                       \
                                                             \
    class AddRemove##CLS : public QUndoCommand {             \
    protected:                                               \
        AddRemove##CLS(Document* document, MS::Frame* frame, \
                       unsigned index, const MS::CLS&,       \
                       const QString& text);                 \
        ~AddRemove##CLS() = default;                         \
                                                             \
    protected:                                               \
        void add##CLS();                                     \
        void remove##CLS();                                  \
                                                             \
    private:                                                 \
        Document* const _document;                           \
        MS::Frame* const _frame;                             \
        const unsigned _index;                               \
        const MS::CLS _value;                                \
    };                                                       \
                                                             \
    class Add##CLS : public AddRemove##CLS {                 \
    public:                                                  \
        Add##CLS(Document* document, MS::Frame* frame);      \
        ~Add##CLS() = default;                               \
                                                             \
        virtual void undo() final;                           \
        virtual void redo() final;                           \
    };                                                       \
                                                             \
    class Clone##CLS : public AddRemove##CLS {               \
    public:                                                  \
        Clone##CLS(Document* document, MS::Frame* frame,     \
                   unsigned index);                          \
        ~Clone##CLS() = default;                             \
                                                             \
        virtual void undo() final;                           \
        virtual void redo() final;                           \
    };                                                       \
                                                             \
    class Remove##CLS : public AddRemove##CLS {              \
    public:                                                  \
        Remove##CLS(Document* document, MS::Frame* frame,    \
                    unsigned index);                         \
        ~Remove##CLS() = default;                            \
                                                             \
        virtual void undo() final;                           \
        virtual void redo() final;                           \
    };

CREATE_COMMAND_CLASSES(FrameObject)
CREATE_COMMAND_CLASSES(ActionPoint)
CREATE_COMMAND_CLASSES(EntityHitbox)

#undef CREATE_COMMAND_CLASSES
}
}
}
}
