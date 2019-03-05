/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "listactionhelper.h"
#include "multilistactions.h"
#include "gui-qt/common/properties/propertytablemanager.h"
#include "gui-qt/common/properties/propertytablemodel.h"
#include <QTreeView>
#include <type_traits>

namespace UnTech {
namespace GuiQt {
class PropertyDelegate;

namespace Accessor {

class MultipleSelectionTableView : public QTreeView {
    Q_OBJECT

private:
    MultiListActions* const _actions;
    QList<AbstractListMultipleSelectionAccessor*> _accessors;

    PropertyDelegate* const _delegate;
    QMenu* const _selectedContextMenu;
    QMenu* const _noSelectionContextMenu;

    PropertyTableModel* _model;

public:
    explicit MultipleSelectionTableView(QWidget* parent = nullptr);
    ~MultipleSelectionTableView() = default;

    void setPropertyManagers(const QList<PropertyTableManager*>& managers, const QStringList& columns);

    // MUST NOT call this method
    virtual void setModel(QAbstractItemModel*) final;

    const MultiListActions* viewActions() const { return _actions; }
    PropertyTableModel* propertyTableModel() const { return _model; }

    QMenu* selectedContextmenu() { return _selectedContextMenu; }
    QMenu* noSelectionContextMenu() { return _noSelectionContextMenu; }

    void rebuildMenus();

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) final;

public:
    // Order of parameters MUST MATCH setPropertyManagers
    //
    // NOTE: This function DOES NOT set the managers's resourceItem/accessor.
    // You will need to do that yourself.
    template <class... AccessorsT>
    void setAccessors(AccessorsT*... accessors)
    {
        constexpr int nAccessors = sizeof...(AccessorsT);

        auto for_each_accessor = [=](auto&& f) { for_each(f, accessors...); };
        auto for_each_accessor_i = [=](auto&& f) { for_each_i(f, accessors...); };

        disconnectAll();
        _accessors = { accessors... };

        _actions->setAccessors(_accessors);

        if (_model->managers().size() != nAccessors) {
            qWarning("Invalid number of accessors");
            return;
        }

        if (_accessors.contains(nullptr)) {
            return;
        }

        if (_model == nullptr) {
            qWarning("Must call setPropertyManagers before setAccessors");
            return;
        }

        auto onSelectionModelChanged = [=]() {
            Q_ASSERT(_model);

            for_each_accessor_i([&](auto* accessor, int aId) {
                using Accessor = typename std::remove_pointer<decltype(accessor)>::type;
                using index_type = typename Accessor::index_type;
                std::vector<index_type> selected;

                for (const auto& index : this->selectionModel()->selectedRows()) {
                    auto mi = _model->toManagerIdAndIndex(index);
                    if (mi.first == aId) {
                        selected.push_back(mi.second);
                    }
                }
                accessor->setSelectedIndexes(std::move(selected));
            });
        };

        auto onSelectedIndexesChanged = [=]() {
            Q_ASSERT(_model);

            QItemSelection sel;
            for_each_accessor_i([&](auto* accessor, int aId) {
                if (accessor) {
                    for (auto si : accessor->selectedIndexes()) {
                        QModelIndex index = _model->toModelIndex(aId, si);
                        if (index.isValid()) {
                            sel.select(index, index);
                        }
                    }
                }
            });

            selectionModel()->select(
                sel, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

            // BUGFIX: Sometimes the view will not hightlight the new selection
            viewport()->update();
        };
        onSelectedIndexesChanged();

        for_each_accessor([&](auto* accessor) {
            using AT = typename std::remove_pointer<decltype(accessor)>::type;

            connect(accessor, &AT::selectedIndexesChanged,
                    this, onSelectedIndexesChanged);
            connect(accessor, &AT::listReset,
                    this, onSelectedIndexesChanged);
        });

        connect(this->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, onSelectionModelChanged);
    }

private:
    void disconnectAll();

    template <class AccessorT>
    inline static QUndoStack* getUndoStack(AccessorT* a)
    {
        return a->resourceItem()->undoStack();
    }

    template <class AccessorT, class... Extras>
    inline static QUndoStack* getUndoStack(AccessorT* a, Extras*...)
    {
        return a->resourceItem()->undoStack();
    }

    template <typename Functor, class AccessorT>
    inline static int for_each_call(Functor&& functor, AccessorT* accessor)
    {
        functor(accessor);
        return 0;
    }

    template <typename Functor, class... AccessorsT>
    inline static void for_each(Functor&& functor, AccessorsT*... accessors)
    {
        int l[] = { for_each_call(functor, accessors)... };
        Q_UNUSED(l);
    }

    template <typename Functor, class AccessorT>
    inline static void for_each_i_call(Functor&& functor, int aId, AccessorT* accessor)
    {
        functor(accessor, aId);
    }

    template <typename Functor, class AccessorT, class... Extras>
    inline static void for_each_i_call(Functor&& functor, int aId, AccessorT* accessor, Extras*... extras)
    {
        for_each_i_call(functor, aId, accessor);
        for_each_i_call(functor, aId + 1, extras...);
    }

    template <typename Functor, class... AccessorsT>
    inline static void for_each_i(Functor&& functor, AccessorsT*... accessors)
    {
        for_each_i_call(functor, 0, accessors...);
    }
};
}
}
}
