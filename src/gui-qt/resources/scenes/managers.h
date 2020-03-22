/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/listaccessortablemanager.h"
#include "models/common/idstring.h"
#include "models/common/optional.h"
#include "models/resources/scenes.h"

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace Scenes {
class SceneList;

namespace RES = UnTech::Resources;

class SceneTableManager : public Accessor::ListAccessorTableManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        SCENE_SETTINGS,
        PALETTE,
        LAYER_0_TYPE,
        LAYER_0,
        LAYER_1_TYPE,
        LAYER_1,
        LAYER_2_TYPE,
        LAYER_2,
        LAYER_3_TYPE,
        LAYER_3,
    };
    constexpr static int N_PROPERTIES = 11;

    const static QStringList LAYER_TYPE_SHORT_STRINGS;

public:
    explicit SceneTableManager(QObject* parent = nullptr);
    ~SceneTableManager() = default;

    void setSceneList(SceneList* sList);

    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const final;

    virtual QVariant data(int index, int id) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

private:
    QString printableLayerString(const idstring& layer) const;
    optional<RES::SceneSettingsInput> getSceneSettings(const idstring& sceneSettings) const;

private:
    SceneList* accessor() const;
};

}
}
}
}
