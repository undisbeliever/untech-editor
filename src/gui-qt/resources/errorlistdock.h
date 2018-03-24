/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QDockWidget>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace Ui {
class ErrorListDock;
}
class AbstractProject;
class AbstractResourceItem;

class ErrorListDock : public QDockWidget {
    Q_OBJECT

public:
    ErrorListDock(QWidget* parent = nullptr);
    ~ErrorListDock();

    void setProject(AbstractProject* project);

private slots:
    void onSelectedResourceChanged();

    void updateErrorList();

private:
    std::unique_ptr<Ui::ErrorListDock> _ui;
    AbstractProject* _project;
    AbstractResourceItem* _currentItem;
};
}
}
}
