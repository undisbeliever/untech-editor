/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "abstractresourceitem.h"
#include "common.h"
#include "resourcevalidationworker.h"
#include "gui-qt/common/aboutdialog.h"
#include "gui-qt/common/graphics/zoomsettingsmanager.h"
#include "gui-qt/common/graphics/zoomsettingsui.h"
#include "gui-qt/mainwindow.ui.h"

#include "errorlistdock.h"
#include "genericpropertieswidget.h"
#include "resourcestreedock.h"
#include "tabbar.h"

#include "gui-qt/project.h"

#include "gui-qt/entity/entity-function-tables/editorwidget.h"
#include "gui-qt/entity/entity-rom-entries/editorwidget.h"
#include "gui-qt/entity/entity-rom-structs/editorwidget.h"
#include "gui-qt/metasprite/actionpoints/editorwidget.h"
#include "gui-qt/metasprite/exportorder/editorwidget.h"
#include "gui-qt/metasprite/metasprite/editorwidget.h"
#include "gui-qt/metasprite/spriteimporter/editorwidget.h"
#include "gui-qt/metatiles/mttileset/editorwidget.h"
#include "gui-qt/project-settings/editorwidget.h"
#include "gui-qt/resources/palette/editorwidget.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>

using namespace UnTech::GuiQt;

const QString MainWindow::OPEN_PROJECT_FILTERS = QString::fromUtf8(
    "UnTech Project File (*.utproject);;All Files (*)");

const QString MainWindow::SAVE_PROJECT_FILTER = QString::fromUtf8(
    "UnTech Project File (*.utproject)");

const QString MainWindow::PROJECT_EXTENSION = QString::fromUtf8("utproject");

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _project(nullptr)
    , _selectedResource(nullptr)
    , _currentEditor(nullptr)
    , _ui(std::make_unique<Ui::MainWindow>())
    , _tabBar(new TabBar(this))
    , _projectWindow(new QMainWindow(this))
    , _resourcesTreeDock(new ResourcesTreeDock(_projectWindow))
    , _errorListDock(new ErrorListDock(_projectWindow))
    , _centralStackedWidget(new QStackedWidget(_projectWindow))
    , _zoomSettingsManager(new ZoomSettingsManager(this))
    , _zoomSettingsUi(new ZoomSettingsUi(this))
    , _undoGroup(new QUndoGroup(this))
    , _editors({
          new ProjectSettings::EditorWidget(this),
          new Entity::EntityRomStructs::EditorWidget(this),
          new Entity::EntityFunctionTables::EditorWidget(this),
          new Entity::EntityRomEntries::EditorWidget(this),
          new Resources::Palette::EditorWidget(this),
          new MetaSprite::ExportOrder::EditorWidget(this),
          new MetaTiles::MtTileset::EditorWidget(_zoomSettingsManager, this),
          new MetaSprite::ActionPoints::EditorWidget(this),
          new MetaSprite::SpriteImporter::EditorWidget(_zoomSettingsManager, this),
          new MetaSprite::MetaSprite::EditorWidget(_zoomSettingsManager, this),
      })
    , _editorDockWidgets()
{
    _ui->setupUi(this);

    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    QVBoxLayout* centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setMargin(0);
    centralLayout->setSpacing(0);
    centralWidget->setLayout(centralLayout);

    centralLayout->addWidget(_tabBar);
    centralLayout->addWidget(_projectWindow);

    _projectWindow->addDockWidget(Qt::LeftDockWidgetArea, _resourcesTreeDock);
    _projectWindow->addDockWidget(Qt::LeftDockWidgetArea, _errorListDock);

    _projectWindow->setCentralWidget(_centralStackedWidget);

    // Have the left and right docks take up the whole height of the documentWindow
    _projectWindow->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    _projectWindow->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    _projectWindow->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    _projectWindow->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // Set Initial Zoom values
    {
        _zoomSettingsManager->set("metatiles", 3, ZoomSettings::NTSC);
        _zoomSettingsManager->set("spriteimporter", 3, ZoomSettings::NTSC);
        _zoomSettingsManager->set("metasprite", 6, ZoomSettings::NTSC);
        _zoomSettingsManager->set("metasprite-preview", 6, ZoomSettings::NTSC);
    }

    // Shrink errorListDock
    _projectWindow->resizeDocks({ _errorListDock }, { 10 }, Qt::Vertical);

    // Update Menu
    {
        _ui->action_AddResource->setMenu(_resourcesTreeDock->addResourceMenu());
    }

    // Status Bar
    {
        _zoomSettingsUi->setZoomSettings(nullptr);
        statusBar()->addPermanentWidget(_zoomSettingsUi->aspectRatioComboBox());
        statusBar()->addPermanentWidget(_zoomSettingsUi->zoomComboBox());
    }

    // Default (blank) widgets are always index 0
    Q_ASSERT(_centralStackedWidget->count() == 0);
    _centralStackedWidget->addWidget(new QWidget(_centralStackedWidget));

    // Setup Dock Widgets
    _projectWindow->setDockOptions(AnimatedDocks | AllowTabbedDocks | AllowNestedDocks);

    for (AbstractEditorWidget* editor : _editors) {
        // Prevents centralStackedWidget from expanding when widget is changed
        editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        _centralStackedWidget->addWidget(editor);

        QList<QDockWidget*> dockWidgets = editor->createDockWidgets(_projectWindow);
        for (auto* dw : dockWidgets) {
            if (_projectWindow->dockWidgetArea(dw) == Qt::NoDockWidgetArea) {
                _projectWindow->addDockWidget(Qt::RightDockWidgetArea, dw);
            }

            dw->setFeatures(QDockWidget::DockWidgetMovable);
            dw->hide();
        }

        _editorDockWidgets.append(dockWidgets);
    };

    // Do not show context menu on any of the docked widgets
    for (auto* c : _projectWindow->children()) {
        if (auto* dw = qobject_cast<QDockWidget*>(c)) {
            dw->setContextMenuPolicy(Qt::PreventContextMenu);

            // By default docks will be raised in reverse order.
            _dockWidgetsRaisedState[dw->objectName()] = true;
        }
    }

    // Restore state
    readSettings();
    hideAllEditorDockWidgets();

    setProject(nullptr);

    updateEditViewMenus();

    connect(_undoGroup, &QUndoGroup::cleanChanged,
            this, &MainWindow::onUndoGroupCleanChanged);

    connect(_tabBar, &TabBar::closeProjectRequested,
            this, &MainWindow::onMenuCloseProject);

    connect(_ui->action_New, &QAction::triggered,
            this, &MainWindow::onMenuNew);
    connect(_ui->action_Open, &QAction::triggered,
            this, &MainWindow::onMenuOpen);
    connect(_ui->menu_OpenRecent, &OpenRecentMenu::recentFileSelected,
            this, &MainWindow::onMenuOpenRecent);
    connect(_ui->action_Save, &QAction::triggered,
            this, &MainWindow::onMenuSave);
    connect(_ui->action_SaveAll, &QAction::triggered,
            this, &MainWindow::onMenuSaveAll);
    connect(_ui->action_RevertResource, &QAction::triggered,
            this, &MainWindow::onMenuRevertResource);
    connect(_ui->action_CloseProject, &QAction::triggered,
            this, &MainWindow::onMenuCloseProject);
    connect(_ui->action_Quit, &QAction::triggered,
            this, &MainWindow::close);

    connect(_ui->action_About, &QAction::triggered,
            this, &MainWindow::onMenuAbout);
}

MainWindow::~MainWindow() = default;

void MainWindow::loadProject(const QString& filename)
{
    QString ext = QFileInfo(filename).suffix();

    std::unique_ptr<Project> project = Project::loadProject(filename);
    if (project) {
        _ui->menu_OpenRecent->addFilename(project->filename());
        setProject(std::move(project));
    }
}

void MainWindow::setProject(std::unique_ptr<Project>&& project)
{
    auto oldProject = std::move(_project);

    if (oldProject) {
        oldProject->setSelectedResource(nullptr);
        oldProject->disconnect(this);
    }
    _project = std::move(project);

    _tabBar->setProject(_project.get());
    _resourcesTreeDock->setProject(_project.get());
    _errorListDock->setProject(_project.get());

    // Close the errors dock as it is now empty
    _errorListDock->close();

    if (_project != nullptr) {
        Q_ASSERT(!_project->filename().isEmpty());

        _undoGroup->addStack(_project->undoStack());
        _project->undoStack()->setActive();

        _project->validationWorker()->validateAllResources();

        for (AbstractResourceList* rl : _project->resourceLists()) {
            for (AbstractResourceItem* item : rl->items()) {
                _undoGroup->addStack(item->undoStack());
            }
        }

        connect(_project.get(), &Project::filenameChanged,
                this, &MainWindow::updateGuiFilePath);

        connect(_project.get(), &Project::selectedResourceChanged,
                this, &MainWindow::onSelectedResourceChanged);
    }

    onSelectedResourceChanged();

    updateGuiFilePath();
}

void MainWindow::updateGuiFilePath()
{
    QString relativePath;
    QString filePath;

    auto* exItem = qobject_cast<AbstractExternalResourceItem*>(_selectedResource);
    if (exItem) {
        filePath = exItem->absoluteFilePath();
        relativePath = exItem->relativeFilePath();
    }
    else if (_project) {
        filePath = _project->filename();
        relativePath = QFileInfo(_project->filename()).fileName();
    }

    setWindowFilePath(filePath);
    if (!relativePath.isEmpty()) {
        setWindowTitle(QStringLiteral("[*]") + relativePath);
        _ui->action_Save->setText(tr("Save \"%1\"").arg(relativePath));
    }
    else {
        setWindowTitle(QString());
        _ui->action_Save->setText(tr("Save"));
    }

    _ui->action_RevertResource->setEnabled(exItem != nullptr);
    if (exItem) {
        _ui->action_RevertResource->setText(tr("Revert \"%1\"").arg(relativePath));
    }
    else {
        _ui->action_RevertResource->setText(tr("Revert"));
    }
}

void MainWindow::onUndoGroupCleanChanged()
{
    _ui->action_Save->setEnabled(!_undoGroup->isClean());
    setWindowModified(!_undoGroup->isClean());
}

void MainWindow::onResourceItemCreated(AbstractResourceItem* item)
{
    _undoGroup->addStack(item->undoStack());
}

void MainWindow::onSelectedResourceChanged()
{
    if (_selectedResource) {
        _selectedResource->disconnect(this);
    }
    _selectedResource = nullptr;

    if (_project) {
        _selectedResource = _project->selectedResource();
    }

    if (_selectedResource) {
        _selectedResource->undoStack()->setActive();

        if (_selectedResource->state() == ResourceState::NOT_LOADED) {
            _selectedResource->loadResource();
        }

        if (auto* exItem = qobject_cast<AbstractExternalResourceItem*>(_selectedResource)) {
            connect(exItem, &AbstractExternalResourceItem::relativeFilePathChanged,
                    this, &MainWindow::updateGuiFilePath);
        }
    }
    else {
        if (_project) {
            _project->undoStack()->setActive();
        }
    }

    updateGuiFilePath();

    int editorIndex = -1;
    for (int i = 0; i < _editors.size(); i++) {
        bool s = _editors.at(i)->setResourceItem(_selectedResource);
        if (s) {
            editorIndex = i;
        }
    }
    setEditorIndex(editorIndex);
}

void MainWindow::setEditorIndex(int index)
{
    AbstractEditorWidget* editor = _editors.value(index, nullptr);

    if (_currentEditor == editor) {
        return;
    }

    saveDockWidgetsRaisedState();

    if (_currentEditor) {
        _currentEditor->disconnect(this);
        _errorListDock->disconnect(_currentEditor);

        if (QWidget* sw = _currentEditor->statusBarWidget()) {
            statusBar()->removeWidget(sw);
        }
    }
    _currentEditor = editor;

    // hide all docks except for the ones used by the editor
    Q_ASSERT(_editorDockWidgets.size() == _editors.size());
    for (int i = 0; i < _editorDockWidgets.size(); i++) {
        for (QDockWidget* dw : _editorDockWidgets.at(i)) {
            dw->setHidden(i != index);
        }
    }
    restoreDockWidgetsRaisedState();

    if (editor) {
        _zoomSettingsUi->setZoomSettings(editor->zoomSettings());
        _centralStackedWidget->setCurrentIndex(index + 1);

        if (QWidget* sw = editor->statusBarWidget()) {
            statusBar()->insertPermanentWidget(0, sw);
            sw->show();
        }

        connect(_currentEditor, &AbstractEditorWidget::zoomSettingsChanged,
                this, &MainWindow::onEditorZoomSettingsChanged);

        connect(_errorListDock, &ErrorListDock::errorDoubleClicked,
                editor, &AbstractEditorWidget::onErrorDoubleClicked);
    }
    else {
        _zoomSettingsUi->setZoomSettings(nullptr);
        _centralStackedWidget->setCurrentIndex(0);
    }

    updateEditViewMenus();
}

void MainWindow::hideAllEditorDockWidgets()
{
    for (auto& dockWidgets : _editorDockWidgets) {
        for (QDockWidget* dw : dockWidgets) {
            dw->hide();
        }
    }
}

void MainWindow::saveDockWidgetsRaisedState()
{
    const int editorIndex = _editors.indexOf(_currentEditor);
    if (editorIndex < 0) {
        return;
    }

    auto saveState = [this](QDockWidget* dw) {
        // KUDOS: Pavel Strakhov
        // https://stackoverflow.com/a/22238571
        bool isRaised = not dw->visibleRegion().isEmpty();
        _dockWidgetsRaisedState[dw->objectName()] = isRaised;
    };

    saveState(_resourcesTreeDock);
    saveState(_errorListDock);

    for (auto* dw : _editorDockWidgets.at(editorIndex)) {
        saveState(dw);
    }
}

void MainWindow::restoreDockWidgetsRaisedState()
{
    const int editorIndex = _editors.indexOf(_currentEditor);
    if (editorIndex < 0) {
        return;
    }

    auto restoreState = [this](QDockWidget* dw) {
        if (_dockWidgetsRaisedState[dw->objectName()]) {
            dw->raise();
        }
    };

    restoreState(_resourcesTreeDock);
    restoreState(_errorListDock);

    for (auto* dw : _editorDockWidgets.at(editorIndex)) {
        restoreState(dw);
    }
}

void MainWindow::onEditorZoomSettingsChanged()
{
    Q_ASSERT(_currentEditor);

    _zoomSettingsUi->setZoomSettings(_currentEditor->zoomSettings());
}

void MainWindow::updateEditViewMenus()
{
    _ui->menu_Edit->clear();
    _ui->menu_View->clear();

    QAction* undoAction = _undoGroup->createUndoAction(this);
    QAction* redoAction = _undoGroup->createRedoAction(this);
    undoAction->setIcon(QIcon(":/icons/undo.svg"));
    undoAction->setShortcuts(QKeySequence::Undo);
    redoAction->setIcon(QIcon(":/icons/redo.svg"));
    redoAction->setShortcuts(QKeySequence::Redo);
    _ui->menu_Edit->addAction(undoAction);
    _ui->menu_Edit->addAction(redoAction);

    _zoomSettingsUi->populateMenu(_ui->menu_View);

    if (_currentEditor) {
        _currentEditor->populateMenu(_ui->menu_Edit, _ui->menu_View);
    }
}

bool MainWindow::unsavedChangesDialog()
{
    if (_project == nullptr) {
        return true;
    }
    bool success = true;

    QStringList unsavedFilenames = _project->unsavedFilenames();

    if (!unsavedFilenames.isEmpty()) {
        QString dialogText;
        if (unsavedFilenames.size() == 1) {
            dialogText = tr("There is one unsaved file.\nDo you wish to save?");
        }
        else {
            dialogText = tr("There are %1 unsaved files.\nDo you wish to save them all?")
                             .arg(unsavedFilenames.size());
        }

        QMessageBox dialog(QMessageBox::Warning,
                           tr("Save Changes?"), dialogText,
                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                           this);
        dialog.setDetailedText(unsavedFilenames.join("\n"));
        dialog.button(QMessageBox::Save)->setText(tr("Save All"));

        dialog.exec();

        success = (dialog.result() == QMessageBox::Discard);
        if (dialog.result() == QMessageBox::Save) {
            success = onMenuSaveAll();
        }
    }

    return success;
}

void MainWindow::onMenuNew()
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    QFileDialog saveDialog(this, "Create New Resource File");
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setNameFilter(SAVE_PROJECT_FILTER);
    saveDialog.setDefaultSuffix(PROJECT_EXTENSION);
    saveDialog.setOption(QFileDialog::DontUseNativeDialog);

    saveDialog.exec();

    if (saveDialog.result() == QDialog::Accepted) {
        Q_ASSERT(saveDialog.selectedFiles().size() == 1);
        QString filename = saveDialog.selectedFiles().first();

        std::unique_ptr<Project> project = Project::newProject(filename);
        if (project) {
            _ui->menu_OpenRecent->addFilename(project->filename());
            setProject(std::move(project));
        }
    }
}

void MainWindow::onMenuOpen()
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    const QString filename = QFileDialog::getOpenFileName(
        this, tr("Open Project"), QString(), OPEN_PROJECT_FILTERS,
        nullptr, QFileDialog::DontUseNativeDialog);

    if (!filename.isNull()) {
        loadProject(filename);
    }
}

void MainWindow::onMenuOpenRecent(QString filename)
{
    Q_ASSERT(filename.isEmpty() == false);

    if (unsavedChangesDialog() == false) {
        return;
    }

    loadProject(filename);
}

void MainWindow::onMenuSave()
{
    Q_ASSERT(_project != nullptr);
    try {
        if (auto* exItem = qobject_cast<AbstractExternalResourceItem*>(_selectedResource)) {
            // current resource is external

            Q_ASSERT(!exItem->filename().isEmpty());
            exItem->saveResource();
        }
        else {
            // current resource is internal

            Q_ASSERT(!_project->filename().isEmpty());
            _project->saveProject(_project->filename());
        }
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(this, tr("Error Saving File"), ex.what());
    }
}

bool MainWindow::onMenuSaveAll()
{
    Q_ASSERT(_project != nullptr);
    Q_ASSERT(!_project->filename().isEmpty());

    bool s = _project->saveProject(_project->filename());
    if (s) {
        _ui->menu_OpenRecent->addFilename(_project->filename());
    }

    for (AbstractExternalResourceItem* item : _project->unsavedExternalResources()) {
        try {
            item->saveResource();
            s &= true;
        }
        catch (const std::exception& ex) {
            QMessageBox::critical(this, tr("Error Saving File"), ex.what());
            s = false;
        }
    }

    return s;
}

void MainWindow::onMenuRevertResource()
{
    Q_ASSERT(_project != nullptr);
    Q_ASSERT(!_project->filename().isEmpty());

    auto* item = qobject_cast<AbstractExternalResourceItem*>(_project->selectedResource());
    if (item == nullptr) {
        return;
    }

    bool revertItem = item->undoStack()->count() == 0;

    if (revertItem == false) {
        QString dialogText = tr("Are you are sure you wish to revert \"%1\"?\n\n"
                                "You will not be able to undo this action.")
                                 .arg(item->relativeFilePath());

        QMessageBox dialog(QMessageBox::Warning,
                           tr("Revert File?"), dialogText,
                           QMessageBox::Yes | QMessageBox::Cancel,
                           this);
        dialog.setDefaultButton(QMessageBox::Cancel);
        dialog.exec();

        revertItem = dialog.result() == QMessageBox::Yes;
    }

    if (revertItem) {
        auto* list = item->resourceList();
        list->revertResource(item);
    }
}

void MainWindow::onMenuCloseProject()
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    setProject(nullptr);
}

void MainWindow::onMenuAbout()
{
    AboutDialog().exec();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (unsavedChangesDialog()) {
        event->accept();
        saveSettings();
    }
    else {
        event->ignore();
    }
}

constexpr static int STATE_VERSION = 0x42;

void MainWindow::readSettings()
{
    assertDockWidgetObjectNamesValid();

    QSettings settings;

    _zoomSettingsManager->readSettings(settings);

    restoreGeometry(settings.value("geometry").toByteArray());

    const auto& snList = settingsStateNameWindowList();
    for (const auto& it : snList) {
        const QString& stateName = it.first;

        if (settings.contains(stateName) == false) {
            // required to prevent docks from resizing on layout change
            saveSettings();
            break;
        }
    }

    for (const auto& it : snList) {
        const QString& stateName = it.first;
        QMainWindow* mw = it.second;

        mw->restoreState(settings.value(stateName).toByteArray(), STATE_VERSION);
    }

    QStringList raisedDocks = settings.value("raised_docks").toStringList();
    if (not raisedDocks.empty()) {
        for (auto it = _dockWidgetsRaisedState.begin(); it != _dockWidgetsRaisedState.end(); it++) {
            it.value() = raisedDocks.contains(it.key());
        }
    }

    restoreDockWidgetsRaisedState();
}

void MainWindow::saveSettings()
{
    saveDockWidgetsRaisedState();

    QSettings settings;

    _zoomSettingsManager->saveSettings(settings);

    settings.setValue("geometry", saveGeometry());

    const auto& snList = settingsStateNameWindowList();
    for (const auto& it : snList) {
        const QString& stateName = it.first;
        QMainWindow* mw = it.second;

        settings.setValue(stateName, mw->saveState(STATE_VERSION));
    }

    QStringList raisedDocks;
    for (auto it = _dockWidgetsRaisedState.begin(); it != _dockWidgetsRaisedState.end(); it++) {
        if (it.value()) {
            raisedDocks.append(it.key());
        }
    }
    settings.setValue("raised_docks", raisedDocks);
}

QVector<QPair<QString, QMainWindow*>> MainWindow::settingsStateNameWindowList()
{
    return {
        qMakePair(QStringLiteral("MainWindow_state"), this),
        qMakePair(QStringLiteral("ProjectWindow_state"), _projectWindow),
    };
}

void MainWindow::assertDockWidgetObjectNamesValid()
{
    QSet<QString> dockWidgetNames;

    for (const auto& snList : settingsStateNameWindowList()) {
        QMainWindow* mainWindow = snList.second;

        for (auto* c : mainWindow->children()) {
            if (auto* dw = qobject_cast<QDockWidget*>(c)) {
                const QString dockName = dw->objectName();

                if (dockName.isEmpty()) {
                    qFatal("QDockWidget objectName is empty");
                }
                if (dockWidgetNames.contains(dockName)) {
                    qFatal("QDockWidget objectNames are not unique");
                }
                dockWidgetNames.insert(dockName);
            }
        }
    }
}
