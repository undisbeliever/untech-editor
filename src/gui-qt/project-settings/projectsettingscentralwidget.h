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
class ProjectSettingsCentralWidget;
}
class ProjectSettingsResourceItem;

class ProjectSettingsCentralWidget : public QWidget {
    Q_OBJECT

public:
    ProjectSettingsCentralWidget(QWidget* parent = 0);
    ~ProjectSettingsCentralWidget();

    void setResourceItem(ProjectSettingsResourceItem* item);

private:
    std::unique_ptr<Ui::ProjectSettingsCentralWidget> const _ui;

    ProjectSettingsResourceItem* _item;
};
}
}
}
