/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "gui-qt/common/helpers.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources::SceneSettings;

const QStringList SceneSettingsTableManager::BGMODE_STRINGS = {
    QStringLiteral("Mode 0"),
    QStringLiteral("Mode 1"),
    QStringLiteral("Mode 1 (bg3 priotity)"),
    QStringLiteral("Mode 2"),
    QStringLiteral("Mode 3"),
    QStringLiteral("Mode 4"),
};

const QStringList SceneSettingsTableManager::LAYER_TYPE_STRINGS = {
    QStringLiteral(""),
    QStringLiteral("Background Image"),
    QStringLiteral("MetaTile Tileset"),
    QStringLiteral("Text Console"),
};

SceneSettingsTableManager::SceneSettingsTableManager(QObject* parent)
    : ListAccessorTableManager(parent)
{
    setItemsMovable(true);

    const auto layerTypeRange = qVariantRange(LAYER_TYPE_STRINGS.size());

    addProperty(tr("Name"), NAME, PropertyType::IDSTRING);
    addProperty(tr("BG Mode"), BG_MODE, PropertyType::COMBO, BGMODE_STRINGS, qVariantRange(BGMODE_STRINGS.size()));
    addProperty(tr("Layer 0 Type"), LAYER_0_TYPE, PropertyType::COMBO, LAYER_TYPE_STRINGS, layerTypeRange);
    addProperty(tr("Layer 1 Type"), LAYER_1_TYPE, PropertyType::COMBO, LAYER_TYPE_STRINGS, layerTypeRange);
    addProperty(tr("Layer 2 Type"), LAYER_2_TYPE, PropertyType::COMBO, LAYER_TYPE_STRINGS, layerTypeRange);
    addProperty(tr("Layer 3 Type"), LAYER_3_TYPE, PropertyType::COMBO, LAYER_TYPE_STRINGS, layerTypeRange);
}

void SceneSettingsTableManager::setSceneSettingsList(SceneSettingsList* ssList)
{
    setAccessor(ssList);
}

inline SceneSettingsList* SceneSettingsTableManager::accessor() const
{
    return static_cast<SceneSettingsList*>(ListAccessorTableManager::accessor());
}

QVariant SceneSettingsTableManager::data(int index, int id) const
{
    auto* ssList = accessor();
    if (ssList == nullptr
        || index < 0
        || (unsigned)index >= ssList->size()) {

        return QVariant();
    }

    const RES::SceneSettingsInput& ssi = ssList->list()->at(index);

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(ssi.name);

    case PropertyId::BG_MODE:
        return int(ssi.bgMode);

    case PropertyId::LAYER_0_TYPE:
        return int(ssi.layerTypes.at(0));

    case PropertyId::LAYER_1_TYPE:
        return int(ssi.layerTypes.at(1));

    case PropertyId::LAYER_2_TYPE:
        return int(ssi.layerTypes.at(2));

    case PropertyId::LAYER_3_TYPE:
        return int(ssi.layerTypes.at(3));
    }

    return QVariant();
}

bool SceneSettingsTableManager::setData(int index, int id, const QVariant& value)
{
    auto* ssList = this->accessor();
    if (ssList == nullptr
        || index < 0
        || (unsigned)index >= ssList->size()) {

        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return ssList->edit_setName(index, value.toString().toStdString());

    case PropertyId::BG_MODE:
        return ssList->edit_setBgMode(index, static_cast<RES::BgMode>(value.toInt()));

    case PropertyId::LAYER_0_TYPE:
        return ssList->edit_setLayerType(index, 0, static_cast<RES::LayerType>(value.toInt()));

    case PropertyId::LAYER_1_TYPE:
        return ssList->edit_setLayerType(index, 1, static_cast<RES::LayerType>(value.toInt()));

    case PropertyId::LAYER_2_TYPE:
        return ssList->edit_setLayerType(index, 2, static_cast<RES::LayerType>(value.toInt()));

    case PropertyId::LAYER_3_TYPE:
        return ssList->edit_setLayerType(index, 3, static_cast<RES::LayerType>(value.toInt()));
    }

    return false;
}
