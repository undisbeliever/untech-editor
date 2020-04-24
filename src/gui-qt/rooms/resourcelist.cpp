/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcelist.h"
#include "resourceitem.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/project.h"
#include "models/project/project.h"
#include "models/rooms/rooms-serializer.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Rooms;

ResourceList::ResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::ROOM)
{
}

const QString ResourceList::resourceTypeNameSingle() const
{
    return tr("Room");
}

const QString ResourceList::resourceTypeNamePlural() const
{
    return tr("Rooms");
}

UnTech::ExternalFileList<UnTech::Rooms::RoomInput>& ResourceList::rooms() const
{
    return project()->projectFile()->rooms;
}

size_t ResourceList::nItems() const
{
    return rooms().size();
}

ResourceItem* ResourceList::buildResourceItem(size_t index)
{
    return new ResourceItem(this, index);
}

const QVector<AbstractResourceList::AddResourceSettings>& ResourceList::addResourceSettings() const
{
    const static QVector<AbstractResourceList::AddResourceSettings> filters = {
        { .title = tr("Add Room"),
          .filter = QString::fromUtf8("UnTech Room File (*.utroom)"),
          .extension = QString::fromUtf8("utroom"),
          .canCreateFile = true }
    };

    return filters;
}

void ResourceList::do_addResource(int settingIndex, const std::string& filename)
{
    Q_ASSERT(settingIndex == 0);

    auto& rooms = this->rooms();
    rooms.insert_back(filename);

    QFileInfo fi(QString::fromStdString(filename));
    if (!fi.exists()) {
        QString name = fi.baseName();
        IdstringValidator().fixup(name);

        UnTech::Rooms::RoomInput roomInput;
        roomInput.name = name.toStdString();
        roomInput.map = grid<uint8_t>(32, 14, 0);
        UnTech::Rooms::saveRoomInput(roomInput, filename);
    }
}

void ResourceList::do_removeResource(unsigned index)
{
    auto& rooms = this->rooms();

    Q_ASSERT(index < rooms.size());
    rooms.remove(index);
}
