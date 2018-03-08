/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "propertymanager.h"
#include <QAction>
#include <QTreeView>

namespace UnTech {
namespace GuiQt {
class PropertyModel;
class PropertyManager;
class PropertyDelegate;

class PropertyView : public QTreeView {
    Q_OBJECT

public:
    PropertyView(QWidget* parent = nullptr);

    void setPropertyManager(PropertyManager* manager);
    PropertyManager* propertyManager() const { return _manager; }

    // MUST NOT call this method
    virtual void setModel(QAbstractItemModel*) final;

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) final;

private:
    QStringList showAddFilenameDialog(const PropertyManager::Property& property);

private slots:
    void onSelectionChanged();

    void onInsertActionTriggered();
    void onRemoveActionTriggered();

private:
    PropertyModel* _model;
    PropertyManager* _manager;
    PropertyDelegate* const _delegate;

    QAction* _insertAction;
    QAction* _removeAction;
};
}
}
