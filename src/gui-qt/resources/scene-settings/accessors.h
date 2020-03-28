/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "models/resources/scenes.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace SceneSettings {
class ResourceItem;

namespace RES = UnTech::Resources;

class SceneSettingsList : public Accessor::NamedListAccessor<RES::SceneSettingsInput, ResourceItem> {
    Q_OBJECT

    using UndoHelper = Accessor::ListAndSelectionUndoHelper<SceneSettingsList>;

public:
    SceneSettingsList(ResourceItem* resourceItem);
    ~SceneSettingsList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool edit_setBgMode(index_type index, UnTech::Resources::BgMode bgMode);
    bool edit_setLayerType(index_type index, unsigned layerId, RES::LayerType layerType);
};

}
}
}
}
