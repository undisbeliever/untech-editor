/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractresourcelist.h"

namespace UnTech {
namespace GuiQt {

namespace ProjectSettings {
class ProjectSettingsResourceItem;
}

class Project;
class AbstractResourceItem;

class StaticResourceList : public AbstractResourceList {
    Q_OBJECT

    enum Indexes {
        PROJECT_SETTINGS,
    };
    constexpr static size_t N_ITEMS = 1;

public:
    StaticResourceList(Project* project);
    ~StaticResourceList() = default;

    virtual const QString resourceTypeNameSingle() const final;
    virtual const QString resourceTypeNamePlural() const final;

    virtual const QList<AddResourceSettings>& addResourceSettings() const final;

    auto* projectSettingsResourceItem() const { return _projectSettingsResourceItem; }

protected:
    virtual size_t nItems() const final;
    virtual AbstractResourceItem* buildResourceItem(size_t index) final;

    virtual void do_addResource(int settingIndex, const std::string& filename) final;
    virtual void do_removeResource(unsigned index) final;

private:
    ProjectSettings::ProjectSettingsResourceItem* const _projectSettingsResourceItem;
};

}
}
