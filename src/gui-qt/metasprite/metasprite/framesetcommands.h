/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

class ChangeFrameSetName : public QUndoCommand {
public:
    ChangeFrameSetName(Document* document, const idstring& name);
    ~ChangeFrameSetName() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* const _document;
    const idstring _oldName;
    const idstring _newName;
};

class ChangeFrameSetTilesetType : public QUndoCommand {
    using TilesetType = UnTech::MetaSprite::TilesetType;

public:
    ChangeFrameSetTilesetType(Document* document, const TilesetType& tilesetType);
    ~ChangeFrameSetTilesetType() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* const _document;
    const TilesetType _oldTilesetType;
    const TilesetType _newTilesetType;
};

class ChangeFrameSetExportOrder : public QUndoCommand {
public:
    ChangeFrameSetExportOrder(
        Document* document, const idstring& exportOrder);
    ~ChangeFrameSetExportOrder() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* const _document;
    const idstring _oldExportOrder;
    const idstring _newExportOrder;
};
}
}
}
}
