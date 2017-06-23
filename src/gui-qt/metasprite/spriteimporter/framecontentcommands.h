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

#define CHANGE_COMMAND_CLASS(CLS)                          \
    class Change##CLS : public QUndoCommand {              \
    public:                                                \
        Change##CLS(Document* document, SI::Frame* frame,  \
                    unsigned index, const SI::CLS& value); \
        ~Change##CLS() = default;                          \
                                                           \
        virtual void undo() final;                         \
        virtual void redo() final;                         \
                                                           \
    private:                                               \
        Document* _document;                               \
        SI::Frame* _frame;                                 \
        unsigned _index;                                   \
        const SI::CLS _old;                                \
        const SI::CLS _new;                                \
    };
CHANGE_COMMAND_CLASS(FrameObject)
CHANGE_COMMAND_CLASS(ActionPoint)
CHANGE_COMMAND_CLASS(EntityHitbox)

#undef CHANGE_COMMAND_CLASS
}
}
}
}
