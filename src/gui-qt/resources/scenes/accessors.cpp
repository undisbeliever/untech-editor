/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "models/project/project.h"

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::Resources::Scenes;

template <>
const NamedList<RES::SceneInput>* Accessor::NamedListAccessor<RES::SceneInput, ResourceItem>::list() const
{
    const auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->resourceScenes.scenes;
}

template <>
NamedList<RES::SceneInput>* Accessor::NamedListAccessor<RES::SceneInput, ResourceItem>::getList()
{
    auto* projectFile = resourceItem()->project()->projectFile();
    Q_ASSERT(projectFile);
    return &projectFile->resourceScenes.scenes;
}

SceneList::SceneList(ResourceItem* resourceItem)
    : NamedListAccessor(resourceItem, 255)
{
}

QString SceneList::typeName() const
{
    return tr("Scene Setting");
}

QString SceneList::typeNamePlural() const
{
    return tr("Scene Settings");
}

bool SceneList::edit_setSceneSettings(NamedListAccessor::index_type index, const idstring& sceneSettings)
{
    return UndoHelper(this).editField(
        index, sceneSettings,
        tr("Edit Scene Settings"),
        [](DataT& s) -> idstring& { return s.sceneSettings; });
}

bool SceneList::edit_setPalette(NamedListAccessor::index_type index, const idstring& palette)
{
    return UndoHelper(this).editField(
        index, palette,
        tr("Edit Palette"),
        [](DataT& s) -> idstring& { return s.palette; });
}

bool SceneList::edit_setLayer(NamedListAccessor::index_type index, unsigned layerId, const idstring& layer)
{
    return UndoHelper(this).editField(
        index, layer,
        tr("Edit Layer"),
        [=](DataT& s) -> idstring& { return s.layers.at(layerId); });
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<RES::SceneInput, ResourceItem>;
