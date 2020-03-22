/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/listaccessortablemanager.h"

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace SceneSettings {
class SceneSettingsList;

class SceneSettingsTableManager : public Accessor::ListAccessorTableManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        BG_MODE,
        LAYER_0_TYPE,
        LAYER_1_TYPE,
        LAYER_2_TYPE,
        LAYER_3_TYPE,
    };
    constexpr static int N_PROPERTIES = 6;

    const static QStringList BGMODE_STRINGS;
    const static QStringList LAYER_TYPE_STRINGS;

public:
    explicit SceneSettingsTableManager(QObject* parent = nullptr);
    ~SceneSettingsTableManager() = default;

    void setSceneSettingsList(SceneSettingsList* ssList);

    virtual QVariant data(int index, int id) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

private:
    SceneSettingsList* accessor() const;
};

}
}
}
}
