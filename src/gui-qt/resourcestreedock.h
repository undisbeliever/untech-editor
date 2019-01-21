/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QDockWidget>
#include <QMenu>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Ui {
class ResourcesTreeDock;
}
class Project;
class AbstractResourceList;
class ResourcesTreeModel;

class ResourcesTreeDock : public QDockWidget {
    Q_OBJECT

public:
    ResourcesTreeDock(QWidget* parent = nullptr);
    ~ResourcesTreeDock();

    void setProject(Project* project);

    QMenu* addResourceMenu() const { return _addResourceMenu; }

private slots:
    void onAddResourceMenuTriggered(QAction* action);
    void onRemoveResourceTriggered();

    void onSelectedResourceChanged();
    void onResourcesTreeSelectionChanged();

private:
    void setupAddResourceMenu();

private:
    std::unique_ptr<Ui::ResourcesTreeDock> const _ui;
    ResourcesTreeModel* const _model;
    QMenu* const _addResourceMenu;

    Project* _project;
};
}
}
