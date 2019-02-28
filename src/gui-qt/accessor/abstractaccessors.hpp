/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractaccessors.h"
#include "namedlistundohelper.h"

namespace UnTech {
namespace GuiQt {
namespace Accessor {

template <class T, class RI>
NamedListAccessor<T, RI>::NamedListAccessor(RI* resourceItem, size_t maxSize)
    : AbstractNamedListAccessor(resourceItem, maxSize)
    , _resourceItem(resourceItem)
{
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
    return NamedListAndSelectionUndoHelper<NamedListAccessor<T, RI>>(this).renameItem(index, name);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::do_addItem(size_t index)
{
    return NamedListUndoHelper<NamedListAccessor<T, RI>>(this).addItem(index);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::do_addItemWithName(size_t index, const UnTech::idstring& name)
{
    return NamedListUndoHelper<NamedListAccessor<T, RI>>(this).addItem(index, name);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::do_cloneItem(size_t index)
{
    return NamedListUndoHelper<NamedListAccessor<T, RI>>(this).cloneItem(index);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::do_cloneItemWithName(size_t index, const idstring& name)
{
    return NamedListUndoHelper<NamedListAccessor<T, RI>>(this).cloneItem(index, name);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::removeItem(size_t index)
{
    return NamedListAndSelectionUndoHelper<NamedListAccessor<T, RI>>(this).removeItem(index);
}

template <class T, class RI>
bool NamedListAccessor<T, RI>::moveItem(size_t from, size_t to)
{
    return NamedListAndSelectionUndoHelper<NamedListAccessor<T, RI>>(this).moveItem(from, to);
}

}
}
}
