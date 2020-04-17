/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/accessor/listactionhelper.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ExportOrder {

// ExportNameList and AlternativesList are implemented manually as they
// AlternativesList::dataChanged signal is three levels deep.

class ExportNameList final : public Accessor::AbstractListSingleSelectionAccessor {
    Q_OBJECT

public:
    using DataT = UnTech::MetaSprite::FrameSetExportOrder::ExportName;
    using ListT = NamedList<DataT>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<bool>;
    using SignalArgsT = ArgsT;

    using UndoHelper = Accessor::ListAndSelectionUndoHelper<ExportNameList>;

private:
    bool _selectedListIsFrame;

public:
    ExportNameList(ResourceItem* exportOrder);

    ResourceItem* resourceItem() const { return static_cast<ResourceItem*>(AbstractListSingleSelectionAccessor::resourceItem()); }

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    virtual bool listExists() const final;
    virtual size_t size() const final;

    bool selectedListIsFrame() const { return _selectedListIsFrame; }
    void setSelectedListIsFrame(bool isFrame);

    void setSelectedIndex(bool isFrame, index_type index);

    inline bool addItem() { return addItem(INT_MAX); }

    virtual bool addItem(size_t index) final;
    virtual bool cloneItem(size_t index) final;
    virtual bool removeItem(size_t index) final;
    virtual bool moveItem(size_t from, size_t to) final;

    bool editList_setName(bool isFrame, index_type index, const idstring& name);

protected:
    template <class, class>
    friend class Accessor::ListUndoHelper;
    friend class Accessor::ListActionHelper;
    ListT* getList(bool isFrame);
    ArgsT selectedListTuple() const;

    const ListT* selectedList() const;

signals:
    void dataChanged(bool isFrame, index_type index);
    void listChanged(bool isFrame);

    void listAboutToChange(bool isFrame);
    void itemAdded(bool isFrame, index_type index);
    void itemAboutToBeRemoved(bool isFrame, index_type index);
    void itemMoved(bool isFrame, index_type from, index_type to);

private slots:
    void onDataChanged(bool isFrame, size_t index);
    void onListChanged(bool isFrame);
    void onListAboutToChange(bool isFrame);
    void onItemAdded(bool isFrame, size_t index);
    void onItemAboutToBeRemoved(bool isFrame, size_t index);
    void onItemMoved(bool isFrame, size_t from, size_t to);
};

class AlternativesList final : public Accessor::AbstractListSingleSelectionAccessor {
    Q_OBJECT

public:
    using DataT = UnTech::MetaSprite::NameReference;
    using ListT = std::vector<DataT>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<bool, index_type>;
    using SignalArgsT = ArgsT;

    using UndoHelper = Accessor::ListAndSelectionUndoHelper<AlternativesList>;

private:
    index_type _selectedIndex;

public:
    AlternativesList(ResourceItem* exportOrder);

    ResourceItem* resourceItem() const { return static_cast<ResourceItem*>(AbstractListSingleSelectionAccessor::resourceItem()); }

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    virtual bool listExists() const final;
    virtual size_t size() const final;

    inline bool addItem() { return addItem(INT_MAX); }

    virtual bool addItem(size_t index) final;
    virtual bool cloneItem(size_t index) final;
    virtual bool removeItem(size_t index) final;
    virtual bool moveItem(size_t from, size_t to) final;

    bool editList_setValue(bool isFrame, index_type exportIndex, index_type altIndex, const DataT& value);

protected:
    template <class, class>
    friend class Accessor::ListUndoHelper;
    friend class Accessor::ListActionHelper;
    ListT* getList(bool isFrame, index_type enIndex);
    ArgsT selectedListTuple() const;

    const ListT* selectedList() const;

signals:
    void dataChanged(bool isFrame, index_type index, index_type altIndex);
    void listChanged(bool isFrame, index_type index);

    void listAboutToChange(bool isFrame, index_type index);
    void itemAdded(bool isFrame, index_type index, index_type altIndex);
    void itemAboutToBeRemoved(bool isFrame, index_type index, index_type altIndex);
    void itemMoved(bool isFrame, index_type index, index_type from, index_type to);

private slots:
    void onParentSelectedIndexChanged();
    void onDataChanged(bool isFrame, size_t parentIndex, size_t index);
    void onListChanged(bool isFrame, size_t parentIndex);
    void onListAboutToChange(bool isFrame, size_t parentIndex);
    void onItemAdded(bool isFrame, size_t parentIndex, size_t index);
    void onItemAboutToBeRemoved(bool isFrame, size_t parentIndex, size_t index);
    void onItemMoved(bool isFrame, size_t parentIndex, size_t from, size_t to);
};
}
}
}
}
