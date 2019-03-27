/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "exportorderresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/accessor/listactionhelper.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ExportOrder {

// ExportNameList and AlternativesList are implemented manually as they
// AlternativesList::dataChanged signal is three levels deep.

class ExportNameList : public QObject {
    Q_OBJECT

public:
    using DataT = UnTech::MetaSprite::FrameSetExportOrder::ExportName;
    using ListT = std::vector<DataT>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<bool>;
    using SignalArgsT = ArgsT;

    using UndoHelper = Accessor::ListAndSelectionUndoHelper<ExportNameList>;

    constexpr static index_type maxSize() { return UnTech::MetaSprite::MAX_EXPORT_NAMES; }

private:
    ExportOrderResourceItem* const _exportOrder;

    bool _selectedListIsFrame;
    index_type _selectedIndex;

public:
    ExportNameList(ExportOrderResourceItem* exportOrder);

    ExportOrderResourceItem* resourceItem() const { return _exportOrder; }

    QString typeName() const { return tr("Export Name"); }

    const bool& selectedListIsFrame() const { return _selectedListIsFrame; }
    void setSelectedListIsFrame(bool isFrame);

    index_type selectedIndex() const { return _selectedIndex; }
    void setSelectedIndex(index_type index);
    void setSelectedIndex(bool isFrame, index_type index);
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool isSelectedItemValid() const;

    bool editList_setName(bool isFrame, index_type index, const idstring& name);

    bool editList_addFrame();
    bool editList_addAnimation();

    bool editSelectedList_cloneSelected();
    bool editSelectedList_removeSelected();
    bool editSelectedList_raiseSelectedToTop();
    bool editSelectedList_raiseSelected();
    bool editSelectedList_lowerSelected();
    bool editSelectedList_lowerSelectedToBottom();

protected:
    template <class, class>
    friend class Accessor::ListUndoHelper;
    friend class Accessor::ListActionHelper;
    ListT* getList(bool isFrame)
    {
        auto* eo = _exportOrder->dataEditable();
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

    void listAboutToChange(bool isFrame);
    void itemAdded(bool isFrame, index_type index);
    void itemAboutToBeRemoved(bool isFrame, index_type index);
    void itemMoved(bool isFrame, index_type from, index_type to);

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
    using SignalArgsT = ArgsT;

    using UndoHelper = Accessor::ListAndSelectionUndoHelper<AlternativesList>;

    constexpr static index_type maxSize() { return 256; }

private:
    ExportOrderResourceItem* const _exportOrder;

    index_type _selectedIndex;

public:
    AlternativesList(ExportOrderResourceItem* exportOrder);

    ExportOrderResourceItem* resourceItem() const { return _exportOrder; }

    QString typeName() const { return tr("Export Name Alternative"); }

    index_type selectedIndex() const { return _selectedIndex; }

    void setSelectedIndex(index_type index);
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool editList_setValue(bool isFrame, index_type exportIndex, index_type altIndex, const DataT& value);

    bool editSelectedList_addItem();
    bool editSelectedList_cloneSelected();
    bool editSelectedList_removeSelected();
    bool editSelectedList_raiseSelectedToTop();
    bool editSelectedList_raiseSelected();
    bool editSelectedList_lowerSelected();
    bool editSelectedList_lowerSelectedToBottom();

protected:
    template <class, class>
    friend class Accessor::ListUndoHelper;
    friend class Accessor::ListActionHelper;
    ListT* getList(bool isFrame, index_type index)
    {
        auto* eo = _exportOrder->dataEditable();
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

    void listAboutToChange(bool isFrame, index_type index);
    void itemAdded(bool isFrame, index_type index, index_type altIndex);
    void itemAboutToBeRemoved(bool isFrame, index_type index, index_type altIndex);
    void itemMoved(bool isFrame, index_type index, index_type from, index_type to);

    void selectedListChanged();
    void selectedIndexChanged();
};
}
}
}
}
