/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "projectsettingspropertieswidget.h"
#include "projectsettingspropertymanager.h"
#include "projectsettingsresourceitem.h"
#include "gui-qt/project-settings/projectsettingspropertieswidget.ui.h"

using namespace UnTech::GuiQt::ProjectSettings;

ProjectSettingsPropertiesWidget::ProjectSettingsPropertiesWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::ProjectSettingsPropertiesWidget>())
    , _manager(new ProjectSettingsPropertyManager(this))
{
    _ui->setupUi(this);

    _ui->propertyView->setPropertyManager(_manager);
}

ProjectSettingsPropertiesWidget::~ProjectSettingsPropertiesWidget() = default;

void ProjectSettingsPropertiesWidget::setResourceItem(ProjectSettingsResourceItem* item)
{
    _manager->setResourceItem(item);
}
