/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetcommands.h"
#include "document.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

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
