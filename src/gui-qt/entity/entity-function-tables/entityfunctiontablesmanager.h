/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertytablemanager.h"

namespace UnTech {
namespace GuiQt {
namespace Entity {
class EntityFunctionTableList;

class EntityFunctionTablesManager : public PropertyTableManager {
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

    virtual int rowCount() const final;
    virtual QVariant data(int index, int id) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

    virtual bool canInsertItem() final;
    virtual bool canCloneItem(int index) final;

    virtual bool insertItem(int index) final;
    virtual bool cloneItem(int index) final;
    virtual bool removeItem(int index) final;
    virtual bool moveItem(int from, int to) final;

private:
    EntityFunctionTableList* _ftList;
};

}
}
}
