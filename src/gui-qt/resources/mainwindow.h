/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QUndoGroup>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;
class OpenRecentMenu;
class AbstractEditor;
class AbstractProject;
class AbstractResourceItem;

namespace Resources {
namespace Ui {
class MainWindow;
}
class TabBar;
class ResourcesTreeDock;
class ErrorListDock;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void loadProject(const QString& filename);
    void setProject(std::unique_ptr<AbstractProject>&& project);

private slots:
    // updates windowTitle, windowFilePath and action_Save->text
    void updateGuiFilePath();
    void onUndoGroupCleanChanged();
    void onResourceItemCreated(AbstractResourceItem* item);
    void onSelectedResourceChanged();

private:
    bool unsavedChangesDialog();

private slots:
    void onMenuNew();
    void onMenuOpen();
    void onMenuOpenRecent(QString filename);
    void onMenuSave();
    bool onMenuSaveAll();
    void onMenuCloseProject();

    void onMenuAbout();

protected:
    virtual void closeEvent(QCloseEvent* event) override;

    void readSettings();
    void saveSettings();

private:
    std::unique_ptr<AbstractProject> _project;
    AbstractResourceItem* _selectedResource;

    std::unique_ptr<Ui::MainWindow> _ui;

    TabBar* _tabBar;
    QMainWindow* _projectWindow;
    ResourcesTreeDock* _resourcesTreeDock;
    ErrorListDock* _errorListDock;
    QDockWidget* _propertiesDock;

    QStackedWidget* _propertiesStackedWidget;
    QStackedWidget* _centralStackedWidget;

    ZoomSettings* _zoomSettings;
    QUndoGroup* _undoGroup;
    QVector<AbstractEditor*> _editors;
};
}
}
}
