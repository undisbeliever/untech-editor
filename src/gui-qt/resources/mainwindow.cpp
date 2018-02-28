/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "abstractresourceitem.h"
#include "common.h"
#include "document.h"
#include "resourcevalidationworker.h"
#include "gui-qt/common/aboutdialog.h"
#include "gui-qt/common/graphics/zoomsettings.h"
#include "gui-qt/resources/mainwindow.ui.h"

#include "genericpropertieswidget.h"
#include "mttileset/mttilesetcentralwidget.h"
#include "mttileset/mttilesetpropertymanager.h"
#include "palette/palettecentralwidget.h"
#include "palette/palettepropertymanager.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _document(nullptr)
    , _ui(std::make_unique<Ui::MainWindow>())
    , _zoomSettings(new ZoomSettings(3.0, ZoomSettings::NTSC, this))
    , _undoGroup(new QUndoGroup(this))
{
    _ui->setupUi(this);

    // Update Menu
    {
        QAction* undoAction = _undoGroup->createUndoAction(this);
        QAction* redoAction = _undoGroup->createRedoAction(this);
        undoAction->setIcon(QIcon(":/icons/undo.svg"));
        undoAction->setShortcuts(QKeySequence::Undo);
        redoAction->setIcon(QIcon(":/icons/redo.svg"));
        redoAction->setShortcuts(QKeySequence::Redo);
        _ui->menu_Edit->addAction(undoAction);
        _ui->menu_Edit->addAction(redoAction);

        _zoomSettings->populateMenu(_ui->menu_View);
    }

    // Status Bar
    {
        QComboBox* aspectRatioComboBox = new QComboBox(this);
        _zoomSettings->setAspectRatioComboBox(aspectRatioComboBox);
        statusBar()->addPermanentWidget(aspectRatioComboBox);

        QComboBox* zoomComboBox = new QComboBox(this);
        _zoomSettings->setZoomComboBox(zoomComboBox);
        statusBar()->addPermanentWidget(zoomComboBox);
    }

    // Have the left and right docks take up the whole height of the MainWindow
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // ::NOTE Order MUST match ResourceTypeIndex::

    auto addWidgets = [this](AbstractResourceWidget* centralWidget, AbstractResourceWidget* propertiesWidget) {
        _resourceWidgets.append(centralWidget);
        _ui->centralStackedWidget->addWidget(centralWidget);
        _resourceWidgets.append(propertiesWidget);
        _ui->propertiesStackedWidget->addWidget(propertiesWidget);
    };

    _ui->centralStackedWidget->addWidget(new QLabel("Blank GUI", this));
    _ui->propertiesStackedWidget->addWidget(new QLabel("Blank Properties", this));

    addWidgets(new PaletteCentralWidget(this),
               new GenericPropertiesWidget(new PalettePropertiesManager(this), this));
    addWidgets(new MtTilesetCentralWidget(this, _zoomSettings),
               new GenericPropertiesWidget(new MtTilesetPropertiesManager(this), this));

    readSettings();

    setDocument(nullptr);

    connect(_undoGroup, &QUndoGroup::cleanChanged,
            this, &MainWindow::updateWindowTitle);

    connect(_ui->action_New, &QAction::triggered,
            this, &MainWindow::onMenuNew);
    connect(_ui->action_Open, &QAction::triggered,
            this, &MainWindow::onMenuOpen);
    connect(_ui->menu_OpenRecent, &OpenRecentMenu::recentFileSelected,
            this, &MainWindow::onMenuOpenRecent);
    connect(_ui->action_Save, &QAction::triggered,
            this, &MainWindow::onMenuSave);
    connect(_ui->action_Quit, &QAction::triggered,
            this, &MainWindow::close);

    connect(_ui->action_About, &QAction::triggered,
            this, &MainWindow::onMenuAbout);
}

MainWindow::~MainWindow() = default;

void MainWindow::loadDocument(const QString& filename)
{
    std::unique_ptr<Document> doc = std::make_unique<Document>();
    if (doc->loadDocument(filename)) {
        setDocument(std::move(doc));
        _ui->menu_OpenRecent->addFilename(filename);
    }
}

void MainWindow::setDocument(std::unique_ptr<Document>&& document)
{
    auto oldDocument = std::move(_document);

    if (_document) {
        _document->disconnect(this);
    }
    _document = std::move(document);

    _ui->resourcesTreeDock->setDocument(_document.get());
    _ui->errorListDock->setDocument(_document.get());

    // Close the errors dock as it is now empty
    _ui->errorListDock->close();

    _ui->action_Save->setEnabled(_document != nullptr);

    _ui->centralStackedWidget->setCurrentIndex(0);
    _ui->propertiesStackedWidget->setCurrentIndex(0);

    if (_document != nullptr) {
        Q_ASSERT(!_document->filename().isEmpty());

        _undoGroup->addStack(_document->undoStack());
        _document->undoStack()->setActive();

        _document->validationWorker()->validateAllResources();

        connect(_document.get(), &Document::filenameChanged,
                this, &MainWindow::updateWindowTitle);

        connect(_document.get(), &Document::selectedResourceChanged,
                this, &MainWindow::onSelectedResourceChanged);
    }

    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    if (_document != nullptr) {
        QString title = QFileInfo(_document->filename()).fileName();
        if (title.isEmpty()) {
            title = "untitled";
        }
        title.prepend("[*]");

        setWindowTitle(title);
        setWindowFilePath(_document->filename());
        setWindowModified(!_document->undoStack()->isClean());
    }
    else {
        setWindowTitle(QString());
        setWindowFilePath(QString());
        setWindowModified(false);
    }
}

void MainWindow::onSelectedResourceChanged()
{
    if (_document == nullptr) {
        _ui->centralStackedWidget->setCurrentIndex(0);
        _ui->propertiesStackedWidget->setCurrentIndex(0);
    }
    AbstractResourceItem* item = _document->selectedResource();

    if (item) {
        if (item->state() == ResourceState::NOT_LOADED) {
            item->loadResource();
        }

        int index = (int)item->resourceTypeIndex() + 1;

        _ui->centralStackedWidget->setCurrentIndex(index);
        _ui->propertiesStackedWidget->setCurrentIndex(index);
    }
    else {
        _ui->centralStackedWidget->setCurrentIndex(0);
        _ui->propertiesStackedWidget->setCurrentIndex(0);
    }

    for (auto* widget : _resourceWidgets) {
        widget->setResourceItem(item);
    }
}

bool MainWindow::unsavedChangesDialog()
{
    bool success = true;

    if (_document && !_document->undoStack()->isClean()) {
        QMessageBox dialog(QMessageBox::Warning,
                           tr("Save Changes?"),
                           tr("There are unsaved changes. Do you want to save?"),
                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                           this);
        dialog.exec();

        success = (dialog.result() == QMessageBox::Discard);
        if (dialog.result() == QMessageBox::Save) {
            success = onMenuSave();
        }
    }

    return success;
}

void MainWindow::onMenuNew()
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    auto doc = std::make_unique<Document>();

    QFileDialog saveDialog(this, "Create New Resource File");
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setNameFilter(doc->fileFilter());
    saveDialog.setDefaultSuffix(doc->defaultFileExtension());

    if (!doc->filename().isEmpty()) {
        saveDialog.selectFile(doc->filename());
    }
    saveDialog.exec();

    if (saveDialog.result() == QDialog::Accepted) {
        Q_ASSERT(saveDialog.selectedFiles().size() == 1);
        QString filename = saveDialog.selectedFiles().first();

        bool s = doc->saveDocument(filename);
        if (s) {
            _ui->menu_OpenRecent->addFilename(filename);
            setDocument(std::move(doc));
        }
    }
}

void MainWindow::onMenuOpen()
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    std::unique_ptr<Document> doc = std::make_unique<Document>();

    const QString filename = QFileDialog::getOpenFileName(
        this, tr("Open"), QString(), doc->fileFilter());

    if (!filename.isNull()) {
        loadDocument(filename);
    }
}

void MainWindow::onMenuOpenRecent(QString filename)
{
    Q_ASSERT(filename.isEmpty() == false);

    if (unsavedChangesDialog() == false) {
        return;
    }

    loadDocument(filename);
}

bool MainWindow::onMenuSave()
{
    Q_ASSERT(_document != nullptr);
    Q_ASSERT(!_document->filename().isEmpty());

    bool s = _document->saveDocument(_document->filename());
    if (s) {
        _ui->menu_OpenRecent->addFilename(_document->filename());
    }
    return s;
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

void MainWindow::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("window_state").toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("window_state", saveState());
}
