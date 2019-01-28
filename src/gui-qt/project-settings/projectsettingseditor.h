/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "projectsettingscentralwidget.h"
#include "projectsettingspropertieswidget.h"
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

    virtual bool setResourceItem(Project* project, AbstractResourceItem* item)
    {
        _editorWidget->setProject(project);
        _propertyWidget->setProject(project);

        // show this editor when no item is selected
        return project != nullptr && item == nullptr;
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
