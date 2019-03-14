/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/vectorset-upoint.h"
#include "models/common/vectorset.h"
#include <QObject>

namespace UnTech {
class idstring;
template <class T>
class NamedList;
template <class T>
class grid;

namespace GuiQt {
class AbstractResourceItem;

namespace Accessor {
template <class T, class U>
class ListUndoHelper;

template <class T>
class ListWithNoSelectionUndoHelper;
template <class T>
class ListAndSelectionUndoHelper;
template <class T>
class ListAndMultipleSelectionUndoHelper;

class AbstractListAccessor : public QObject {
    Q_OBJECT

    AbstractResourceItem* const _resourceItem;
    const size_t _maxSize;

public:
    AbstractListAccessor(AbstractResourceItem* resourceItem, size_t maxSize);
    ~AbstractListAccessor() = default;

    virtual QString typeName() const = 0;
    virtual QString typeNamePlural() const = 0;

    AbstractResourceItem* resourceItem() const { return _resourceItem; }

    virtual bool listExists() const = 0;
    virtual size_t size() const = 0;
    size_t maxSize() const { return _maxSize; }

    bool addItem();

    virtual bool addItem(size_t index) = 0;
    virtual bool cloneItem(size_t index) = 0;
    virtual bool removeItem(size_t index) = 0;
    virtual bool moveItem(size_t from, size_t to) = 0;

signals:
    // In the ChildList classes these signals are emitted if the currently selected list changes

    // Emitted when the underlying list has changed (ie, ResourceItem is reverted to old state)
    void listReset();

    void dataChanged(size_t index);

    void listAboutToChange();
    void listChanged();
    void itemAdded(size_t index);
    void itemAboutToBeRemoved(size_t index);
    void itemMoved(size_t from, size_t to);
};

class AbstractListSingleSelectionAccessor : public AbstractListAccessor {
    Q_OBJECT

    size_t _selectedIndex;

public:
    AbstractListSingleSelectionAccessor(AbstractResourceItem* resourceItem, size_t maxSize);
    ~AbstractListSingleSelectionAccessor() = default;

    size_t selectedIndex() const { return _selectedIndex; }
    inline bool isSelectedIndexValid() const { return _selectedIndex < size(); }
    void setSelectedIndex(size_t index);
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool cloneSelectedItem();
    bool removeSelectedItem();

    bool raiseSelectedItemToTop();
    bool raiseSelectedItem();
    bool lowerSelectedItem();
    bool lowerSelectedItemToBottom();

private:
    void onDataChanged(size_t index);

signals:
    void selectedIndexChanged();
    void selectedDataChanged();
};

class AbstractListMultipleSelectionAccessor : public AbstractListAccessor {
    Q_OBJECT

    vectorset<size_t> _selectedIndexes;

public:
    AbstractListMultipleSelectionAccessor(AbstractResourceItem* resourceItem, size_t maxSize);
    ~AbstractListMultipleSelectionAccessor() = default;

    const vectorset<size_t>& selectedIndexes() const { return _selectedIndexes; }
    void setSelectedIndexes(const vectorset<size_t>& selected);
    void setSelectedIndexes(vectorset<size_t>&& selected);
    void clearSelection();
    void selectAll();

    bool cloneSelectedItems();
    bool removeSelectedItems();

    bool raiseSelectedItems();
    bool lowerSelectedItems();

    virtual bool cloneMultipleItems(const vectorset<size_t>& indexes) = 0;
    virtual bool removeMultipleItems(const vectorset<size_t>& indexes) = 0;
    virtual bool moveMultipleItems(const vectorset<size_t>& indexes, int offset) = 0;

signals:
    void selectedIndexesChanged();
};

class AbstractNamedListAccessor : public AbstractListSingleSelectionAccessor {
    Q_OBJECT

public:
    AbstractNamedListAccessor(AbstractResourceItem* resourceItem, size_t maxSize);
    ~AbstractNamedListAccessor() = default;

    virtual QStringList itemNames() const = 0;
    virtual QString itemName(size_t index) const = 0;

    bool editSelected_setName(const idstring& name);

    virtual bool edit_setName(size_t index, const idstring& name) = 0;

    // Will set selectedIndex to new item
    bool addItemWithName(const idstring& name);
    virtual bool addItemWithName(size_t index, const idstring& name) = 0;
    bool cloneSelectedItemWithName(const idstring& name);
    virtual bool cloneItemWithName(size_t index, const idstring& name) = 0;

signals:
    void nameChanged(size_t index);
};

// NOTE: Emits listReset when the parentAccessor's selectedIndex has changed
class AbstractChildListSingleSelectionAccessor : public AbstractListSingleSelectionAccessor {
    Q_OBJECT

    AbstractListSingleSelectionAccessor* const _parentAccessor;
    // Required for AbstractListAccessor::listAboutToChange()
    size_t _parentIndex;

public:
    AbstractChildListSingleSelectionAccessor(AbstractListSingleSelectionAccessor* parentAccessor, size_t maxSize);
    ~AbstractChildListSingleSelectionAccessor() = default;

    AbstractListSingleSelectionAccessor* parentAccessor() const { return _parentAccessor; }
    size_t parentIndex() const { return _parentIndex; }

private slots:
    void onParentSelectedIndexChanged();
    void onDataChanged(size_t parentIndex, size_t index);
    void onListChanged(size_t parentIndex);
    void onListAboutToChange(size_t parentIndex);
    void onItemAdded(size_t parentIndex, size_t index);
    void onItemAboutToBeRemoved(size_t parentIndex, size_t index);
    void onItemMoved(size_t parentIndex, size_t from, size_t to);

signals:
    void dataChanged(size_t parentIndex, size_t index);

    void listChanged(size_t parentIndex);

    void listAboutToChange(size_t parentIndex);
    void itemAdded(size_t parentIndex, size_t index);
    void itemAboutToBeRemoved(size_t parentIndex, size_t index);
    void itemMoved(size_t parentIndex, size_t from, size_t to);
};

// NOTE: Emits listReset when the parentAccessor's selectedIndex has changed
class AbstractChildListMultipleSelectionAccessor : public AbstractListMultipleSelectionAccessor {
    Q_OBJECT

    AbstractListSingleSelectionAccessor* const _parentAccessor;
    // Required for AbstractListAccessor::listAboutToChange()
    size_t _parentIndex;

public:
    AbstractChildListMultipleSelectionAccessor(AbstractListSingleSelectionAccessor* parentAccessor, size_t maxSize);
    ~AbstractChildListMultipleSelectionAccessor() = default;

    AbstractListSingleSelectionAccessor* parentAccessor() const { return _parentAccessor; }
    size_t parentIndex() const { return _parentIndex; }

private slots:
    void onParentSelectedIndexChanged();
    void onDataChanged(size_t parentIndex, size_t index);
    void onListChanged(size_t parentIndex);
    void onListAboutToChange(size_t parentIndex);
    void onItemAdded(size_t parentIndex, size_t index);
    void onItemAboutToBeRemoved(size_t parentIndex, size_t index);
    void onItemMoved(size_t parentIndex, size_t from, size_t to);

signals:
    void dataChanged(size_t parentIndex, size_t index);

    void listChanged(size_t parentIndex);

    void listAboutToChange(size_t parentIndex);
    void itemAdded(size_t parentIndex, size_t index);
    void itemAboutToBeRemoved(size_t parentIndex, size_t index);
    void itemMoved(size_t parentIndex, size_t from, size_t to);
};

template <class T, class ResourceItemT>
class VectorSingleSelectionAccessor : public AbstractListSingleSelectionAccessor {
public:
    using DataT = T;
    using ListT = ::std::vector<T>;
    using index_type = size_t;
    using ArgsT = ::std::tuple<>;
    using SignalArgsT = ArgsT;

    using UndoHelper = ListAndSelectionUndoHelper<VectorSingleSelectionAccessor>;

private:
    ResourceItemT* const _resourceItem;

public:
    VectorSingleSelectionAccessor(ResourceItemT* resourceItem, size_t maxSize);
    ~VectorSingleSelectionAccessor() = default;

    ResourceItemT* resourceItem() const { return static_cast<ResourceItemT*>(AbstractListAccessor::resourceItem()); }

    virtual bool listExists() const final;
    virtual size_t size() const final;

    const std::vector<T>* list() const;

    // may return nullptr
    const T* selectedItem() const
    {
        if (const std::vector<T>* l = list()) {
            if (selectedIndex() < l->size()) {
                return &l->at(selectedIndex());
            }
        }
        return nullptr;
    }

    virtual bool addItem(size_t index) final;
    virtual bool cloneItem(size_t index) final;
    virtual bool removeItem(size_t index) final;
    virtual bool moveItem(size_t from, size_t to) final;

protected:
    template <class, class>
    friend class ListUndoHelper;
    std::vector<T>* getList();
    ArgsT selectedListTuple() const;
};

class AbstractGridAccessor : public QObject {
    Q_OBJECT

public:
    using index_type = upoint;
    using selection_type = upoint_vectorset;

private:
    AbstractResourceItem* const _resourceItem;

    upoint_vectorset _selectedCells;

public:
    AbstractGridAccessor(AbstractResourceItem* resourceItem);
    ~AbstractGridAccessor() = default;

    AbstractResourceItem* resourceItem() const { return _resourceItem; }

    virtual usize size() const = 0;

    const upoint_vectorset& selectedCells() const { return _selectedCells; }
    void setSelectedCells(const upoint_vectorset& selected);
    void setSelectedCells(upoint_vectorset&& selected);
    void clearSelection();

private:
    bool testSelection(const upoint_vectorset& selected) const;

signals:
    void gridReset();
    void gridChanged();

    void gridAboutToBeResized();
    void gridResized();

    void selectedCellsChanged();
};

template <class T, class ResourceItemT>
class NamedListAccessor : public AbstractNamedListAccessor {
public:
    using DataT = T;
    using ListT = ::UnTech::NamedList<T>;
    using index_type = size_t;
    using ArgsT = ::std::tuple<>;
    using SignalArgsT = ArgsT;

    using UndoHelper = ListAndSelectionUndoHelper<NamedListAccessor>;

public:
    NamedListAccessor(ResourceItemT* resourceItem, size_t maxSize);
    ~NamedListAccessor() = default;

    ResourceItemT* resourceItem() const { return static_cast<ResourceItemT*>(AbstractListAccessor::resourceItem()); }

    virtual bool listExists() const final;
    virtual size_t size() const final;

    virtual QStringList itemNames() const final;
    virtual QString itemName(size_t index) const final;

    virtual bool edit_setName(index_type index, const idstring& name) final;

    const NamedList<T>* list() const;

    // may return nullptr
    const T* selectedItem() const
    {
        if (const NamedList<T>* l = list()) {
            if (selectedIndex() < l->size()) {
                return &l->at(selectedIndex());
            }
        }
        return nullptr;
    }

    virtual bool addItem(size_t index) final;
    virtual bool addItemWithName(size_t index, const idstring& name) final;
    virtual bool cloneItem(size_t index) final;
    virtual bool cloneItemWithName(size_t index, const idstring& name) final;
    virtual bool removeItem(size_t index) final;
    virtual bool moveItem(size_t from, size_t to) final;

protected:
    template <class, class>
    friend class ListUndoHelper;
    NamedList<T>* getList();
    ArgsT selectedListTuple() const;
};

template <class T, class ResourceItemT>
class ChildVectorAccessor : public AbstractChildListSingleSelectionAccessor {
public:
    using DataT = T;
    using ListT = ::std::vector<T>;
    using index_type = size_t;
    using ArgsT = ::std::tuple<size_t>;
    using SignalArgsT = ArgsT;

    using UndoHelper = ListAndSelectionUndoHelper<ChildVectorAccessor>;

public:
    ChildVectorAccessor(AbstractListSingleSelectionAccessor* parentAccessor, ResourceItemT* resourceItem, size_t maxSize);
    ~ChildVectorAccessor() = default;

    ResourceItemT* resourceItem() const { return static_cast<ResourceItemT*>(AbstractListAccessor::resourceItem()); }

    const std::vector<T>* childList() const;
    virtual bool listExists() const final;
    virtual size_t size() const final;

    const std::vector<T>* list(size_t parentIndex) const;

    // may return nullptr
    const T* selectedItem() const
    {
        if (const std::vector<T>* l = childList()) {
            if (selectedIndex() < l->size()) {
                return &l->at(selectedIndex());
            }
        }
        return nullptr;
    }

    virtual bool addItem(size_t index) final;
    virtual bool cloneItem(size_t index) final;
    virtual bool removeItem(size_t index) final;
    virtual bool moveItem(size_t from, size_t to) final;

protected:
    template <class, class>
    friend class ListUndoHelper;
    std::vector<T>* getList(size_t parentIndex);
    ArgsT selectedListTuple() const;
};

template <class T, class ResourceItemT>
class ChildVectorMultipleSelectionAccessor : public AbstractChildListMultipleSelectionAccessor {
public:
    using DataT = T;
    using ListT = ::std::vector<T>;
    using index_type = size_t;
    using ArgsT = ::std::tuple<size_t>;
    using SignalArgsT = ArgsT;

    using UndoHelper = ListAndMultipleSelectionUndoHelper<ChildVectorMultipleSelectionAccessor>;

public:
    ChildVectorMultipleSelectionAccessor(AbstractListSingleSelectionAccessor* parentAccessor, ResourceItemT* resourceItem, size_t maxSize);
    ~ChildVectorMultipleSelectionAccessor() = default;

    ResourceItemT* resourceItem() const { return static_cast<ResourceItemT*>(AbstractListAccessor::resourceItem()); }

    const std::vector<T>* childList() const;
    virtual bool listExists() const final;
    virtual size_t size() const final;

    const std::vector<T>* list(size_t parentIndex) const;

    virtual bool addItem(size_t index) final;
    virtual bool cloneItem(size_t index) final;
    virtual bool cloneMultipleItems(const vectorset<size_t>& indexes) final;
    virtual bool removeItem(size_t index) final;
    virtual bool removeMultipleItems(const vectorset<size_t>& indexes) final;

    virtual bool moveItem(size_t from, size_t to) final;
    virtual bool moveMultipleItems(const vectorset<size_t>& indexes, int offset) final;

protected:
    template <class, class>
    friend class ListUndoHelper;
    std::vector<T>* getList(size_t parentIndex);
    ArgsT selectedListTuple() const;
};

}
}
}
