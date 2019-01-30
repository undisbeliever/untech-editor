/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "staticresourcelist.h"
#include "project.h"
#include "project-settings/projectsettingsresourceitem.h"

using namespace UnTech::GuiQt;

StaticResourceList::StaticResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::STATIC)
    , _projectSettingsResourceItem(new ProjectSettings::ProjectSettingsResourceItem(this, PROJECT_SETTINGS))
{
}

const QString StaticResourceList::resourceTypeNameSingle() const
{
    return tr("Project");
}

const QString StaticResourceList::resourceTypeNamePlural() const
{
    return tr("Project");
}

const QList<AbstractResourceList::AddResourceSettings>& StaticResourceList::addResourceSettings() const
{
    static const QList<AbstractResourceList::AddResourceSettings> emptyList;
    return emptyList;
}

size_t StaticResourceList::nItems() const
{
    return N_ITEMS;
}

AbstractResourceItem* StaticResourceList::buildResourceItem(size_t index)
{
    assert(index < N_ITEMS);
    switch (static_cast<Indexes>(index)) {
    case PROJECT_SETTINGS:
        return _projectSettingsResourceItem;
    }

    throw std::out_of_range("index is invalid");
}

void StaticResourceList::do_addResource(int, const std::string&)
{
}

void StaticResourceList::do_removeResource(unsigned)
{
}
