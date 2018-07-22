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
namespace Ui {
class MainWindow;
}
class ZoomSettingsManager;
class ZoomSettingsUi;
class OpenRecentMenu;
class AbstractEditor;
class AbstractProject;
class AbstractProjectLoader;
class AbstractResourceItem;
class TabBar;
class ResourcesTreeDock;
class ErrorListDock;

class MainWindow : public QMainWindow {
    Q_OBJECT

    const static QString ALL_FILE_FILTERS;

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
    void setEditor(AbstractEditor* editor);
    void updateEditViewMenus();

    bool unsavedChangesDialog();

private slots:
    void onEditorZoomSettingsChanged();

    void onMenuNew(QAction* action);
    void onMenuOpen();
    void onMenuOpenRecent(QString filename);
    void onMenuSave();
    bool onMenuSaveAll();
    void onMenuRevertResource();
    void onMenuCloseProject();

    void onMenuAbout();

protected:
    virtual void closeEvent(QCloseEvent* event) override;

    void readSettings();
    void saveSettings();
    QVector<QPair<QString, QMainWindow*>> settingsStateNameWindowList();

private:
    std::unique_ptr<AbstractProject> _project;
    AbstractResourceItem* _selectedResource;
    AbstractEditor* _currentEditor;

    std::unique_ptr<Ui::MainWindow> const _ui;

    TabBar* const _tabBar;
    QMainWindow* const _projectWindow;
    ResourcesTreeDock* const _resourcesTreeDock;
    ErrorListDock* const _errorListDock;
    QDockWidget* const _propertiesDock;

    QStackedWidget* const _propertiesStackedWidget;
    QStackedWidget* const _centralStackedWidget;

    ZoomSettingsManager* const _zoomSettingsManager;
    ZoomSettingsUi* const _zoomSettingsUi;

    QUndoGroup* const _undoGroup;

    QList<AbstractEditor*> const _editors;
    QList<AbstractProjectLoader*> const _projectLoaders;
};
}
}
