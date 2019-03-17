/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/call.h"
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

    template <class AccessorT>
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
};
}
}
}
