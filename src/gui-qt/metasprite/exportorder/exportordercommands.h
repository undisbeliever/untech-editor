/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "exportorderresourceitem.h"
#include "models/metasprite/frameset-exportorder.h"
#include <QApplication>
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class ExportOrderResourceItem;

class EditExportOrderExportNameCommand : public QUndoCommand {
    using FrameSetExportOrder = UnTech::MetaSprite::FrameSetExportOrder;

public:
    EditExportOrderExportNameCommand(ExportOrderResourceItem* item,
                                     bool isFrame, unsigned index,
                                     idstring&& newExportName)
        : QUndoCommand(QCoreApplication::tr("Edit Export Order Name"))
        , _exportOrderItem(item)
        , _isFrame(isFrame)
        , _index(index)
        , _oldExportName(item->exportName(isFrame, index).name)
        , _newExportName(newExportName)
    {
        Q_ASSERT(_oldExportName != _newExportName);
    }

    ~EditExportOrderExportNameCommand() = default;

    virtual void undo() final
    {
        _exportOrderItem->setExportName(_isFrame, _index, _oldExportName);
    }

    virtual void redo() final
    {
        _exportOrderItem->setExportName(_isFrame, _index, _newExportName);
    }

private:
    ExportOrderResourceItem* const _exportOrderItem;
    const bool _isFrame;
    const unsigned _index;
    const idstring _oldExportName;
    const idstring _newExportName;
};

class EditExportOrderAlternativeCommand : public QUndoCommand {
    using FrameSetExportOrder = UnTech::MetaSprite::FrameSetExportOrder;
    using NameReference = UnTech::MetaSprite::NameReference;

public:
    EditExportOrderAlternativeCommand(ExportOrderResourceItem* item,
                                      bool isFrame, unsigned index, unsigned altIndex,
                                      NameReference&& alt)
        : QUndoCommand(QCoreApplication::tr("Edit Export Order Alternative"))
        , _exportOrderItem(item)
        , _isFrame(isFrame)
        , _index(index)
        , _altIndex(altIndex)
        , _oldAlt(item->exportName(isFrame, index).alternatives.at(altIndex))
        , _newAlt(alt)
    {
        Q_ASSERT(_oldAlt != _newAlt);
    }

    ~EditExportOrderAlternativeCommand() = default;

    virtual void undo() final
    {
        _exportOrderItem->setExportNameAlternative(_isFrame, _index, _altIndex, _oldAlt);
    }

    virtual void redo() final
    {
        _exportOrderItem->setExportNameAlternative(_isFrame, _index, _altIndex, _newAlt);
    }

private:
    ExportOrderResourceItem* const _exportOrderItem;
    const bool _isFrame;
    const unsigned _index;
    const unsigned _altIndex;
    const NameReference _oldAlt;
    const NameReference _newAlt;
};
}
}
}
