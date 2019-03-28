/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "managers.h"
#include "projectsettingsresourceitem.h"
#include "gui-qt/project-settings/editorwidget.ui.h"

using namespace UnTech::GuiQt::ProjectSettings;

EditorWidget::EditorWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::EditorWidget>())
    , _manager(new ProjectSettingsPropertyManager(this))
{
    _ui->setupUi(this);

    _ui->propertyView->setPropertyManager(_manager);
}

EditorWidget::~EditorWidget() = default;

void EditorWidget::setResourceItem(ProjectSettingsResourceItem* item)
{
    _manager->setResourceItem(item);
}
