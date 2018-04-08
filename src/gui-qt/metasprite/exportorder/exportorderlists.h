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

private:
    ExportOrderResourceItem* const _exportOrder;

    bool _selectedListIsFrame;
    index_type _selectedIndex;

public:
    ExportNameList(ExportOrderResourceItem* exportOrder)
        : QObject(exportOrder)
        , _exportOrder(exportOrder)
        , _selectedListIsFrame(true)
        , _selectedIndex(INT_MAX)
    {
    }

    ExportOrderResourceItem* resourceItem() const { return _exportOrder; }

    QString typeName() const { return tr("Export Name"); }

    const bool& selectedListIsFrame() const { return _selectedListIsFrame; }
    void setSelectedListIsFrame(bool isFrame)
    {
        if (_selectedListIsFrame != isFrame) {
            unselectItem();

            _selectedListIsFrame = isFrame;
            emit selectedListChanged();
        }
    }

    index_type selectedIndex() const { return _selectedIndex; }
    void setSelectedIndex(index_type index)
    {
        if (_selectedIndex != index) {
            _selectedIndex = index;
            emit selectedIndexChanged();
        }
    }
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool isSelectedItemValid() const
    {
        auto* eo = _exportOrder->exportOrderEditable();
        if (eo == nullptr) {
            return false;
        }
        const auto& nl = _selectedListIsFrame ? &eo->stillFrames : &eo->animations;
        return _selectedIndex < nl->size();
    }

protected:
    friend class Undo::ListUndoHelper<ExportNameList>;
    friend class Undo::ListActionHelper;
    ListT* getList(bool isFrame)
    {
        auto* eo = _exportOrder->exportOrderEditable();
        if (eo == nullptr) {
            return nullptr;
        }
        return isFrame ? &eo->stillFrames : &eo->animations;
    }

    ArgsT selectedListTuple() const
    {
        return std::make_tuple(_selectedListIsFrame);
    }

signals:
    void dataChanged(bool isFrame, index_type index);
    void listChanged(bool isFrame);
    void itemAdded(bool isFrame, index_type index);
    void itemAboutToBeRemoved(bool isFrame, index_type index);

    void selectedListChanged();
    void selectedIndexChanged();
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

    index_type _selectedIndex;

public:
    AlternativesList(ExportOrderResourceItem* exportOrder)
        : QObject(exportOrder)
        , _exportOrder(exportOrder)
        , _selectedIndex(INT_MAX)
    {
        connect(_exportOrder->exportNameList(), &ExportNameList::selectedIndexChanged,
                this, &AlternativesList::unselectItem);
    }

    ExportOrderResourceItem* resourceItem() const { return _exportOrder; }

    QString typeName() const { return tr("Export Name Alternative"); }

    index_type selectedIndex() const { return _selectedIndex; }

    void setSelectedIndex(index_type index)
    {
        if (_selectedIndex != index) {
            _selectedIndex = index;
            emit selectedIndexChanged();
        }
    }
    void unselectItem() { setSelectedIndex(INT_MAX); }

protected:
    friend class Undo::ListUndoHelper<AlternativesList>;
    friend class Undo::ListActionHelper;
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

    ArgsT selectedListTuple() const
    {
        const ExportNameList* enl = _exportOrder->exportNameList();
        return std::make_tuple(enl->selectedListIsFrame(), enl->selectedIndex());
    }

signals:
    void dataChanged(bool isFrame, index_type index, index_type altIndex);
    void listChanged(bool isFrame, index_type index);
    void itemAdded(bool isFrame, index_type index, index_type altIndex);
    void itemAboutToBeRemoved(bool isFrame, index_type index, index_type altIndex);

    void selectedListChanged();
    void selectedIndexChanged();
};

using ExportNameUndoHelper = Undo::ListUndoHelper<ExportNameList>;
using AlternativesUndoHelper = Undo::ListUndoHelper<AlternativesList>;
}
}
}
}
