/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "actions.h"
#include "document.h"
#include "framedock.h"
#include "framesetdock.h"
#include "sigraphicsscene.h"
#include "gui-qt/common/graphics/zoomsettings.h"
#include "gui-qt/metasprite/animation/animationdock.h"
#include "gui-qt/metasprite/layersettings.h"
#include "gui-qt/metasprite/spriteimporter/mainwindow.ui.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
    , _document(nullptr)
    , _actions(new Actions(this))
    , _zoomSettings(new ZoomSettings(4.0, ZoomSettings::NTSC, this))
    , _layerSettings(new LayerSettings(this))
    , _undoGroup(new QUndoGroup(this))
{
    _ui->setupUi(this);

    _ui->graphicsView->setZoomSettings(_zoomSettings);
    _ui->graphicsView->setRubberBandSelectionMode(Qt::ContainsItemShape);
    _graphicsScene = new SiGraphicsScene(_actions, _layerSettings, this);
    _ui->graphicsView->setScene(_graphicsScene);

    _frameSetDock = new FrameSetDock(_actions, this);
    addDockWidget(Qt::RightDockWidgetArea, _frameSetDock);

    _frameDock = new FrameDock(_actions, this);
    addDockWidget(Qt::RightDockWidgetArea, _frameDock);

    _animationDock = new Animation::AnimationDock(this);
    addDockWidget(Qt::RightDockWidgetArea, _animationDock);

    tabifyDockWidget(_frameSetDock, _frameDock);
    tabifyDockWidget(_frameSetDock, _animationDock);

    _frameSetDock->raise();

    setDocument(nullptr);

    setupMenubar();
    setupStatusbar();

    connect(_undoGroup, &QUndoGroup::cleanChanged, this, &MainWindow::updateWindowTitle);

    connect(_ui->actionNew, &QAction::triggered, this, &MainWindow::onActionNew);
    connect(_ui->actionOpen, &QAction::triggered, this, &MainWindow::onActionOpen);
    connect(_ui->actionSave, &QAction::triggered, this, &MainWindow::saveDocument);
    connect(_ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveDocumentAs);
    connect(_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupMenubar()
{
    QAction* undoAction = _undoGroup->createUndoAction(this);
    QAction* redoAction = _undoGroup->createRedoAction(this);

    undoAction->setShortcuts(QKeySequence::Undo);
    redoAction->setShortcuts(QKeySequence::Redo);

    _ui->menuEdit->addAction(undoAction);
    _ui->menuEdit->addAction(redoAction);
    _ui->menuEdit->addSeparator();
    _ui->menuEdit->addAction(_actions->raiseSelected());
    _ui->menuEdit->addAction(_actions->lowerSelected());
    _ui->menuEdit->addAction(_actions->cloneSelected());
    _ui->menuEdit->addAction(_actions->removeSelected());
    _ui->menuEdit->addSeparator();
    _ui->menuEdit->addAction(_actions->addFrame());
    _ui->menuEdit->addAction(_actions->cloneFrame());
    _ui->menuEdit->addAction(_actions->renameFrame());
    _ui->menuEdit->addAction(_actions->removeFrame());
    _ui->menuEdit->addSeparator();
    _ui->menuEdit->addAction(_actions->addRemoveTileHitbox());
    _ui->menuEdit->addSeparator();
    _ui->menuEdit->addAction(_actions->addFrameObject());
    _ui->menuEdit->addAction(_actions->addActionPoint());
    _ui->menuEdit->addAction(_actions->addEntityHitbox());

    QAction* zoomIn = _ui->menuView->addAction(tr("Zoom In"));
    zoomIn->setShortcut(Qt::CTRL + Qt::Key_Plus);
    connect(zoomIn, &QAction::triggered, _zoomSettings, &ZoomSettings::zoomIn);

    QAction* zoomOut = _ui->menuView->addAction(tr("Zoom Out"));
    zoomOut->setShortcut(Qt::CTRL + Qt::Key_Minus);
    connect(zoomOut, &QAction::triggered, _zoomSettings, &ZoomSettings::zoomOut);

    _ui->menuView->addSeparator();
    _layerSettings->populateMenu(_ui->menuView);
}

void MainWindow::setupStatusbar()
{
    QPushButton* layerButton = new QPushButton(tr("Layers"), this);
    QMenu* layerMenu = new QMenu(this);
    _layerSettings->populateMenu(layerMenu);
    layerButton->setMenu(layerMenu);
    statusBar()->addPermanentWidget(layerButton);

    _aspectRatioComboBox = new QComboBox(this);
    _zoomSettings->setAspectRatioComboBox(_aspectRatioComboBox);
    statusBar()->addPermanentWidget(_aspectRatioComboBox);

    _zoomComboBox = new QComboBox(this);
    _zoomSettings->setZoomComboBox(_zoomComboBox);
    statusBar()->addPermanentWidget(_zoomComboBox);
}

void MainWindow::setDocument(std::unique_ptr<Document> document)
{
    auto oldDocument = std::move(_document);

    _document = std::move(document);

    _actions->setDocument(_document.get());
    _graphicsScene->setDocument(_document.get());
    _frameSetDock->setDocument(_document.get());
    _frameDock->setDocument(_document.get());
    _animationDock->setDocument(_document.get());

    _ui->actionSave->setEnabled(_document != nullptr);
    _ui->actionSaveAs->setEnabled(_document != nullptr);

    if (_document != nullptr) {
        _undoGroup->addStack(_document->undoStack());
        _document->undoStack()->setActive();

        _ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    }
    else {
        _ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
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

void MainWindow::onActionNew()
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    setDocument(std::make_unique<Document>(this));
}

void MainWindow::onActionOpen()
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    QString filter = tr(Document::FILE_FILTER);

    const QString filename = QFileDialog::getOpenFileName(
        this, tr("Open FrameSet"), QString(), filter);

    if (!filename.isNull()) {
        auto doc = Document::loadDocument(filename);
        if (doc) {
            setDocument(std::move(doc));
        }
    }
}

bool MainWindow::saveDocument()
{
    if (_document->filename().isEmpty()) {
        return saveDocumentAs();
    }
    else {
        return _document->saveDocument(_document->filename());
    }
}

bool MainWindow::saveDocumentAs()
{
    QString filter = tr(Document::FILE_FILTER);

    const QString filename = QFileDialog::getSaveFileName(
        this, tr("Save FrameSet"), _document->filename(), filter);

    if (!filename.isNull()) {
        return _document->saveDocument(filename);
    }

    return false;
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
            success = saveDocument();
        }
    }

    return success;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (unsavedChangesDialog()) {
        event->accept();
    }
    else {
        event->ignore();
    }
}
