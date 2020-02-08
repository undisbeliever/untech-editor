/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "project.h"
#include "gui-qt/metatiles/mttileset/resourcelist.h"
#include "gui-qt/resources/palette/resourcelist.h"
#include "models/project/project-data.h"

namespace UnTech {
namespace GuiQt {

class ProjectDataSlots {

public:
    static void connect(Project* const project)
    {
        using ProjectData = UnTech::Project::ProjectData;

        connectList(
            project, project->palettes(),
            [](ProjectData & pd) -> auto& { return pd._palettes; });

        connectList(
            project, project->mtTilesets(),
            [](ProjectData & pd) -> auto& { return pd._metaTileTilesets; });
    }

private:
    template <typename UnaryFunctionT>
    static void connectList(Project* const project,
                            AbstractResourceList* const resourceList,
                            UnaryFunctionT getDataStore)
    {
        // Wipe the dataStore if the list changes (item added/moved/removed).
        QObject::connect(resourceList, &AbstractResourceList::listChanged,
                         resourceList, [=]() {
                             auto& dataStore = getDataStore(project->projectData());

                             dataStore.clearAllAndResize(resourceList->items().size());

                             for (auto* item : resourceList->items()) {
                                 item->markUnchecked();
                             }
                         });

        // Remove old name from dataStore if resourceItem name changes.
        QObject::connect(resourceList, &AbstractResourceList::resourceItemNameAboutToChange,
                         project, [=](AbstractResourceItem* resourceItem) {
                             auto& dataStore = getDataStore(project->projectData());

                             const idstring name = resourceItem->name().toStdString();
                             dataStore.removeName(name);
                             dataStore.clear(resourceItem->index());
                         });
    }
};

}
}
