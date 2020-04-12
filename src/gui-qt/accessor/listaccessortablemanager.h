/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertytablemanager.h"

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class AbstractListAccessor;

// NOTE: ListAccessorTableManager items are moveable by default
class ListAccessorTableManager : public PropertyTableManager {
    Q_OBJECT

private:
    AbstractListAccessor* _accessor;

public:
    explicit ListAccessorTableManager(QObject* parent = nullptr);
    ~ListAccessorTableManager() = default;

protected:
    void setAccessor(AbstractListAccessor* accessor);

public:
    AbstractListAccessor* accessor() const { return _accessor; }

    virtual int rowCount() const final;

    virtual bool canInsertItem() final;
    virtual bool canCloneItem(int index) final;

    virtual bool insertItem(int index) final;
    virtual bool cloneItem(int index) final;
    virtual bool removeItem(int index) final;
    virtual bool moveItem(int from, int to) final;

signals:
    void accessorChanged();

private slots:
    void onListReset();
};

}
}
}
