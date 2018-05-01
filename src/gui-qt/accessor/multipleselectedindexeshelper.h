/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/vectorset.h"
#include <QObject>
#include <functional>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class MultipleSelectedIndexesHelper {

    template <class SelectorT, typename T, typename index_type>
    struct SlotBuilder;

    template <class SelectorT, typename... SignalArgsT, typename index_type>
    struct SlotBuilder<SelectorT, std::tuple<SignalArgsT...>, index_type> {

        static auto buildOnItemAdded(SelectorT* selector)
        {
            return [=](const SignalArgsT... args, index_type newIndex) {
                if (std::tie(args...) == selector->selectedListTuple()) {
                    const vectorset<index_type>& selected = selector->selectedIndexes();

                    std::vector<index_type> newSel;
                    newSel.reserve(selected.size());

                    for (const index_type& i : selected) {
                        if (i >= newIndex) {
                            newSel.push_back(i + 1);
                        }
                        else {
                            newSel.push_back(i);
                        }
                    }

                    selector->setSelectedIndexes(std::move(newSel));
                }
            };
        }

        static auto buildOnItemAboutToBeRemoved(SelectorT* selector)
        {
            return [=](const SignalArgsT... args, index_type indexToRemove) {
                if (std::tie(args...) == selector->selectedListTuple()) {
                    const vectorset<index_type>& selected = selector->selectedIndexes();

                    std::vector<index_type> newSel;
                    newSel.reserve(selected.size());

                    for (const index_type& i : selected) {
                        if (i < indexToRemove) {
                            newSel.push_back(i);
                        }
                        else if (i > indexToRemove) {
                            newSel.push_back(i - 1);
                        }
                    }

                    selector->setSelectedIndexes(std::move(newSel));
                }
            };
        }

        static auto buildOnItemMoved(SelectorT* selector)
        {
            return [=](const SignalArgsT... args, index_type from, index_type to) {
                if (std::tie(args...) == selector->selectedListTuple()) {
                    const vectorset<index_type>& selected = selector->selectedIndexes();

                    std::vector<index_type> newSel;
                    newSel.reserve(selected.size());

                    for (const index_type& i : selected) {
                        if (i == from) {
                            newSel.push_back(to);
                        }
                        else if (i > from && i <= to) {
                            newSel.push_back(i - 1);
                        }
                        else if (i >= to && i < from) {
                            newSel.push_back(i + 1);
                        }
                        else {
                            newSel.push_back(i);
                        }
                    }

                    selector->setSelectedIndexes(std::move(newSel));
                }
            };
        }
    };

public:
    template <class AccessorT, class SelectorT>
    static void buildAndConnectSlots(AccessorT* accessor, SelectorT* selector)
    {
        using SignalArgsT = typename AccessorT::SignalArgsT;
        using index_type = typename AccessorT::index_type;

        using SB = SlotBuilder<SelectorT, SignalArgsT, index_type>;

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
