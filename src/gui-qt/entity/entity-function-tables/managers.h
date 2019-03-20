/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/listaccessortablemanager.h"

namespace UnTech {
namespace GuiQt {
namespace Entity {
class EntityFunctionTableList;

class EntityFunctionTablesManager : public Accessor::ListAccessorTableManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        ENTITY_STRUCT,
        EXPORT_ORDER,
        PARAMETER_TYPE,
        COMMENT
    };
    constexpr static int N_PROPERTIES = 4;

    const static QStringList PARAMETER_TYPE_STRINGS;

public:
    explicit EntityFunctionTablesManager(QObject* parent = nullptr);
    ~EntityFunctionTablesManager() = default;

    void setFunctionTableList(EntityFunctionTableList* ftList);

    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const;

    virtual QVariant data(int index, int id) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

private:
    EntityFunctionTableList* accessor() const;
};

}
}
}
