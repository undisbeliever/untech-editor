/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/spriteimporter.h"
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;

namespace SI = UnTech::MetaSprite::SpriteImporter;

#define CREATE_COMMAND_CLASSES(CLS)                          \
    class Change##CLS : public QUndoCommand {                \
    public:                                                  \
        Change##CLS(Document* document, SI::Frame* frame,    \
                    unsigned index, const SI::CLS& value);   \
        ~Change##CLS() = default;                            \
                                                             \
        virtual void undo() final;                           \
        virtual void redo() final;                           \
                                                             \
    private:                                                 \
        Document* _document;                                 \
        SI::Frame* _frame;                                   \
        unsigned _index;                                     \
        const SI::CLS _old;                                  \
        const SI::CLS _new;                                  \
    };                                                       \
                                                             \
    class AddRemove##CLS : public QUndoCommand {             \
    protected:                                               \
        AddRemove##CLS(Document* document, SI::Frame* frame, \
                       unsigned index, const SI::CLS&,       \
                       const QString& text);                 \
        ~AddRemove##CLS() = default;                         \
                                                             \
    protected:                                               \
        void add##CLS();                                     \
        void remove##CLS();                                  \
                                                             \
    private:                                                 \
        Document* _document;                                 \
        SI::Frame* _frame;                                   \
        unsigned _index;                                     \
        const SI::CLS _value;                                \
    };                                                       \
                                                             \
    class Add##CLS : public AddRemove##CLS {                 \
    public:                                                  \
        Add##CLS(Document* document, SI::Frame* frame);      \
        ~Add##CLS() = default;                               \
                                                             \
        virtual void undo() final;                           \
        virtual void redo() final;                           \
    };                                                       \
                                                             \
    class Clone##CLS : public AddRemove##CLS {               \
    public:                                                  \
        Clone##CLS(Document* document, SI::Frame* frame,     \
                   unsigned index);                          \
        ~Clone##CLS() = default;                             \
                                                             \
        virtual void undo() final;                           \
        virtual void redo() final;                           \
    };                                                       \
                                                             \
    class Remove##CLS : public AddRemove##CLS {              \
    public:                                                  \
        Remove##CLS(Document* document, SI::Frame* frame,    \
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
