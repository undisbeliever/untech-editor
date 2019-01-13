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
class Project;

namespace Resources {
namespace Ui {
class ResourceFileCentralWidget;
}
class ResourceFilePropertyManager;

class ResourceFileCentralWidget : public QWidget {
    Q_OBJECT

public:
    ResourceFileCentralWidget(QWidget* parent = 0);
    ~ResourceFileCentralWidget();

    void setProject(Project* project);

private:
    std::unique_ptr<Ui::ResourceFileCentralWidget> const _ui;

    Project* _project;
};
}
}
}
