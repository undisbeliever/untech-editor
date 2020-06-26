/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "nestedlistaccessors.h"
#include "nestedlistundohelper.h"

namespace UnTech {
namespace GuiQt {
namespace Accessor {

template <class P, class C, class RI>
NestedNlvMulitpleSelectionAccessor<P, C, RI>::NestedNlvMulitpleSelectionAccessor(RI* resourceItem, size_t maxSize)
    : AbstractNestedListMultipleSelectionAccessor(resourceItem, maxSize)
{
}

template <class P, class C, class RI>
size_t NestedNlvMulitpleSelectionAccessor<P, C, RI>::parentListSize() const
{
    const ParentListT* pl = parentList();
    if (pl == nullptr) {
        return 0;
    }
    return pl->size();
}

template <class P, class C, class RI>
size_t NestedNlvMulitpleSelectionAccessor<P, C, RI>::childListSize(size_t parentIndex) const
{
    const ParentListT* pl = parentList();
    if (pl == nullptr) {
        return 0;
    }
    if (parentIndex < 0 || parentIndex >= pl->size()) {
        return 0;
    }
    return childList(pl->at(parentIndex)).size();
}

template <class P, class C, class RI>
bool NestedNlvMulitpleSelectionAccessor<P, C, RI>::addItem(size_t parentIndex, size_t childIndex)
{
    return UndoHelper(this).addItem(parentIndex, childIndex);
}

template <class P, class C, class RI>
bool NestedNlvMulitpleSelectionAccessor<P, C, RI>::cloneItem(size_t parentIndex, size_t childIndex)
{
    return UndoHelper(this).cloneItem(parentIndex, childIndex);
}

template <class P, class C, class RI>
bool NestedNlvMulitpleSelectionAccessor<P, C, RI>::removeItem(size_t parentIndex, size_t childIndex)
{
    return UndoHelper(this).removeItem(parentIndex, childIndex);
}

template <class P, class C, class RI>
bool NestedNlvMulitpleSelectionAccessor<P, C, RI>::moveItem(index_type fromParentIndex, index_type fromChildIndex,
                                                            index_type toParentIndex, index_type toChildIndex)
{
    return UndoHelper(this).moveItem(fromParentIndex, fromChildIndex, toParentIndex, toChildIndex);
}

template <class P, class C, class RI>
bool NestedNlvMulitpleSelectionAccessor<P, C, RI>::cloneMultipleItems(const vectorset<index_pair_t>& indexes)
{
    return UndoHelper(this).cloneMultipleItems(indexes);
}

template <class P, class C, class RI>
bool NestedNlvMulitpleSelectionAccessor<P, C, RI>::removeMultipleItems(const vectorset<index_pair_t>& indexes)
{
    return UndoHelper(this).removeMultipleItems(indexes);
}

template <class P, class C, class RI>
bool NestedNlvMulitpleSelectionAccessor<P, C, RI>::moveMultipleItems(const vectorset<index_pair_t>& indexes, const MoveMultipleDirection direction)
{
    return UndoHelper(this).moveMultipleItems(indexes, direction);
}

template <class P, class C, class RI>
bool NestedNlvMulitpleSelectionAccessor<P, C, RI>::moveMultipleItemsToChildList(const vectorset<index_pair_t>& indexes, size_t targetParentIndex)
{
    return UndoHelper(this).moveMultipleItemsToChildList(indexes, targetParentIndex);
}

template <class P, class C, class RI>
bool NestedNlvMulitpleSelectionAccessor<P, C, RI>::setSelected_Ptr(const void* ptr)
{
    if (const ParentListT* pList = parentList()) {
        for (size_t pi = 0; pi < pList->size(); pi++) {
            const ChildListT& cList = childList(pList->at(pi));

            for (size_t ci = 0; ci < cList.size(); ci++) {
                if (&cList.at(ci) == ptr) {
                    setSelectedIndex(pi, ci);
                    return true;
                }
            }
        }
    }

    clearSelection();
    return false;
}

}
}
}
