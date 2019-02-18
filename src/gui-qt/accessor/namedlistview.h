/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "actions.h"
#include "namedlistmodel.h"
#include "namedlistundohelper.h"
#include "models/common/namedlist.h"
#include <QListView>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class NamedListView : public QListView {
    Q_OBJECT

public:
    explicit NamedListView(QWidget* parent = nullptr);
    ~NamedListView() = default;

    // MUST NOT call this method
    virtual void setModel(QAbstractItemModel*) final;

    const NamedListActions& namedListActions() const { return _actions; }
    NamedListModel* idmapListModel() const { return _model; }

    QMenu* selectedContextMenu() const { return _selectedContextMenu; }
    QMenu* noSelectionContextMenu() const { return _noSelectionContextMenu; }

    template <class AccessorT>
    void setAccessor(AccessorT* accessor)
    {
        using DataT = typename AccessorT::DataT;
        using ListT = UnTech::NamedList<DataT>;
        using index_type = typename AccessorT::index_type;

        _actions.add->disconnect(this);
        _actions.clone->disconnect(this);
        _actions.rename->disconnect(this);
        _actions.raise->disconnect(this);
        _actions.lower->disconnect(this);
        _actions.remove->disconnect(this);

        if (auto* sm = selectionModel()) {
            sm->disconnect(this);
        }

        if (_accessor) {
            _accessor->disconnect(this);
        }
        _accessor = accessor;

        _actions.updateText(AccessorT::typeName());
        _actions.disableAll();

        _model->setAccessor(accessor);

        if (accessor) {
            auto onSelectedItemChanged = [=]() {
                const ListT* list = accessor->list();
                const index_type selectedIndex = accessor->selectedIndex();

                if (list) {
                    QModelIndex index = _model->toModelIndex(selectedIndex);
                    setCurrentIndex(index);
                }
                else {
                    setCurrentIndex(QModelIndex());
                }

                // BUGFIX: Update view to redraw the previously selected item.
                //
                // I have no idea why the view glitches and both the current
                // and previous selection are highlighted but scheduling an
                // update fixes it.
                viewport()->update();

                bool selectionValid = selectedIndex < list->size();
                bool canAdd = list->size() < AccessorT::max_size;

                _actions.add->setEnabled(canAdd);
                _actions.clone->setEnabled(selectionValid && canAdd);
                _actions.rename->setEnabled(selectionValid);
                _actions.raise->setEnabled(selectionValid && selectedIndex > 0);
                _actions.lower->setEnabled(selectionValid && selectedIndex + 1 < list->size());
                _actions.remove->setEnabled(selectionValid);
            };
            onSelectedItemChanged();

            connect(accessor, &AccessorT::selectedIndexChanged,
                    this, onSelectedItemChanged);
            connect(accessor, &AccessorT::listChanged,
                    this, onSelectedItemChanged);

            connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                    this, [=]() {
                        index_type index = _model->toIndex(currentIndex());
                        accessor->setSelectedIndex(index);
                    });

            connect(_actions.add, &QAction::triggered,
                    this, [=]() {
                        if (_actions.add->isEnabled() == false) {
                            return;
                        }
                        const idstring name = addIdstringDialog(accessor->typeName());
                        if (name.isValid()) {
                            NamedListAndSelectionUndoHelper<AccessorT>(accessor).addItem(name);
                        }
                    });

            connect(_actions.clone, &QAction::triggered,
                    this, [=]() {
                        if (_actions.clone->isEnabled() == false) {
                            return;
                        }
                        const auto index = accessor->selectedIndex();
                        const ListT* list = accessor->list();
                        Q_ASSERT(list);
                        const auto& item = list->at(index);
                        const idstring name = cloneIdstringDialog(item.name, accessor->typeName());
                        if (name.isValid()) {
                            NamedListAndSelectionUndoHelper<AccessorT>(accessor).cloneSelectedItem(name);
                        }
                    });

            connect(_actions.rename, &QAction::triggered,
                    this, [=]() {
                        if (_actions.rename->isEnabled() == false) {
                            return;
                        }
                        const ListT* list = accessor->list();
                        Q_ASSERT(list);
                        const auto& item = list->at(accessor->selectedIndex());
                        const idstring name = renameIdstringDialog(item.name, accessor->typeName());
                        if (name.isValid()) {
                            NamedListAndSelectionUndoHelper<AccessorT>(accessor).renameSelectedItem(name);
                        }
                    });

            connect(_actions.raise, &QAction::triggered,
                    this, [=]() {
                        NamedListAndSelectionUndoHelper<AccessorT>(accessor).raiseSelectedItem();
                    });

            connect(_actions.lower, &QAction::triggered,
                    this, [=]() {
                        NamedListAndSelectionUndoHelper<AccessorT>(accessor).lowerSelectedItem();
                    });

            connect(_actions.remove, &QAction::triggered,
                    this, [=]() {
                        NamedListAndSelectionUndoHelper<AccessorT>(accessor).removeSelectedItem();
                    });
        }
    }

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) final;

private:
    idstring addIdstringDialog(const QString& typeName);
    idstring cloneIdstringDialog(const idstring& id, const QString& typeName);
    idstring renameIdstringDialog(const idstring& oldId, const QString& typeName);

private:
    NamedListActions _actions;
    NamedListModel* const _model;
    QMenu* const _selectedContextMenu;
    QMenu* const _noSelectionContextMenu;

    QObject* _accessor;
};
}
}
}
