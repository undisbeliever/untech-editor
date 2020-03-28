/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/staticresourcelist.h"
#include "models/resources/scenes.h"

namespace UnTech {
namespace GuiQt {
class StaticResourceList;

namespace Resources {
namespace SceneSettings {
class SceneSettingsList;

namespace RES = UnTech::Resources;

class ResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

    using ResourceScenes = UnTech::Resources::ResourceScenes;

public:
    ResourceItem(StaticResourceList* list, unsigned index);
    ~ResourceItem() = default;

    StaticResourceList* resourceList() const { return static_cast<StaticResourceList*>(_list); }

    SceneSettingsList* sceneSettingsList() const { return _sceneSettingsList; }

protected:
    virtual bool compileResource(ErrorList& err) final;

private:
    SceneSettingsList* const _sceneSettingsList;
};

}
}
}
}
