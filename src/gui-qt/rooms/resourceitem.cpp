/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "resourcelist.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/entity/entity-rom-entries/resourceitem.h"
#include "gui-qt/project-settings/resourceitem.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/scenes/resourceitem.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project-data.h"
#include "models/rooms/rooms-serializer.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Rooms;

ResourceItem::ResourceItem(ResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _rooms(parent->rooms())
    , _mapGrid(new MapGrid(this))
    , _roomEntrances(new RoomEntranceList(this))
    , _entityGroups(new EntityGroupList(this))
    , _entityEntries(new EntityEntriesList(this))
{
    Q_ASSERT(index < _rooms.size());

    setFilename(QString::fromStdString(roomInputItem().filename));

    if (auto ri = roomInput()) {
        setName(QString::fromStdString(ri->name));
    }

    setDependencies({
        { ResourceTypeIndex::STATIC, project()->staticResources()->projectSettings()->name() },
        { ResourceTypeIndex::STATIC, project()->staticResources()->scenes()->name() },
        { ResourceTypeIndex::STATIC, project()->staticResources()->entities()->name() },
    });

    connect(_mapGrid, &MapGrid::gridChanged,
            this, &AbstractResourceItem::markUnchecked);

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

void ResourceItem::saveResourceData(const std::filesystem::path& filename) const
{
    if (auto ri = this->roomInput()) {
        UnTech::Rooms::saveRoomInput(*ri, filename);
    }
}

bool ResourceItem::loadResourceData(ErrorList& err)
{
    auto& extFileItem = roomInputItem();

    setFilename(QString::fromStdString(extFileItem.filename));

    if (extFileItem.filename.empty()) {
        err.addErrorString("Missing filename");
        return false;
    }

    try {
        extFileItem.loadFile();
        Q_ASSERT(extFileItem.value);

        setName(QString::fromStdString(extFileItem.value->name));
        return true;
    }
    catch (const std::exception& ex) {
        extFileItem.value = nullptr;

        err.addErrorString(ex.what());
        return false;
    }
}

bool ResourceItem::compileResource(ErrorList& err)
{
    return project()->projectData().compileRoom(index(), err);
}

bool ResourceItem::edit_setName(const idstring& name)
{
    return UndoHelper(this).editName(name);
}

bool ResourceItem::edit_setScene(const idstring& scene)
{
    return UndoHelper(this).editField(
        scene,
        tr("Edit Scene"),
        [](DataT& r) -> idstring& { return r.scene; },
        [](ResourceItem& item) { emit item.sceneChanged(); });
}
