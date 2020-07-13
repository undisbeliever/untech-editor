/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/listaccessortablemanager.h"

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
namespace InteractiveTiles {
class FunctionTableList;

class FunctionTableManager : public Accessor::ListAccessorTableManager {
    Q_OBJECT

public:
    enum PropertyId {
        NAME,
    };

public:
    explicit FunctionTableManager(QObject* parent = nullptr);
    ~FunctionTableManager() = default;

    void setFunctionTableList(FunctionTableList* itfList);

    virtual QVariant data(int index, int id) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

private:
    FunctionTableList* accessor() const;
};

class FixedFunctionTableManager final : public PropertyTableManager {
    Q_OBJECT

    using PropertyId = FunctionTableManager::PropertyId;

public:
    explicit FixedFunctionTableManager(QObject* parent = nullptr);
    ~FixedFunctionTableManager() = default;

    virtual int rowCount() const final;
    virtual QVariant data(int index, int id) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;
};

}
}
}
}
