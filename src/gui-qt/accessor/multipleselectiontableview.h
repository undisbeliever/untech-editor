/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "actions.h"
#include "listactionhelper.h"
#include "listandmultipleselectionundohelper.h"
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
    MultiTableViewActions _actions;
    QList<QObject*> _accessors;

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

    const MultiTableViewActions& viewActions() const { return _actions; }
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
        constexpr size_t nAccessors = sizeof...(AccessorsT);

        auto for_each_accessor = [=](auto&& f) { for_each(f, accessors...); };
        auto for_each_accessor_i = [=](auto&& f) { for_each_i(f, accessors...); };

        disconnectAll();
        _accessors = { accessors... };

        if (_actions.add.size() != nAccessors
            || _model->managers().size() != nAccessors) {

            qWarning("Invalid number of accessors");
            return;
        }

        for_each_accessor_i([this](auto* a, int aId) {
            using AT = typename std::remove_pointer<decltype(a)>::type;
            _actions.add.at(aId)->setText(tr("Add %1").arg(AT::typeName()));
        });

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
            Q_ASSERT(_actions.add.size() == nAccessors);

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

            // update action status
            std::array<ListActionStatus, nAccessors> status = { ListActionHelper::status(accessors)... };
            ListActionStatus selected = ListActionStatus::mergeArray(status);

            for (int i = 0; i < _actions.add.size(); i++) {
                _actions.add.at(i)->setEnabled(status.at(i).canAdd);
            }
            _actions.clone->setEnabled(selected.canClone);
            _actions.raise->setEnabled(selected.canRaise);
            _actions.lower->setEnabled(selected.canLower);
            _actions.remove->setEnabled(selected.canRemove);
        };
        onSelectedIndexesChanged();

        for_each_accessor([&](auto* accessor) {
            using AT = typename std::remove_pointer<decltype(accessor)>::type;

            connect(accessor, &AT::selectedIndexesChanged,
                    this, onSelectedIndexesChanged);
            connect(accessor, &AT::selectedListChanged,
                    this, onSelectedIndexesChanged);
        });

        connect(this->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, onSelectionModelChanged);

        for_each_accessor_i([&](auto* accessor, int aId) {
            Q_ASSERT(accessor);
            Q_ASSERT(aId < _accessors.size());
            Q_ASSERT(aId < _actions.add.size());

            connect(_actions.add.at(aId), &QAction::triggered,
                    this, [=]() {
                        for_each_accessor([](auto* a) {
                            a->clearSelection();
                        });

                        using AT = typename std::remove_pointer<decltype(accessor)>::type;
                        ListAndMultipleSelectionUndoHelper<AT>(accessor).addItemToSelectedList();
                    });
        });

        connect(_actions.clone, &QAction::triggered,
                this, [=]() {
                    if (_actions.clone->isEnabled() == false) {
                        return;
                    }
                    QUndoStack* undoStack = getUndoStack(accessors...);

                    undoStack->beginMacro(tr("Clone"));

                    for_each_accessor([&](auto* a) {
                        using AT = typename std::remove_pointer<decltype(a)>::type;
                        ListAndMultipleSelectionUndoHelper<AT>(a).cloneSelectedItems();
                    });
                    undoStack->endMacro();
                });

        connect(_actions.raise, &QAction::triggered,
                this, [=]() {
                    if (_actions.raise->isEnabled() == false) {
                        return;
                    }
                    QUndoStack* undoStack = getUndoStack(accessors...);

                    undoStack->beginMacro(tr("Raise"));

                    for_each_accessor([&](auto* a) {
                        using AT = typename std::remove_pointer<decltype(a)>::type;
                        ListAndMultipleSelectionUndoHelper<AT>(a).raiseSelectedItems();
                    });
                    undoStack->endMacro();
                });

        connect(_actions.lower, &QAction::triggered,
                this, [=]() {
                    if (_actions.lower->isEnabled() == false) {
                        return;
                    }
                    QUndoStack* undoStack = getUndoStack(accessors...);

                    undoStack->beginMacro(tr("Lower"));
                    for_each_accessor([&](auto* a) {
                        using AT = typename std::remove_pointer<decltype(a)>::type;
                        ListAndMultipleSelectionUndoHelper<AT>(a).lowerSelectedItems();
                    });
                    undoStack->endMacro();
                });

        connect(_actions.remove, &QAction::triggered,
                this, [=]() {
                    if (_actions.remove->isEnabled() == false) {
                        return;
                    }
                    QUndoStack* undoStack = getUndoStack(accessors...);

                    undoStack->beginMacro(tr("Remove"));
                    for_each_accessor([&](auto* a) {
                        using AT = typename std::remove_pointer<decltype(a)>::type;
                        ListAndMultipleSelectionUndoHelper<AT>(a).removeSelectedItems();
                    });
                    undoStack->endMacro();
                });
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
