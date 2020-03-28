/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/listaccessortablemanager.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ActionPoints {
class ActionPointFunctionsList;

class ActionPointFunctionsManager : public Accessor::ListAccessorTableManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        MANUALLY_INVOKED,
    };
    constexpr static int N_PROPERTIES = 2;

    const static QStringList PARAMETER_TYPE_STRINGS;

public:
    explicit ActionPointFunctionsManager(QObject* parent = nullptr);
    ~ActionPointFunctionsManager() = default;

    void setAccessor(ActionPointFunctionsList* ftList);

    virtual QVariant data(int index, int id) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

private:
    ActionPointFunctionsList* accessor() const;
};

}
}
}
}
