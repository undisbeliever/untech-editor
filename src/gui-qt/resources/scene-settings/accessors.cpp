/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "gui-qt/project.h"
#include "models/project/project.h"

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Resources::SceneSettings;

template <>
const NamedList<RES::SceneSettingsInput>* Accessor::NamedListAccessor<RES::SceneSettingsInput, ResourceItem>::list() const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->resourceScenes.settings;
}

template <>
NamedList<RES::SceneSettingsInput>* Accessor::NamedListAccessor<RES::SceneSettingsInput, ResourceItem>::getList()
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->resourceScenes.settings;
}

SceneSettingsList::SceneSettingsList(ResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, 255)
{
}

QString SceneSettingsList::typeName() const
{
    return tr("Scene Setting");
}

QString SceneSettingsList::typeNamePlural() const
{
    return tr("Scene Settings");
}

bool SceneSettingsList::edit_setBgMode(NamedListAccessor::index_type index, RES::BgMode bgMode)
{
    return UndoHelper(this).editField(
        index, bgMode,
        tr("Edit BG Mode"),
        [](DataT& s) -> RES::BgMode& { return s.bgMode; });
}

bool SceneSettingsList::edit_setLayerType(NamedListAccessor::index_type index, unsigned layerId, RES::LayerType layerType)
{
    return UndoHelper(this).editField(
        index, layerType,
        tr("Edit Layer Type"),
        [=](DataT& s) -> RES::LayerType& { return s.layerTypes.at(layerId); });
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<RES::SceneSettingsInput, ResourceItem>;
