/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/project.h"
#include "models/resources/scenes.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace Scenes {
class ResourceItem;

namespace RES = UnTech::Resources;

class SceneList : public Accessor::NamedListAccessor<RES::SceneInput, ResourceItem> {
    Q_OBJECT

    using UndoHelper = Accessor::ListAndSelectionUndoHelper<SceneList>;

public:
    SceneList(ResourceItem* resourceItem);
    ~SceneList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool edit_setSceneSettings(index_type index, const idstring& sceneSettings);
    bool edit_setPalette(index_type index, const idstring& palette);
    bool edit_setLayer(index_type index, unsigned layerId, const idstring& layer);
};

}
}
}
}
