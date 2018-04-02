/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

class ChangeFrameSetImageFile : public QUndoCommand {
public:
    ChangeFrameSetImageFile(Document* document, const std::string& filename);
    ~ChangeFrameSetImageFile() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* const _document;
    const std::string _oldFilename;
    const std::string _newFilename;
    const rgba _oldTransparentColor;
};

class ChangeFrameSetTransparentColor : public QUndoCommand {
public:
    ChangeFrameSetTransparentColor(Document* document, const rgba& color);
    ~ChangeFrameSetTransparentColor() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* const _document;
    const rgba _oldColor;
    const rgba _newColor;
};

class ChangeFrameSetGrid : public QUndoCommand {
public:
    ChangeFrameSetGrid(Document* document, const SI::FrameSetGrid& grid);
    ~ChangeFrameSetGrid() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* const _document;
    const SI::FrameSetGrid _oldGrid;
    const SI::FrameSetGrid _newGrid;
};

class ChangeFrameSetPalette : public QUndoCommand {
public:
    ChangeFrameSetPalette(Document* document, const SI::UserSuppliedPalette& palette);
    ~ChangeFrameSetPalette() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* const _document;
    const SI::UserSuppliedPalette _oldPalette;
    const SI::UserSuppliedPalette _newPalette;
};
}
}
}
}
