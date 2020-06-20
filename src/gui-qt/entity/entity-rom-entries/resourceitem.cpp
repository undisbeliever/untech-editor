/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "gui-qt/entity/entity-function-tables/resourceitem.h"
#include "gui-qt/entity/entity-rom-structs/resourceitem.h"
#include "gui-qt/metasprite/metasprite/drawframepixmap.hpp"
#include "gui-qt/project-settings/resourceitem.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project-data.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Entity::EntityRomEntries;

ResourceItem::ResourceItem(StaticResourceList* list, unsigned index,
                           EN::EntityType entityType)
    : AbstractInternalResourceItem(list, index)
    , _entriesList(new EntityRomEntriesList(this, entityType))
{
    switch (entityType) {
    case EN::EntityType::ENTITY:
        setName(tr("Entities"));
        break;

    case EN::EntityType::PROJECTILE:
        setName(tr("Projectiles"));
        break;

    case EN::EntityType::PLAYER:
        setName(tr("Players"));
        break;
    }

    setRemovable(false);

    if (entityType == EN::EntityType::ENTITY) {
        // Adding "Projectiles" and "Players" prevents errors from players/projectiles appearing in the entities resource item.
        setDependencies({
            { ResourceTypeIndex::STATIC, resourceList()->projectSettings()->name() },
            { ResourceTypeIndex::STATIC, resourceList()->entityFunctionTables()->name() },
            { ResourceTypeIndex::STATIC, tr("Projectiles") },
            { ResourceTypeIndex::STATIC, tr("Players") },
            { ResourceTypeIndex::MS_FRAMESET, QString() },
        });
    }
    else {
        setDependencies({
            { ResourceTypeIndex::STATIC, resourceList()->projectSettings()->name() },
            { ResourceTypeIndex::STATIC, resourceList()->entityFunctionTables()->name() },
            { ResourceTypeIndex::MS_FRAMESET, QString() },
        });
    }

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);

    connect(this, &AbstractResourceItem::resourceComplied,
            this, &ResourceItem::updateEntityPixmaps);

    // When an item is changed (ie, by the undoStack) change selected entry
    connect(_entriesList, &EntityRomEntriesList::dataChanged,
            _entriesList, &EntityRomEntriesList::setSelectedIndex);
    connect(_entriesList, &EntityRomEntriesList::itemAdded,
            _entriesList, &EntityRomEntriesList::setSelectedIndex);
}

bool ResourceItem::compileResource(UnTech::ErrorList& err)
{
    if (_entriesList->entityType() == EN::EntityType::ENTITY) {
        // EntityRomData is required by the Room Subsystem.
        return project()->projectData().compileEntityRomData(err);
    }
    else {
        // Players and projectiles - only check entries, don't compile entity rom data.
        // EntityRomData will be compiled by the "Entities" ResourceItem after the player and projectiles are validated.

        using namespace UnTech::Entity;

        auto* entries = _entriesList->list();
        Q_ASSERT(entries);

        const auto* projectFile = project()->projectFile();
        Q_ASSERT(projectFile);

        const auto& structFieldMap = project()->staticResources()->entityRomStructs()->structFieldMap();
        const auto ftFieldMap = generateFunctionTableFieldMap(projectFile->entityRomData.functionTables, structFieldMap, *projectFile, err);

        bool valid = true;

        for (const auto& e : *entries) {
            valid &= e.validate(_entriesList->entityType(), *projectFile, ftFieldMap, err);
        }

        return valid;
    }
}

void ResourceItem::updateEntityPixmaps()
{
    constexpr QSize MIN_PIXMAP_SIZE{ EntityPixmap::MIN_PIXMAP_SIZE, EntityPixmap::MIN_PIXMAP_SIZE };

    const auto* entities = _entriesList->list();
    Q_ASSERT(entities);
    Q_ASSERT(project()->projectFile());

    const auto& frameSets = project()->projectFile()->frameSets;

    QVector<EntityPixmap> pixmaps;
    pixmaps.reserve(entities->size());

    std::unordered_map<idstring, unsigned> nameMap;
    nameMap.reserve(entities->size());

    size_t index = 0;
    for (const EN::EntityRomEntry& en : *entities) {
        std::tuple<QPixmap, QPoint, QRectF> p;

        auto it = std::find_if(frameSets.begin(), frameSets.end(),
                               [&](auto& fs) { return fs.msFrameSet && fs.msFrameSet->name == en.frameSetId; });
        if (it != frameSets.end()) {
            const auto& msFrameSet = *it->msFrameSet;
            if (auto frame = msFrameSet.frames.find(en.displayFrame)) {
                p = GuiQt::MetaSprite::MetaSprite::drawFramePixmap(msFrameSet, *frame, en.defaultPalette, MIN_PIXMAP_SIZE);
                nameMap.emplace(en.name, index);
            }
        }

        pixmaps.append({ .pixmap = std::move(std::get<0>(p)),
                         .name = QString::fromStdString(en.name),
                         .origin = std::get<1>(p),
                         .boundingBox = std::get<2>(p) });

        index++;
    }

    _entityPixmaps = std::move(pixmaps);
    _entityNameMap = std::move(nameMap);
    emit entityPixmapsChanged();
}
