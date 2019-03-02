/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/call.h"
#include "models/common/vectorset.h"
#include <QtGlobal>
#include <functional>
#include <type_traits>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

struct ListActionStatus {
    bool selectionValid = false;

    bool canAdd = false;
    bool canClone = false;
    bool canRemove = false;

    bool canRaise = false;
    bool canLower = false;

    ListActionStatus() = default;

    ListActionStatus(const ListActionStatus& a, const ListActionStatus& b)
    {
        selectionValid = a.selectionValid | b.selectionValid;
        canAdd = (a.canAdd & b.canAdd) | (a.canAdd & !b.selectionValid) | (!a.selectionValid & b.canAdd);
        canClone = (a.canClone & b.canClone) | (a.canClone & !b.selectionValid) | (!a.selectionValid & b.canClone);
        canRemove = (a.canRemove & b.canRemove) | (a.canRemove & !b.selectionValid) | (!a.selectionValid & b.canRemove);
        canRaise = (a.canRaise & b.canRaise) | (a.canRaise & !b.selectionValid) | (!a.selectionValid & b.canRaise);
        canLower = (a.canLower & b.canLower) | (a.canLower & !b.selectionValid) | (!a.selectionValid & b.canLower);
    }

    template <class... T>
    ListActionStatus(const ListActionStatus& a, const ListActionStatus& b, T... v)
        : ListActionStatus(ListActionStatus(a, b), v...)
    {
    }

    template <size_t N>
    static ListActionStatus mergeArray(const std::array<ListActionStatus, N>& array)
    {
        static_assert(N > 1, "Cannot merge 0 array");

        ListActionStatus s = array.at(0);
        for (size_t i = 1; i < array.size(); i++) {
            s = ListActionStatus(s, array.at(i));
        }

        return s;
    }
};

class ListActionHelper {
public:
    template <class AccessorT, typename... AT>
    static bool canAddToList(AccessorT* a, AT... args)
    {
        using ArgsT = typename AccessorT::ArgsT;
        using ListT = typename AccessorT::ListT;

        auto f = std::mem_fn(&AccessorT::getList);
        const ArgsT listArgs = std::make_tuple(args...);
        const ListT* list = mem_fn_call(f, a, listArgs);

        if (list == nullptr) {
            return false;
        }

        return list->size() < a->maxSize();
    }

    template <class AccessorT,
              typename std::enable_if_t<
                  std::is_member_function_pointer<decltype(&AccessorT::selectedIndex)>::value>* = nullptr>
    static ListActionStatus status(AccessorT* a)
    {
        using ArgsT = typename AccessorT::ArgsT;
        using ListT = typename AccessorT::ListT;
        using index_type = typename AccessorT::index_type;

        if (a == nullptr) {
            return ListActionStatus();
        }

        auto f = std::mem_fn(&AccessorT::getList);
        const ArgsT listArgs = a->selectedListTuple();
        const ListT* list = mem_fn_call(f, a, listArgs);

        if (list == nullptr) {
            return ListActionStatus();
        }

        const index_type list_size = list->size();
        const index_type index = a->selectedIndex();
        Q_ASSERT(list_size >= 0);

        ListActionStatus ret;
        ret.selectionValid = index >= 0 && index < list_size;

        ret.canAdd = list_size < a->maxSize();
        ret.canClone = ret.selectionValid && ret.canAdd;
        ret.canRemove = ret.selectionValid;

        ret.canRaise = ret.selectionValid && index != 0;
        ret.canLower = ret.selectionValid && index + 1 < list_size;

        return ret;
    }

    template <class AccessorT,
              typename std::enable_if_t<
                  std::is_member_function_pointer<decltype(&AccessorT::selectedIndexes)>::value>* = nullptr>
    static ListActionStatus status(AccessorT* a)
    {
        using ArgsT = typename AccessorT::ArgsT;
        using ListT = typename AccessorT::ListT;
        using index_type = typename AccessorT::index_type;

        if (a == nullptr) {
            return ListActionStatus();
        }

        const vectorset<index_type>& indexes = a->selectedIndexes();

        auto f = std::mem_fn(&AccessorT::getList);
        const ArgsT listArgs = a->selectedListTuple();
        const ListT* list = mem_fn_call(f, a, listArgs);

        if (list == nullptr) {
            return ListActionStatus();
        }

        const index_type maxSize = a->maxSize();
        const index_type list_size = list->size();
        Q_ASSERT(list_size >= 0);

        ListActionStatus ret;
        ret.selectionValid = !indexes.empty() && indexes.front() >= 0 && indexes.back() < list_size;

        ret.canAdd = list_size < maxSize;
        ret.canClone = ret.selectionValid && list_size + indexes.size() <= maxSize;
        ret.canRemove = ret.selectionValid;

        ret.canRaise = ret.selectionValid && indexes.front() > 0;
        ret.canLower = ret.selectionValid && indexes.back() + 1 < list_size;

        return ret;
    }
};
}
}
}
