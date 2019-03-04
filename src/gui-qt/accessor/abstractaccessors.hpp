/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractaccessors.h"
#include "listandmultipleselectionundohelper.h"
#include "listundohelper.h"

namespace UnTech {
namespace GuiQt {
namespace Accessor {

template <class T, class RI>
VectorSingleSelectionAccessor<T, RI>::VectorSingleSelectionAccessor(RI* resourceItem, size_t maxSize)
    : AbstractListSingleSelectionAccessor(resourceItem, maxSize)
    , _resourceItem(resourceItem)
{
}

template <class T, class RI>
std::tuple<> VectorSingleSelectionAccessor<T, RI>::selectedListTuple() const
{
    return std::make_tuple();
}

template <class T, class RI>
bool VectorSingleSelectionAccessor<T, RI>::listExists() const
{
    return list() != nullptr;
}

template <class T, class RI>
size_t VectorSingleSelectionAccessor<T, RI>::size() const
{
    if (const std::vector<T>* l = list()) {
        return l->size();
    }
    return 0;
}

template <class T, class RI>
bool VectorSingleSelectionAccessor<T, RI>::do_addItem(size_t index)
{
    return ListUndoHelper<VectorSingleSelectionAccessor<T, RI>>(this).addItem(index);
}

template <class T, class RI>
bool VectorSingleSelectionAccessor<T, RI>::do_cloneItem(size_t index)
{
    return ListUndoHelper<VectorSingleSelectionAccessor<T, RI>>(this).cloneItem(index);
}

template <class T, class RI>
bool VectorSingleSelectionAccessor<T, RI>::removeItem(size_t index)
{
    return ListUndoHelper<VectorSingleSelectionAccessor<T, RI>>(this).removeItem(index);
}

template <class T, class RI>
bool VectorSingleSelectionAccessor<T, RI>::moveItem(size_t from, size_t to)
{
    return ListUndoHelper<VectorSingleSelectionAccessor<T, RI>>(this).moveItem(from, to);
}

template <class T, class RI>
NamedListAccessor<T, RI>::NamedListAccessor(RI* resourceItem, size_t maxSize)
    : AbstractNamedListAccessor(resourceItem, maxSize)
{
}

template <class T, class RI>
std::tuple<> NamedListAccessor<T, RI>::selectedListTuple() const
{
    return std::make_tuple();
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::listExists() const
{
    return list() != nullptr;
}

template <class T, class RI>
size_t NamedListAccessor<T, RI>::size() const
{
    if (const NamedList<T>* l = list()) {
        return l->size();
    }
    return 0;
}

template <class T, class RI>
QStringList NamedListAccessor<T, RI>::itemNames() const
{
    QStringList qsl;
    if (const NamedList<T>* nl = list()) {
        qsl.reserve(nl->size());
        std::transform(nl->begin(), nl->end(), std::back_inserter(qsl),
                       [](const T& i) { return QString::fromStdString(i.name); });
    }
    return qsl;
}

template <class T, class RI>
QString NamedListAccessor<T, RI>::itemName(size_t index) const
{
    if (auto* l = list()) {
        if (index < l->size()) {
            return QString::fromStdString(l->at(index).name);
        }
    }
    return QString();
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::edit_setName(NamedListAccessor::index_type index, const UnTech::idstring& name)
{
    return ListUndoHelper<NamedListAccessor<T, RI>>(this).editField(
        index, name,
        tr("Edit name"),
        [](DataT& d) -> idstring& { return d.name; },
        [](NamedListAccessor* a, index_type i) { emit a->nameChanged(i); });
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::do_addItem(size_t index)
{
    return ListUndoHelper<NamedListAccessor<T, RI>>(this).addItem(index);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::do_addItemWithName(size_t index, const UnTech::idstring& name)
{
    T newItem;
    newItem.name = name;

    return ListUndoHelper<NamedListAccessor<T, RI>>(this).addItem(index, newItem);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::do_cloneItem(size_t index)
{
    return ListUndoHelper<NamedListAccessor<T, RI>>(this).cloneItem(index);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::do_cloneItemWithName(size_t index, const idstring& name)
{
    if (auto* item = selectedItem()) {
        T newItem = *item;
        newItem.name = name;

        QString text = tr("Clone %1").arg(typeName());
        return ListUndoHelper<NamedListAccessor<T, RI>>(this).addItem(index, newItem, text);
    }
    return false;
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::removeItem(size_t index)
{
    return ListUndoHelper<NamedListAccessor<T, RI>>(this).removeItem(index);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::moveItem(size_t from, size_t to)
{
    return ListUndoHelper<NamedListAccessor<T, RI>>(this).moveItem(from, to);
}

template <class T, class RI>
ChildVectorAccessor<T, RI>::ChildVectorAccessor(AbstractListSingleSelectionAccessor* parentAccessor, RI* resourceItem, size_t maxSize)
    : AbstractChildListSingleSelectionAccessor(parentAccessor, maxSize)
{
    Q_ASSERT(AbstractListAccessor::resourceItem() == resourceItem);
}

template <class T, class RI>
const std::vector<T>* ChildVectorAccessor<T, RI>::childList() const
{
    return list(parentIndex());
}

template <class T, class RI>
bool ChildVectorAccessor<T, RI>::listExists() const
{
    return childList() != nullptr;
}

template <class T, class RI>
size_t ChildVectorAccessor<T, RI>::size() const
{
    if (const std::vector<T>* l = childList()) {
        return l->size();
    }
    return 0;
}

template <class T, class RI>
std::tuple<size_t> ChildVectorAccessor<T, RI>::selectedListTuple() const
{
    return std::make_tuple(parentIndex());
}

template <class T, class RI>
bool ChildVectorAccessor<T, RI>::do_addItem(size_t index)
{
    return ListUndoHelper<ChildVectorAccessor<T, RI>>(this).addItem(index);
}

template <class T, class RI>
bool ChildVectorAccessor<T, RI>::do_cloneItem(size_t index)
{
    return ListUndoHelper<ChildVectorAccessor<T, RI>>(this).cloneItem(index);
}

template <class T, class RI>
bool ChildVectorAccessor<T, RI>::removeItem(size_t index)
{
    return ListUndoHelper<ChildVectorAccessor<T, RI>>(this).removeItem(index);
}

template <class T, class RI>
bool ChildVectorAccessor<T, RI>::moveItem(size_t from, size_t to)
{
    return ListUndoHelper<ChildVectorAccessor<T, RI>>(this).moveItem(from, to);
}

template <class T, class RI>
ChildVectorMultipleSelectionAccessor<T, RI>::ChildVectorMultipleSelectionAccessor(AbstractListSingleSelectionAccessor* parentAccessor, RI* resourceItem, size_t maxSize)
    : AbstractChildListMultipleSelectionAccessor(parentAccessor, maxSize)
{
    Q_ASSERT(AbstractListAccessor::resourceItem() == resourceItem);
}

template <class T, class RI>
const std::vector<T>* ChildVectorMultipleSelectionAccessor<T, RI>::childList() const
{
    return list(parentIndex());
}

template <class T, class RI>
bool ChildVectorMultipleSelectionAccessor<T, RI>::listExists() const
{
    return childList() != nullptr;
}

template <class T, class RI>
size_t ChildVectorMultipleSelectionAccessor<T, RI>::size() const
{
    if (auto* l = childList()) {
        return l->size();
    }
    return 0;
}

template <class T, class RI>
std::tuple<size_t> ChildVectorMultipleSelectionAccessor<T, RI>::selectedListTuple() const
{
    return std::make_tuple(parentIndex());
}

template <class T, class RI>
bool ChildVectorMultipleSelectionAccessor<T, RI>::do_addItem(size_t index)
{
    return ListUndoHelper<ChildVectorMultipleSelectionAccessor<T, RI>>(this).addItem(index);
}

template <class T, class RI>
bool ChildVectorMultipleSelectionAccessor<T, RI>::do_cloneItem(size_t index)
{
    return ListUndoHelper<ChildVectorMultipleSelectionAccessor<T, RI>>(this).cloneItem(index);
}

template <class T, class RI>
bool ChildVectorMultipleSelectionAccessor<T, RI>::do_cloneMultipleItems(const vectorset<size_t>& indexes)
{
    return ListAndMultipleSelectionUndoHelper<ChildVectorMultipleSelectionAccessor<T, RI>>(this).cloneMultipleItems(indexes);
}

template <class T, class RI>
bool ChildVectorMultipleSelectionAccessor<T, RI>::removeItem(size_t index)
{
    return ListUndoHelper<ChildVectorMultipleSelectionAccessor<T, RI>>(this).removeItem(index);
}

template <class T, class RI>
bool ChildVectorMultipleSelectionAccessor<T, RI>::removeMultipleItems(const vectorset<size_t>& indexes)
{
    return ListAndMultipleSelectionUndoHelper<ChildVectorMultipleSelectionAccessor<T, RI>>(this).removeMultipleItems(indexes);
}

template <class T, class RI>
bool ChildVectorMultipleSelectionAccessor<T, RI>::moveItem(size_t from, size_t to)
{
    return ListUndoHelper<ChildVectorMultipleSelectionAccessor<T, RI>>(this).moveItem(from, to);
}

template <class T, class RI>
bool ChildVectorMultipleSelectionAccessor<T, RI>::moveMultipleItems(const vectorset<size_t>& indexes, int offset)
{
    return ListAndMultipleSelectionUndoHelper<ChildVectorMultipleSelectionAccessor<T, RI>>(this).moveMultipleItems(indexes, offset);
}

}
}
}
