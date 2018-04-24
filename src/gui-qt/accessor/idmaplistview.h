/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "actions.h"
#include "idmaplistmodel.h"
#include "idmapundohelper.h"
#include <QListView>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class IdmapListView : public QListView {
    Q_OBJECT

public:
    explicit IdmapListView(QWidget* parent = nullptr);
    ~IdmapListView() = default;

    // MUST NOT call this method
    virtual void setModel(QAbstractItemModel*) final;

    const IdmapActions& idmapActions() const { return _actions; }
    IdmapListModel* idmapListModel() const { return _model; }

    template <class AccessorT>
    void setAccessor(AccessorT* accessor)
    {
        using DataT = typename AccessorT::DataT;

        _actions.add->disconnect(this);
        _actions.clone->disconnect(this);
        _actions.rename->disconnect(this);
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
                const idmap<DataT>* map = accessor->map();
                const DataT* item = accessor->selectedItem();
                if (map && item) {
                    const idstring& id = accessor->selectedId();
                    QModelIndex index = _model->toModelIndex(id);
                    if (index.isValid()) {
                        // do not update selection on rename, model will take care of it
                        setCurrentIndex(index);
                    }
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

                _actions.add->setEnabled(map != nullptr);
                _actions.clone->setEnabled(map != nullptr && item != nullptr);
                _actions.rename->setEnabled(map != nullptr && item != nullptr);
                _actions.remove->setEnabled(map != nullptr && item != nullptr);
            };
            onSelectedItemChanged();

            connect(accessor, &AccessorT::selectedItemChanged,
                    this, onSelectedItemChanged);
            connect(accessor, &AccessorT::mapChanged,
                    this, onSelectedItemChanged);

            connect(selectionModel(), &QItemSelectionModel::selectionChanged,
                    this, [=]() {
                        idstring id = _model->toIdstring(currentIndex());
                        accessor->setSelectedId(id);
                    });

            connect(_actions.add, &QAction::triggered,
                    this, [=]() {
                        if (_actions.add->isEnabled() == false) {
                            return;
                        }
                        idstring newId = addIdstringDialog(accessor->typeName());
                        IdmapAndSelectionUndoHelper<AccessorT>(accessor).addItem(newId);
                    });

            connect(_actions.clone, &QAction::triggered,
                    this, [=]() {
                        if (_actions.clone->isEnabled() == false) {
                            return;
                        }
                        const idstring& id = accessor->selectedId();
                        idstring newId = cloneIdstringDialog(id, accessor->typeName());
                        IdmapAndSelectionUndoHelper<AccessorT>(accessor).cloneItem(id, newId);
                    });

            connect(_actions.rename, &QAction::triggered,
                    this, [=]() {
                        if (_actions.rename->isEnabled() == false) {
                            return;
                        }
                        const idstring& oldId = accessor->selectedId();
                        idstring newId = renameIdstringDialog(oldId, accessor->typeName());
                        IdmapAndSelectionUndoHelper<AccessorT>(accessor).renameItem(oldId, newId);
                    });

            connect(_actions.remove, &QAction::triggered,
                    this, [=]() {
                        IdmapAndSelectionUndoHelper<AccessorT>(accessor).removeSelectedItem();
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
    IdmapActions _actions;
    IdmapListModel* const _model;

    QObject* _accessor;
};
}
}
}
