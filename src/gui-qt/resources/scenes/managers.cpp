/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/scene-settings/resourceitem.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project-data.h"
#include "models/project/project.h"
#include "models/resources/scene-bgmode.hpp"

using namespace UnTech;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources::Scenes;

const QStringList SceneTableManager::LAYER_TYPE_SHORT_STRINGS = {
    QStringLiteral(""),
    QStringLiteral("BI:"),
    QStringLiteral("MT:"),
    QStringLiteral("Text"),
};

SceneTableManager::SceneTableManager(QObject* parent)
    : ListAccessorTableManager(parent)
{
    setItemsMovable(true);

    addProperty(tr("Name"), NAME, PropertyType::IDSTRING);
    addProperty(tr("Scene Settings"), SCENE_SETTINGS, PropertyType::COMBO);
    addProperty(tr("Palette"), PALETTE, PropertyType::COMBO);
    addProperty(tr("BG1"), LAYER_0_TYPE, PropertyType::NOT_EDITABLE);
    addProperty(tr("(layer 0)"), LAYER_0, PropertyType::COMBO);
    addProperty(tr("BG2"), LAYER_1_TYPE, PropertyType::NOT_EDITABLE);
    addProperty(tr("(layer 1)"), LAYER_1, PropertyType::COMBO);
    addProperty(tr("BG3"), LAYER_2_TYPE, PropertyType::NOT_EDITABLE);
    addProperty(tr("(layer 2)"), LAYER_2, PropertyType::COMBO);
    addProperty(tr("BG4"), LAYER_3_TYPE, PropertyType::NOT_EDITABLE);
    addProperty(tr("(layer 3)"), LAYER_3, PropertyType::COMBO);
}

void SceneTableManager::setSceneList(SceneList* sList)
{
    setAccessor(sList);
}

inline SceneList* SceneTableManager::accessor() const
{
    return static_cast<SceneList*>(ListAccessorTableManager::accessor());
}

optional<UnTech::Resources::SceneSettingsInput> SceneTableManager::getSceneSettings(const idstring& sceneSettings) const
{
    if (accessor() == nullptr) {
        return {};
    }
    const auto* project = accessor()->resourceItem()->project();
    Q_ASSERT(project);
    const auto* projectFile = project->projectFile();
    Q_ASSERT(projectFile);

    const auto& ssData = project->projectData().sceneSettings();
    if (!ssData) {
        return {};
    }

    const auto it = ssData->nameIndexMap.find(sceneSettings);
    if (it == ssData->nameIndexMap.end()) {
        return {};
    }

    return projectFile->resourceScenes.settings.at(it->second);
}

void SceneTableManager::updateParameters(int index, int id, QVariant& param1, QVariant& param2) const
{
    Q_UNUSED(param2);

    auto* sList = accessor();
    if (sList == nullptr
        || index < 0
        || (unsigned)index >= sList->size()) {

        return;
    }

    auto* project = sList->resourceItem()->project();

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        break;

    case PropertyId::SCENE_SETTINGS:
        param1 = convertNameListWithBlank(project->projectFile()->resourceScenes.settings);
        break;

    case PropertyId::PALETTE:
        param1 = convertNameListWithBlank(project->projectFile()->palettes);
        break;

    case PropertyId::LAYER_0_TYPE:
    case PropertyId::LAYER_1_TYPE:
    case PropertyId::LAYER_2_TYPE:
    case PropertyId::LAYER_3_TYPE:
        break;

    case PropertyId::LAYER_0:
    case PropertyId::LAYER_1:
    case PropertyId::LAYER_2:
    case PropertyId::LAYER_3: {
        static_assert(int(PropertyId::LAYER_3) - int(PropertyId::LAYER_0) + 2 == 4 * 2);

        const unsigned layerId = (unsigned(id) - unsigned(PropertyId::LAYER_0)) / 2;

        const RES::SceneInput& scene = sList->list()->at(index);
        const auto& sceneSettings = getSceneSettings(scene.sceneSettings);
        if (!sceneSettings) {
            break;
        }

        switch (sceneSettings->layerTypes.at(layerId)) {
        case RES::LayerType::None:
        case RES::LayerType::TextConsole:
            param1 = QStringList({ QStringLiteral() });
            break;

        case RES::LayerType::BackgroundImage: {
            const unsigned bitDepth = RES::bitDepthForLayer(sceneSettings->bgMode, layerId);

            const auto& backgroundImages = project->projectFile()->backgroundImages;
            QStringList qList;
            qList.reserve(backgroundImages.size());
            for (auto& item : backgroundImages) {
                if (item.bitDepth == bitDepth) {
                    qList.append(QString::fromStdString(item.name));
                }
            }
            param1 = std::move(qList);
            break;
        }

        case RES::LayerType::MetaTileTileset: {
            const unsigned bitDepth = RES::bitDepthForLayer(sceneSettings->bgMode, layerId);

            const auto& metaTileTilesets = project->projectFile()->metaTileTilesets;
            QStringList qList;
            qList.reserve(metaTileTilesets.size());
            for (auto& efi : metaTileTilesets) {
                if (auto& mt = efi.value) {
                    if (mt->animationFrames.bitDepth == bitDepth) {
                        qList.append(QString::fromStdString(mt->name));
                    }
                }
            }
            param1 = std::move(qList);
            break;
        }
        }
    }
    }
}

QVariant SceneTableManager::data(int index, int id) const
{
    auto* sList = accessor();
    if (sList == nullptr
        || index < 0
        || (unsigned)index >= sList->size()) {

        return QVariant();
    }

    const RES::SceneInput& scene = sList->list()->at(index);

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(scene.name);

    case PropertyId::SCENE_SETTINGS:
        return QString::fromStdString(scene.sceneSettings);

    case PropertyId::PALETTE:
        return QString::fromStdString(scene.palette);

    case PropertyId::LAYER_0_TYPE:
    case PropertyId::LAYER_1_TYPE:
    case PropertyId::LAYER_2_TYPE:
    case PropertyId::LAYER_3_TYPE: {
        static_assert(int(PropertyId::LAYER_3_TYPE) - int(PropertyId::LAYER_0_TYPE) + 2 == 4 * 2);

        const unsigned layerId = (unsigned(id) - unsigned(PropertyId::LAYER_0_TYPE)) / 2;

        const auto& sceneSettings = getSceneSettings(scene.sceneSettings);
        if (!sceneSettings) {
            return QStringLiteral(u"⚠⚠");
        }
        return LAYER_TYPE_SHORT_STRINGS.at(int(sceneSettings->layerTypes.at(layerId)));
    }

    case PropertyId::LAYER_0:
    case PropertyId::LAYER_1:
    case PropertyId::LAYER_2:
    case PropertyId::LAYER_3: {
        static_assert(int(PropertyId::LAYER_3) - int(PropertyId::LAYER_0) + 2 == 4 * 2);
        const unsigned layerId = (unsigned(id) - unsigned(PropertyId::LAYER_0)) / 2;
        return QString::fromStdString(scene.layers.at(layerId));
    }
    }

    return QVariant();
}

bool SceneTableManager::setData(int index, int id, const QVariant& value)
{
    auto* sList = this->accessor();
    if (sList == nullptr
        || index < 0
        || (unsigned)index >= sList->size()) {

        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return sList->edit_setName(index, value.toString().toStdString());

    case PropertyId::SCENE_SETTINGS:
        return sList->edit_setSceneSettings(index, value.toString().toStdString());

    case PropertyId::PALETTE:
        return sList->edit_setPalette(index, value.toString().toStdString());

    case PropertyId::LAYER_0:
        return sList->edit_setLayer(index, 0, value.toString().toStdString());

    case PropertyId::LAYER_1:
        return sList->edit_setLayer(index, 1, value.toString().toStdString());

    case PropertyId::LAYER_2:
        return sList->edit_setLayer(index, 2, value.toString().toStdString());

    case PropertyId::LAYER_3:
        return sList->edit_setLayer(index, 3, value.toString().toStdString());

    case PropertyId::LAYER_0_TYPE:
    case PropertyId::LAYER_1_TYPE:
    case PropertyId::LAYER_2_TYPE:
    case PropertyId::LAYER_3_TYPE:
        return false;
    }

    return false;
}
