/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/metasprite.h"
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;

namespace MS = UnTech::MetaSprite::MetaSprite;

class AddRemoveFrame : public QUndoCommand {
protected:
    AddRemoveFrame(Document* document,
                   const idstring& frameId, std::unique_ptr<MS::Frame> frame);
    ~AddRemoveFrame() = default;

protected:
    void addFrame();
    void removeFrame();

private:
    Document* _document;
    const idstring _frameId;
    std::unique_ptr<MS::Frame> _frame;
};

class AddFrame : public AddRemoveFrame {
public:
    AddFrame(Document* document, const idstring& newId);
    ~AddFrame() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class CloneFrame : public AddRemoveFrame {
public:
    CloneFrame(Document* document,
               const idstring& existingId, const idstring& newId);
    ~CloneFrame() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class RemoveFrame : public AddRemoveFrame {
public:
    RemoveFrame(Document* document, const idstring& frameId);
    ~RemoveFrame() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class RenameFrame : public QUndoCommand {
public:
    RenameFrame(Document* document,
                const idstring& oldId, const idstring& newId);
    ~RenameFrame() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    const idstring _oldId;
    const idstring _newId;
};

class ChangeFrameSpriteOrder : public QUndoCommand {
public:
    ChangeFrameSpriteOrder(Document* document,
                           MS::Frame* frame, unsigned order);
    ~ChangeFrameSpriteOrder() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    MS::Frame* _frame;
    const unsigned _oldOrder;
    const unsigned _newOrder;
};

class ChangeFrameSolid : public QUndoCommand {
public:
    ChangeFrameSolid(Document* document,
                     MS::Frame* frame, bool solid);
    ~ChangeFrameSolid() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    MS::Frame* _frame;
    const bool _oldSolid;
    const bool _newSolid;
};

class ChangeFrameTileHitbox : public QUndoCommand {
public:
    ChangeFrameTileHitbox(Document* document,
                          MS::Frame* frame, const ms8rect& hitbox);
    ~ChangeFrameTileHitbox() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    MS::Frame* _frame;
    const ms8rect _oldHitbox;
    const ms8rect _newHitbox;
};
}
}
}
}
