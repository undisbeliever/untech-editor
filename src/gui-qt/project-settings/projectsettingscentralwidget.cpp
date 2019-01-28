/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "projectsettingscentralwidget.h"
#include "gui-qt/project-settings/projectsettingscentralwidget.ui.h"
#include "gui-qt/project.h"

using namespace UnTech::GuiQt::ProjectSettings;

// ::TODO add some stats and total size figures in this widget::

ProjectSettingsCentralWidget::ProjectSettingsCentralWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::ProjectSettingsCentralWidget>())
    , _project(nullptr)
{
    _ui->setupUi(this);
}

ProjectSettingsCentralWidget::~ProjectSettingsCentralWidget() = default;

void ProjectSettingsCentralWidget::setProject(Project* project)
{
    if (_project) {
        _project->disconnect(this);
    }
    _project = project;
}
