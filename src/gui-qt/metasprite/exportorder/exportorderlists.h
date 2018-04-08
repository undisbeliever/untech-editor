/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "exportorderresourceitem.h"
#include "gui-qt/undo/undo.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ExportOrder {

class ExportNameList : public QObject {
    Q_OBJECT

public:
    using DataT = UnTech::MetaSprite::FrameSetExportOrder::ExportName;
    using ListT = DataT::list_t;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<bool>;

    constexpr static index_type max_size = ListT::MAX_SIZE;
    constexpr static char type_name[] = "Export Name";

private:
    ExportOrderResourceItem* const _exportOrder;

public:
    ExportNameList(ExportOrderResourceItem* exportOrder)
        : QObject(exportOrder)
        , _exportOrder(exportOrder)
    {
    }

    ExportOrderResourceItem* resourceItem() const { return _exportOrder; }

    QString typeName() const { return tr("Export Name"); }

protected:
    friend class Undo::ListUndoHelper<ExportNameList>;
    ListT* getList(bool isFrame)
    {
        auto* eo = _exportOrder->exportOrderEditable();
        if (eo == nullptr) {
            return nullptr;
        }
        return isFrame ? &eo->stillFrames : &eo->animations;
    }

signals:
    void dataChanged(bool isFrame, index_type index);
};

class AlternativesList : public QObject {
    Q_OBJECT

public:
    using DataT = UnTech::MetaSprite::NameReference;
    using ListT = std::vector<DataT>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<bool, index_type>;

    constexpr static index_type max_size = 256;

private:
    ExportOrderResourceItem* const _exportOrder;

public:
    AlternativesList(ExportOrderResourceItem* exportOrder)
        : QObject(exportOrder)
        , _exportOrder(exportOrder)
    {
    }

    ExportOrderResourceItem* resourceItem() const { return _exportOrder; }

    QString typeName() const { return tr("Export Name Alternative"); }

protected:
    friend class Undo::ListUndoHelper<AlternativesList>;
    ListT* getList(bool isFrame, index_type index)
    {
        auto* eo = _exportOrder->exportOrderEditable();
        if (eo == nullptr) {
            return nullptr;
        }

        auto& nameList = isFrame ? eo->stillFrames : eo->animations;

        if (index < nameList.size()) {
            return &nameList.at(index).alternatives;
        }
        else {
            return nullptr;
        }
    }

signals:
    void dataChanged(bool isFrame, index_type index, index_type altIndex);
};

using ExportNameUndoHelper = Undo::ListUndoHelper<ExportNameList>;
using AlternativesUndoHelper = Undo::ListUndoHelper<AlternativesList>;
}
}
}
}
