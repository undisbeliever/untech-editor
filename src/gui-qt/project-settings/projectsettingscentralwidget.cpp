/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "projectsettingscentralwidget.h"
#include "projectsettingsresourceitem.h"
#include "gui-qt/project-settings/projectsettingscentralwidget.ui.h"

using namespace UnTech::GuiQt::ProjectSettings;

// ::TODO add some stats and total size figures in this widget::

ProjectSettingsCentralWidget::ProjectSettingsCentralWidget(QWidget* parent)
    : QWidget(parent)
    , _ui(std::make_unique<Ui::ProjectSettingsCentralWidget>())
    , _item(nullptr)
{
    _ui->setupUi(this);
}

void ProjectSettingsCentralWidget::setResourceItem(ProjectSettingsResourceItem* item)
{
    if (_item) {
        _item->disconnect(this);
    }
    _item = item;
}

ProjectSettingsCentralWidget::~ProjectSettingsCentralWidget() = default;
