/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "projectsettingspropertymanager.h"
#include "gui-qt/project.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::ProjectSettings;

namespace RES = UnTech::Resources;
namespace MT = UnTech::MetaTiles;

ProjectSettingsPropertyManager::ProjectSettingsPropertyManager(QObject* parent)
    : PropertyListManager(parent)
    , _project(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    addPropertyGroup(tr("Block Settings:"));
    addProperty(tr("Block Size"), BLOCK_SIZE, Type::UNSIGNED, 1024, 64 * 1024);
    addProperty(tr("Block Count"), BLOCK_COUNT, Type::UNSIGNED, 1, 128);
    addPropertyGroup(tr("MetaTile Settings:"));
    addProperty(tr("Max Map Size"), METATILE_MAX_MAP_SIZE, Type::UNSIGNED, 16 * 14, 32 * 1024);
    addProperty(tr("N. MetaTiles"), METATILE_N_METATILES, Type::UNSIGNED, 16, 1024);
}

void ProjectSettingsPropertyManager::setProject(Project* project)
{
    if (_project == project) {
        return;
    }

    if (_project) {
        _project->disconnect(this);
    }
    _project = project;

    setEnabled(_project != nullptr);

    if (_project) {
        connect(_project, &Project::resourceFileSettingsChanged,
                this, &ProjectSettingsPropertyManager::dataChanged);
    }

    emit dataChanged();
}

QVariant ProjectSettingsPropertyManager::data(int id) const
{
    if (_project == nullptr) {
        return QVariant();
    }

    const auto* pro = _project->projectFile();
    Q_ASSERT(pro);

    switch ((PropertyId)id) {
    case BLOCK_SIZE:
        return pro->blockSettings.size;

    case BLOCK_COUNT:
        return pro->blockSettings.count;

    case METATILE_MAX_MAP_SIZE:
        return pro->metaTileEngineSettings.maxMapSize;

    case METATILE_N_METATILES:
        return pro->metaTileEngineSettings.nMetaTiles;
    }

    return QVariant();
}

bool ProjectSettingsPropertyManager::setData(int id, const QVariant& value)
{
    Q_ASSERT(_project);

    switch ((PropertyId)id) {
    case BLOCK_SIZE:
        return editBlockSettings_setSize(value.toUInt());

    case BLOCK_COUNT:
        return editBlockSettings_setCount(value.toUInt());

    case METATILE_MAX_MAP_SIZE:
        return editMetaTileSettings_setMaxMapSize(value.toUInt());

    case METATILE_N_METATILES:
        return editMetaTileSettings_setNMetaTiles(value.toUInt());
    }

    return false;
}
