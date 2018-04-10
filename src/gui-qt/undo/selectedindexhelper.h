/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QObject>
#include <functional>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace Undo {

class SelectedIndexHelper {

    template <class SelectorT, typename T, typename index_type>
    struct SlotBuilder;

    template <class SelectorT, typename... ArgsT, typename index_type>
    struct SlotBuilder<SelectorT, std::tuple<ArgsT...>, index_type> {

        static auto buildOnItemAdded(SelectorT* selector)
        {
            return [=](const ArgsT&... args, index_type index) {
                if (std::tie(args...) == selector->selectedListTuple()) {
                    index_type sel = selector->selectedIndex();

                    if (sel >= index) {
                        selector->setSelectedIndex(sel + 1);
                    }
                }
            };
        }

        static auto buildOnItemAboutToBeRemoved(SelectorT* selector)
        {
            return [=](const ArgsT&... args, index_type index) {
                if (std::tie(args...) == selector->selectedListTuple()) {
                    index_type sel = selector->selectedIndex();

                    if (sel == index) {
                        selector->unselectItem();
                    }
                    else if (sel > index) {
                        selector->setSelectedIndex(sel - 1);
                    }
                }
            };
        }

        static auto buildOnItemMoved(SelectorT* selector)
        {
            return [=](const ArgsT&... args, index_type from, index_type to) {
                if (std::tie(args...) == selector->selectedListTuple()) {
                    index_type sel = selector->selectedIndex();

                    if (sel == from) {
                        selector->setSelectedIndex(to);
                    }
                    else if (sel > from && sel <= to) {
                        selector->setSelectedIndex(sel - 1);
                    }
                    else if (sel >= to && sel < from) {
                        selector->setSelectedIndex(sel + 1);
                    }
                }
            };
        }
    };

public:
    template <class AccessorT, class SelectorT>
    static void buildAndConnectSlots(AccessorT* accessor, SelectorT* selector)
    {
        using ArgsT = typename AccessorT::ArgsT;
        using index_type = typename AccessorT::index_type;

        using SB = SlotBuilder<SelectorT, ArgsT, index_type>;

        QObject::connect(accessor, &AccessorT::itemAdded,
                         selector, SB::buildOnItemAdded(selector));

        QObject::connect(accessor, &AccessorT::itemAboutToBeRemoved,
                         selector, SB::buildOnItemAboutToBeRemoved(selector));

        QObject::connect(accessor, &AccessorT::itemMoved,
                         selector, SB::buildOnItemMoved(selector));
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
