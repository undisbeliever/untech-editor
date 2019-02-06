/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>
#include <QToolBar>
#include <QTreeView>

namespace UnTech {
namespace GuiQt {
struct Property;
class PropertyTableModel;
class PropertyTableManager;
class PropertyDelegate;

/*
 * The PropertyTableView is a View for the PropertyTableModel that includes
 * insert/clone/remove/move actions via a context menu and exposed QActions.
 *
 * This View is only able to manipulate one table row at a time. If you want to
 * be able to select multiple rows, please use QTreeView + custom code instead.
 */
class PropertyTableView : public QTreeView {
    Q_OBJECT

public:
    PropertyTableView(QWidget* parent = nullptr);

    void setPropertyManagers(const QList<PropertyTableManager*>& managers, const QStringList& columns);
    void setPropertyManager(PropertyTableManager* manager);

    void setSelectedRow(PropertyTableManager* manager, int row);

    // MUST NOT call this method
    virtual void setModel(QAbstractItemModel*) final;

    void populateToolBar(QToolBar* toolBar) const;

    QAction* insertAction() const { return _insertAction; }
    QAction* cloneAction() const { return _cloneAction; }
    QAction* removeAction() const { return _removeAction; }
    QAction* raiseAction() const { return _raiseAction; }
    QAction* lowerAction() const { return _lowerAction; }
    QAction* raiseToTopAction() const { return _raiseToTopAction; }
    QAction* lowerToBottomAction() const { return _lowerToBottomAction; }

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) final;
    virtual void keyPressEvent(QKeyEvent* event) final;

private:
    void setPropertyModel(PropertyTableModel* model);

    void moveModelRow(const QModelIndex& index, int destRow);

private slots:
    void onSelectionChanged();

    void onInsertActionTriggered();
    void onCloneActionTriggered();
    void onRemoveActionTriggered();
    void onRaiseActionTriggered();
    void onLowerActionTriggered();
    void onRaiseToTopActionTriggered();
    void onLowerToBottomActionTriggered();

private:
    PropertyTableModel* _model;
    PropertyDelegate* const _delegate;

    QAction* _insertAction;
    QAction* _cloneAction;
    QAction* _removeAction;
    QAction* _raiseAction;
    QAction* _lowerAction;
    QAction* _raiseToTopAction;
    QAction* _lowerToBottomAction;
};
}
}
