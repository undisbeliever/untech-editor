/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcefilepropertymanager.h"
#include "editresourcefilesettingscommands.h"
#include "gui-qt/resources/resourceproject.h"

using namespace UnTech::GuiQt::Resources;

namespace RES = UnTech::Resources;
namespace MT = UnTech::MetaTiles;

ResourceFilePropertyManager::ResourceFilePropertyManager(QObject* parent)
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

void ResourceFilePropertyManager::setProject(ResourceProject* project)
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
        connect(_project, &ResourceProject::resourceFileSettingsChanged,
                this, &ResourceFilePropertyManager::dataChanged);
    }

    emit dataChanged();
}

QVariant ResourceFilePropertyManager::data(int id) const
{
    if (_project == nullptr) {
        return QVariant();
    }

    const RES::ResourcesFile* res = _project->resourcesFile();
    Q_ASSERT(res);

    switch ((PropertyId)id) {
    case BLOCK_SIZE:
        return res->blockSettings.size;

    case BLOCK_COUNT:
        return res->blockSettings.count;

    case METATILE_MAX_MAP_SIZE:
        return res->metaTileEngineSettings.maxMapSize;

    case METATILE_N_METATILES:
        return res->metaTileEngineSettings.nMetaTiles;
    }

    return QVariant();
}

bool ResourceFilePropertyManager::setData(int id, const QVariant& value)
{
    Q_ASSERT(_project);

    const RES::ResourcesFile* res = _project->resourcesFile();
    Q_ASSERT(res);

    auto editBlock = [&](auto f) {
        RES::BlockSettings bs = res->blockSettings;
        f(bs);
        if (bs != res->blockSettings) {
            _project->undoStack()->push(
                new EditResourceFileSettingsCommand<RES::BlockSettings>(
                    _project, bs,
                    tr("Edit %1").arg(propertyTitle(id))));
            return true;
        }
        return false;
    };

    auto editMetaTile = [&](auto f) {
        MT::EngineSettings es = res->metaTileEngineSettings;
        f(es);
        if (es != res->metaTileEngineSettings) {
            _project->undoStack()->push(
                new EditResourceFileSettingsCommand<MT::EngineSettings>(
                    _project, es,
                    tr("Edit %1").arg(propertyTitle(id))));
            return true;
        }
        return false;
    };

    switch ((PropertyId)id) {
    case BLOCK_SIZE:
        return editBlock([&](auto& bs) { bs.size = value.toUInt(); });

    case BLOCK_COUNT:
        return editBlock([&](auto& bs) { bs.count = value.toUInt(); });

    case METATILE_MAX_MAP_SIZE:
        return editMetaTile([&](auto& es) { es.maxMapSize = value.toUInt(); });

    case METATILE_N_METATILES:
        return editMetaTile([&](auto& es) { es.nMetaTiles = value.toUInt(); });
    }

    return false;
}
