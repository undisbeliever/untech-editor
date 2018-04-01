/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetcommands.h"
#include "document.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

// ChangeFrameSetName
// ==================

ChangeFrameSetName::ChangeFrameSetName(Document* document,
                                       const idstring& name)
    : QUndoCommand(QCoreApplication::tr("Change FrameSet Name"))
    , _document(document)
    , _oldName(document->frameSet()->name)
    , _newName(name)
{
    Q_ASSERT(_oldName != _newName);
}

void ChangeFrameSetName::undo()
{
    _document->frameSet()->name = _oldName;

    emit _document->frameSetNameChanged();
    emit _document->frameSetDataChanged();
}

void ChangeFrameSetName::redo()
{
    _document->frameSet()->name = _newName;

    emit _document->frameSetNameChanged();
    emit _document->frameSetDataChanged();
}

// ChangeFrameSetTilesetType
// =========================

ChangeFrameSetTilesetType::ChangeFrameSetTilesetType(
    Document* document, const TilesetType& tilesetType)
    : QUndoCommand(QCoreApplication::tr("Change Tileset Type"))
    , _document(document)
    , _oldTilesetType(document->frameSet()->tilesetType)
    , _newTilesetType(tilesetType)
{
    Q_ASSERT(_oldTilesetType != _newTilesetType);
}

void ChangeFrameSetTilesetType::undo()
{
    _document->frameSet()->tilesetType = _oldTilesetType;
    emit _document->frameSetDataChanged();
}

void ChangeFrameSetTilesetType::redo()
{
    _document->frameSet()->tilesetType = _newTilesetType;
    emit _document->frameSetDataChanged();
}

// ChangeFrameSetExportOrder
// =========================

ChangeFrameSetExportOrder::ChangeFrameSetExportOrder(
    Document* document, const ExportOrderPtr& exportOrder)
    : QUndoCommand(QCoreApplication::tr("Change Export Order"))
    , _document(document)
    , _oldExportOrder(document->frameSet()->exportOrder)
    , _newExportOrder(exportOrder)
{
    Q_ASSERT(_oldExportOrder != _newExportOrder);
}

void ChangeFrameSetExportOrder::undo()
{
    _document->frameSet()->exportOrder = _oldExportOrder;
    emit _document->frameSetDataChanged();
}

void ChangeFrameSetExportOrder::redo()
{
    _document->frameSet()->exportOrder = _newExportOrder;
    emit _document->frameSetDataChanged();
}

// ChangeFrameSetImageFile
// =======================

ChangeFrameSetImageFile::ChangeFrameSetImageFile(Document* document,
                                                 const std::string& filename)
    : QUndoCommand(QCoreApplication::tr("Change Image"))
    , _document(document)
    , _oldFilename(document->frameSet()->imageFilename)
    , _newFilename(filename)
    , _oldTransparentColor(document->frameSet()->transparentColor)
{
    Q_ASSERT(_oldFilename != _newFilename);
}

void ChangeFrameSetImageFile::undo()
{
    SI::FrameSet* fs = _document->frameSet();

    fs->loadImage(_oldFilename);
    fs->transparentColor = _oldTransparentColor;

    emit _document->frameSetDataChanged();
    emit _document->frameSetImageFilenameChanged();
    emit _document->frameSetImageChanged();
}

void ChangeFrameSetImageFile::redo()
{
    SI::FrameSet* fs = _document->frameSet();

    fs->transparentColor = _oldTransparentColor;
    fs->loadImage(_newFilename);

    emit _document->frameSetDataChanged();
    emit _document->frameSetImageFilenameChanged();
    emit _document->frameSetImageChanged();
}

// ChangeFrameSetTransparentColor
// ==============================

ChangeFrameSetTransparentColor::ChangeFrameSetTransparentColor(Document* document,
                                                               const rgba& color)
    : QUndoCommand(QCoreApplication::tr("Change Transparent Colour"))
    , _document(document)
    , _oldColor(document->frameSet()->transparentColor)
    , _newColor(color)
{
    Q_ASSERT(_oldColor != _newColor);
}

void ChangeFrameSetTransparentColor::undo()
{
    _document->frameSet()->transparentColor = _oldColor;
    emit _document->frameSetDataChanged();
}

void ChangeFrameSetTransparentColor::redo()
{
    _document->frameSet()->transparentColor = _newColor;
    emit _document->frameSetDataChanged();
}

// ChangeFrameSetGrid
// ==================

ChangeFrameSetGrid::ChangeFrameSetGrid(Document* document,
                                       const SI::FrameSetGrid& grid)
    : QUndoCommand(QCoreApplication::tr("Change FrameSet Grid"))
    , _document(document)
    , _oldGrid(document->frameSet()->grid)
    , _newGrid(grid)
{
    Q_ASSERT(_oldGrid != _newGrid);
}

void ChangeFrameSetGrid::undo()
{
    _document->frameSet()->grid = _oldGrid;
    _document->frameSet()->updateFrameLocations();

    emit _document->frameSetDataChanged();
    emit _document->frameSetGridChanged();
}

void ChangeFrameSetGrid::redo()
{
    _document->frameSet()->grid = _newGrid;
    _document->frameSet()->updateFrameLocations();

    emit _document->frameSetDataChanged();
    emit _document->frameSetGridChanged();
}

// ChangeFrameSetPalette
// =====================

ChangeFrameSetPalette::ChangeFrameSetPalette(Document* document,
                                             const SI::UserSuppliedPalette& palette)
    : QUndoCommand()
    , _document(document)
    , _oldPalette(document->frameSet()->palette)
    , _newPalette(palette)
{
    Q_ASSERT(_oldPalette != _newPalette);

    bool oldEnabled = _oldPalette.usesUserSuppliedPalette();
    bool newEnabled = _newPalette.usesUserSuppliedPalette();

    if (!oldEnabled && newEnabled) {
        setText(QCoreApplication::tr("Enable User Supplied Palette"));
    }
    else if (!newEnabled && oldEnabled) {
        setText(QCoreApplication::tr("Disable User Supplied Palette"));
    }
    else {
        setText(QCoreApplication::tr("Change User Supplied Palette"));
    }
}

void ChangeFrameSetPalette::undo()
{
    _document->frameSet()->palette = _oldPalette;
    emit _document->frameSetDataChanged();
    emit _document->frameSetPaletteChanged();
}

void ChangeFrameSetPalette::redo()
{
    _document->frameSet()->palette = _newPalette;
    emit _document->frameSetDataChanged();
    emit _document->frameSetPaletteChanged();
}
