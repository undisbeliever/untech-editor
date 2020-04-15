/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "accessor.h"
#include "models/common/namedlist.h"
#include "models/common/vectorset-upoint.h"
#include "models/common/vectorset.h"
#include <QObject>
#include <optional>

namespace UnTech {
namespace GuiQt {
class AbstractResourceItem;

namespace Accessor {
template <class T, class U>
class NestedListUndoHelper;

template <class T>
class NestedListAndMultipleSelectionUndoHelper;

struct ListActionStatus;

class AbstractNestedListMultipleSelectionAccessor : public QObject {
    Q_OBJECT

    using index_type = size_t;
    using index_pair_t = std::pair<size_t, size_t>;

    AbstractResourceItem* const _resourceItem;
    const size_t _maxChildListSize;
    vectorset<index_pair_t> _selectedIndexes;

    // Only exists if all indexes in `selectedIndexes` point to the same parentIndex.
    std::optional<index_type> _parentIndex;

public:
    AbstractNestedListMultipleSelectionAccessor(AbstractResourceItem* resourceItem, size_t maxChildListSize);
    ~AbstractNestedListMultipleSelectionAccessor() = default;

    size_t maxChildListSize() const { return _maxChildListSize; }

    virtual QString typeName() const = 0;
    virtual QString typeNamePlural() const = 0;

    AbstractResourceItem* resourceItem() const { return _resourceItem; }

    const vectorset<index_pair_t>& selectedIndexes() const { return _selectedIndexes; }
    std::optional<index_type> parentIndex() const { return _parentIndex; }

    void setSelectedIndex(const index_type parentIndex, const index_type childIndex);
    void setSelectedIndexes(const vectorset<index_pair_t>& selected);
    void setSelectedIndexes(vectorset<index_pair_t>&& selected);
    void clearSelection();
    void selectAll();

    ListActionStatus listActionStatus(const index_type parentIndexForAddAction) const;

    bool addItem(size_t parentIndex);

    bool cloneSelectedItems();
    bool removeSelectedItems();
    bool raiseSelectedItemsToTop();
    bool raiseSelectedItems();
    bool lowerSelectedItems();
    bool lowerSelectedItemsToBottom();

    bool moveSelectedItemsToChildList(size_t destinationParentIndex);

    virtual size_t parentListSize() const = 0;
    virtual size_t childListSize(size_t parentIndex) const = 0;

    virtual bool addItem(size_t parentIndex, size_t index) = 0;
    virtual bool cloneItem(size_t parentIndex, size_t index) = 0;
    virtual bool removeItem(size_t parentIndex, size_t index) = 0;
    virtual bool moveItem(index_type fromParentIndex, index_type fromChildIndex, index_type toParentIndex, index_type toChildIndex) = 0;

    virtual bool cloneMultipleItems(const vectorset<index_pair_t>& indexes) = 0;
    virtual bool removeMultipleItems(const vectorset<index_pair_t>& indexes) = 0;
    virtual bool moveMultipleItems(const vectorset<index_pair_t>& indexes, const MoveMultipleDirection direction) = 0;
    virtual bool moveMultipleItemsToChildList(const vectorset<index_pair_t>& indexes, size_t destinationParentIndex) = 0;

signals:
    void selectedIndexesChanged();

    // Emitted when the underlying list has changed (ie, ResourceItem is reverted to old state)
    void listReset();

    void dataChanged(size_t parentIndex, size_t childIndex);

    void listAboutToChange(size_t parentIndex);
    void listChanged(size_t parentIndex);
    void itemAdded(size_t parentIndex, size_t childIndex);
    void itemAboutToBeRemoved(size_t parentIndex, size_t childIndex);
    void itemMoved(index_type fromParentIndex, index_type fromChildIndex, index_type toParentIndex, index_type toChildIndex);

private:
    void updateParentIndex();
};

template <class PT, class CT, class ResourceItemT>
// Nested NameList/Vector Multiple Seclection Accessor
class NestedNlvMulitpleSelectionAccessor : public AbstractNestedListMultipleSelectionAccessor {
public:
    using DataT = CT;
    using ParentT = PT;
    using ParentListT = ::UnTech::NamedList<ParentT>;
    using ChildListT = ::std::vector<DataT>;
    using index_type = size_t;
    using index_pair_t = std::pair<size_t, size_t>;

    using UndoHelper = NestedListAndMultipleSelectionUndoHelper<NestedNlvMulitpleSelectionAccessor>;

public:
    NestedNlvMulitpleSelectionAccessor(ResourceItemT* resourceItem, size_t maxSize);
    ~NestedNlvMulitpleSelectionAccessor() = default;

    ResourceItemT* resourceItem() const { return static_cast<ResourceItemT*>(AbstractNestedListMultipleSelectionAccessor::resourceItem()); }

    const ParentListT* parentList() const;
    static const ChildListT& childList(const ParentT& parentItem);

    virtual size_t parentListSize() const final;
    virtual size_t childListSize(size_t parentIndex) const final;

    inline bool addItem(size_t parentIndex) { return AbstractNestedListMultipleSelectionAccessor::addItem(parentIndex); }

    virtual bool addItem(size_t parentIndex, size_t index) final;
    virtual bool cloneItem(size_t parentIndex, size_t index) final;
    virtual bool removeItem(size_t parentIndex, size_t index) final;
    virtual bool moveItem(index_type fromParentIndex, index_type fromChildIndex, index_type toParentIndex, index_type toChildIndex) final;

    virtual bool cloneMultipleItems(const vectorset<index_pair_t>& indexes) final;
    virtual bool removeMultipleItems(const vectorset<index_pair_t>& indexes) final;
    virtual bool moveMultipleItems(const vectorset<index_pair_t>& indexes, const MoveMultipleDirection direction) final;
    virtual bool moveMultipleItemsToChildList(const vectorset<index_pair_t>& indexes, size_t targetParentIndex) final;

protected:
    template <class, class>
    friend class NestedListUndoHelper;
    ParentListT* getParentList();
    static ChildListT& getChildList(ParentT& parentItem);
};

}
}
}
