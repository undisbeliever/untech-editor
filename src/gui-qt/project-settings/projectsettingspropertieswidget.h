/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QWidget>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace ProjectSettings {
namespace Ui {
class ProjectSettingsPropertiesWidget;
}
class ProjectSettingsResourceItem;
class ProjectSettingsPropertyManager;

class ProjectSettingsPropertiesWidget : public QWidget {
    Q_OBJECT

public:
    ProjectSettingsPropertiesWidget(QWidget* parent = 0);
    ~ProjectSettingsPropertiesWidget();

    void setResourceItem(ProjectSettingsResourceItem* item);

private:
    std::unique_ptr<Ui::ProjectSettingsPropertiesWidget> const _ui;
    ProjectSettingsPropertyManager* const _manager;
};
}
}
}
