/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QObject>
#include <functional>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class SelectedIdmapItemHelper {

public:
    template <class AccessorT, class SelectorT>
    static void buildAndConnectSlots(AccessorT* accessor, SelectorT* selector)
    {
        QObject::connect(
            accessor, &AccessorT::itemAboutToBeRemoved,
            selector, [selector](const idstring& id) {
                if (selector->selectedId() == id) {
                    selector->unselectItem();
                }
            });
        QObject::connect(
            accessor, &AccessorT::itemRenamed,
            selector, [selector](const idstring& oldId, const idstring& newId) {
                if (selector->selectedId() == oldId) {
                    selector->setSelectedId(newId);
                }
            });
    }

    template <class AccessorT>
    static void buildAndConnectSlots(AccessorT* accessor)
    {
        buildAndConnectSlots(accessor, accessor);
    }
};
}
}
}
