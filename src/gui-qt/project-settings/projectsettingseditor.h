/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "projectsettingscentralwidget.h"
#include "projectsettingspropertieswidget.h"
#include "projectsettingsresourceitem.h"
#include "gui-qt/abstracteditor.h"
#include "gui-qt/genericpropertieswidget.h"

namespace UnTech {
namespace GuiQt {
namespace ProjectSettings {

class ProjectSettingsEditor : public AbstractEditor {
    Q_OBJECT

public:
    ProjectSettingsEditor(QWidget* parent)
        : AbstractEditor(parent)
        , _editorWidget(new ProjectSettingsCentralWidget(parent))
        , _propertyWidget(new ProjectSettingsPropertiesWidget(parent))
    {
    }
    ~ProjectSettingsEditor() = default;

    virtual bool setResourceItem(Project*, AbstractResourceItem* aItem)
    {
        auto* item = qobject_cast<ProjectSettingsResourceItem*>(aItem);

        _editorWidget->setResourceItem(item);
        _propertyWidget->setResourceItem(item);

        // show this editor when no item is selected
        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return _propertyWidget; }

private:
    ProjectSettingsCentralWidget* const _editorWidget;
    ProjectSettingsPropertiesWidget* const _propertyWidget;
};
}
}
}
