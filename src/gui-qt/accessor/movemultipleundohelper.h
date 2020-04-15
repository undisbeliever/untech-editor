/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "accessor.h"
#include "models/common/vectorset.h"

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class MoveMultipleUndoHelper {
public:
    template <typename index_type, typename UnaryFunction>
    static void redo(const MoveMultipleDirection direction,
                     const vectorset<index_type>& indexes, const index_type listSize,
                     UnaryFunction moveFunction)
    {
        using MMD = MoveMultipleDirection;

        Q_ASSERT(indexes.empty() == false);

        switch (direction) {
        case MMD::Raise: {
            Q_ASSERT(indexes.front() > 0);
            Q_ASSERT(indexes.back() < listSize);

            for (const index_type& i : indexes) {
                moveFunction(i, i - 1);
            }
            return;
        }

        case MMD::Lower: {
            Q_ASSERT(indexes.front() >= 0);
            Q_ASSERT(indexes.back() + 1 < listSize);

            for (auto it = indexes.rbegin(); it != indexes.rend(); it++) {
                const index_type i = *it;
                moveFunction(i, i + 1);
            }
            return;
        }

        case MMD::RaiseToTop: {
            Q_ASSERT(indexes.front() >= 0);
            Q_ASSERT(indexes.back() < listSize);
            Q_ASSERT(listSize >= indexes.size());

            index_type to = 0;
            for (const index_type& from : indexes) {
                if (from != to) {
                    moveFunction(from, to);
                }
                to++;
            }
            return;
        }

        case MMD::LowerToBottom: {
            Q_ASSERT(indexes.front() >= 0);
            Q_ASSERT(indexes.back() < listSize);
            Q_ASSERT(listSize >= indexes.size());

            index_type to = listSize - 1;
            for (auto it = indexes.rbegin(); it != indexes.rend(); it++) {
                const index_type from = *it;
                if (from != to) {
                    moveFunction(from, to);
                }
                to--;
            }
            return;
        }
        }
    }

    template <typename index_type, typename UnaryFunction>
    static void undo(const MoveMultipleDirection direction,
                     const vectorset<index_type>& indexes, const index_type listSize,
                     UnaryFunction moveFunction)
    {
        using MMD = MoveMultipleDirection;

        Q_ASSERT(indexes.empty() == false);

        switch (direction) {
        case MMD::Raise: {
            Q_ASSERT(indexes.front() > 0);
            Q_ASSERT(indexes.back() < listSize);

            for (auto it = indexes.rbegin(); it != indexes.rend(); it++) {
                const index_type i = *it;
                moveFunction(i - 1, i);
            }
            return;
        }

        case MMD::Lower: {
            Q_ASSERT(indexes.front() >= 0);
            Q_ASSERT(indexes.back() + 1 < listSize);

            for (const index_type i : indexes) {
                moveFunction(i + 1, i);
            }
            return;
        }

        case MMD::RaiseToTop: {
            Q_ASSERT(indexes.front() >= 0);
            Q_ASSERT(indexes.back() < listSize);
            Q_ASSERT(listSize >= indexes.size());

            index_type from = indexes.size() - 1;
            for (auto it = indexes.rbegin(); it != indexes.rend(); it++) {
                const index_type to = *it;
                if (from != to) {
                    moveFunction(from, to);
                }
                from--;
            }
            return;
        }

        case MMD::LowerToBottom: {
            Q_ASSERT(indexes.front() >= 0);
            Q_ASSERT(indexes.back() < listSize);
            Q_ASSERT(listSize >= indexes.size());

            index_type from = listSize - indexes.size();
            for (const index_type to : indexes) {
                if (from != to) {
                    moveFunction(from, to);
                }
                from++;
            }
            return;
        }
        }
    }

    template <typename index_type>
    static bool canMove(const MoveMultipleDirection direction,
                        const vectorset<index_type>& indexes, const index_type listSize)
    {
        using MMD = MoveMultipleDirection;

        if (listSize == 0
            || indexes.empty()
            || indexes.front() < 0
            || indexes.back() >= listSize) {

            return false;
        }

        switch (direction) {
        case MMD::Raise:
            return indexes.front() > 0;

        case MMD::Lower:
            return indexes.back() + 1 < listSize;

        case MMD::RaiseToTop:
            return indexes.back() >= indexes.size();

        case MMD::LowerToBottom:
            return indexes.front() < listSize - indexes.size();
        }

        return false;
    }

    static const char* getText(const MoveMultipleDirection direction)
    {
        using MMD = MoveMultipleDirection;

        switch (direction) {
        case MMD::Raise:
            return "Raise %1";

        case MMD::Lower:
            return "Lower %1";

        case MMD::RaiseToTop:
            return "Raise %1 To Top";

        case MMD::LowerToBottom:
            return "Lower %1 To Bottom";
        }

        return "Move %1";
    }
};
}
}
}
