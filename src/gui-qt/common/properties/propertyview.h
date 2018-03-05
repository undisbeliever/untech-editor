/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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

private:
    PropertyModel* _model;
    PropertyManager* _manager;
    PropertyDelegate* const _delegate;
};
}
}
