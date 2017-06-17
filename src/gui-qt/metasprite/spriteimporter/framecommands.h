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

class ChangeFrameSpriteOrder : public QUndoCommand {
public:
    ChangeFrameSpriteOrder(Document* document,
                           SI::Frame* frame, unsigned order);
    ~ChangeFrameSpriteOrder() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    SI::Frame* _frame;
    const unsigned _oldOrder;
    const unsigned _newOrder;
};

class ChangeFrameLocation : public QUndoCommand {
public:
    ChangeFrameLocation(Document* document,
                        SI::Frame* frame, const SI::FrameLocation& location);
    ~ChangeFrameLocation() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    SI::Frame* _frame;
    const SI::FrameLocation _oldLocation;
    const SI::FrameLocation _newLocation;
};

class ChangeFrameSolid : public QUndoCommand {
public:
    ChangeFrameSolid(Document* document,
                     SI::Frame* frame, bool solid);
    ~ChangeFrameSolid() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    SI::Frame* _frame;
    const bool _oldSolid;
    const bool _newSolid;
};

class ChangeFrameTileHitbox : public QUndoCommand {
public:
    ChangeFrameTileHitbox(Document* document,
                          SI::Frame* frame, const urect& hitbox);
    ~ChangeFrameTileHitbox() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    SI::Frame* _frame;
    const urect _oldHitbox;
    const urect _newHitbox;
};
}
}
}
}
