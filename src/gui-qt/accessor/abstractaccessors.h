/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QObject>

namespace UnTech {
class idstring;
template <class T>
class NamedList;

namespace GuiQt {
namespace Accessor {
template <class T>
class NamedListUndoHelper;

class AbstractListAccessor : public QObject {
    Q_OBJECT

    const size_t _maxSize;
    size_t _selectedIndex;

public:
    using index_type = size_t;

public:
    AbstractListAccessor(QObject* parent, size_t maxSize);
    ~AbstractListAccessor() = default;

    virtual QString typeName() const = 0;

    size_t selectedIndex() const { return _selectedIndex; }
    inline bool isSelectedIndexValid() const { return _selectedIndex < size(); }
    void setSelectedIndex(size_t index);
    void unselectItem() { setSelectedIndex(INT_MAX); }

    virtual size_t size() const = 0;
    size_t maxSize() const { return _maxSize; }

    // Will set selectedIndex to new item
    bool addItem();
    bool cloneSelectedItem();
    bool addItem(size_t index);
    bool cloneItem(size_t index);

    bool removeSelectedItem();

    bool raiseSelectedItemToTop();
    bool raiseSelectedItem();
    bool lowerSelectedItem();
    bool lowerSelectedItemToBottom();

    virtual bool removeItem(size_t index) = 0;
    virtual bool moveItem(size_t from, size_t to) = 0;

protected:
    virtual bool do_addItem(size_t index) = 0;
    virtual bool do_cloneItem(size_t index) = 0;

private:
    void onDataChanged(size_t index);
    void onItemAdded(size_t index);
    void onItemAboutToBeRemoved(size_t index);
    void onItemMoved(size_t from, size_t to);

signals:
    void dataChanged(size_t index);

    void listAboutToChange();
    void listChanged();
    void itemAdded(size_t index);
    void itemAboutToBeRemoved(size_t index);
    void itemMoved(size_t from, size_t to);

    void selectedIndexChanged();
    void selectedDataChanged();
};

class AbstractNamedListAccessor : public AbstractListAccessor {
    Q_OBJECT

public:
    AbstractNamedListAccessor(QObject* parent, size_t maxSize);
    ~AbstractNamedListAccessor() = default;

    virtual QStringList itemNames() const = 0;
    virtual QString itemName(size_t index) const = 0;

    bool editSelected_setName(const idstring& name);

    virtual bool edit_setName(index_type index, const idstring& name) = 0;

    // Will set selectedIndex to new item
    bool addItemWithName(const idstring& name);
    bool addItemWithName(size_t index, const idstring& name);
    bool cloneSelectedItemWithName(const idstring& name);
    bool cloneItemWithName(size_t index, const idstring& name);

protected:
    virtual bool do_addItemWithName(size_t index, const idstring& name) = 0;
    virtual bool do_cloneItemWithName(size_t index, const idstring& name) = 0;

signals:
    void nameChanged(size_t index);
};

template <class T, class ResourceItemT>
class NamedListAccessor : public AbstractNamedListAccessor {
public:
    using DataT = T;
    using ListT = ::UnTech::NamedList<T>;
    using index_type = size_t;

private:
    ResourceItemT* const _resourceItem;

public:
    NamedListAccessor(ResourceItemT* resourceItem, size_t maxSize);
    ~NamedListAccessor() = default;

    ResourceItemT* resourceItem() const { return _resourceItem; }

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

    virtual bool removeItem(size_t index) final;
    virtual bool moveItem(size_t from, size_t to) final;

protected:
    virtual bool do_addItem(size_t index) final;
    virtual bool do_addItemWithName(size_t index, const idstring& name) final;
    virtual bool do_cloneItem(size_t index) final;
    virtual bool do_cloneItemWithName(size_t index, const idstring& name) final;

protected:
    friend class Accessor::NamedListUndoHelper<NamedListAccessor>;
    NamedList<T>* getList();
};

}
}
}
