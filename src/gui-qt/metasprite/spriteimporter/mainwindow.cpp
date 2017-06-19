/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "document.h"
#include "framedock.h"
#include "framesetdock.h"
#include "gui-qt/metasprite/spriteimporter/mainwindow.ui.h"

#include <QFileDialog>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
    , _document(nullptr)
    , _undoGroup(new QUndoGroup(this))
{
    _ui->setupUi(this);

    _frameSetDock = new FrameSetDock(this);
    addDockWidget(Qt::RightDockWidgetArea, _frameSetDock);

    _frameDock = new FrameDock(this);
    addDockWidget(Qt::RightDockWidgetArea, _frameDock);

    tabifyDockWidget(_frameSetDock, _frameDock);

    _frameSetDock->raise();

    setDocument(nullptr);

    createUndoActions();

    connect(_undoGroup, SIGNAL(cleanChanged(bool)), this, SLOT(updateWindowTitle()));

    connect(_ui->actionNew, SIGNAL(triggered()), this, SLOT(onActionNew()));
    connect(_ui->actionOpen, SIGNAL(triggered()), this, SLOT(onActionOpen()));
    connect(_ui->actionSave, SIGNAL(triggered()), this, SLOT(onActionSave()));
    connect(_ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(onActionSaveAs()));
    connect(_ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow() = default;

void MainWindow::createUndoActions()
{
    QAction* undoAction = _undoGroup->createUndoAction(this);
    QAction* redoAction = _undoGroup->createRedoAction(this);

    undoAction->setShortcuts(QKeySequence::Undo);
    redoAction->setShortcuts(QKeySequence::Redo);

    _ui->menuEdit->addAction(undoAction);
    _ui->menuEdit->addAction(redoAction);
}

void MainWindow::setDocument(std::unique_ptr<Document> document)
{
    auto oldDocument = std::move(_document);

    _document = std::move(document);

    _frameSetDock->setDocument(_document.get());
    _frameDock->setDocument(_document.get());

    _ui->actionSave->setEnabled(_document != nullptr);
    _ui->actionSaveAs->setEnabled(_document != nullptr);

    if (_document != nullptr) {
        _undoGroup->addStack(_document->undoStack());
        _document->undoStack()->setActive();
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
    setDocument(std::make_unique<Document>(this));
}

void MainWindow::onActionOpen()
{
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

void MainWindow::onActionSave()
{
    if (_document->filename().isEmpty()) {
        onActionSaveAs();
    }
    else {
        _document->saveDocument(_document->filename());
    }
}

void MainWindow::onActionSaveAs()
{
    QString filter = tr(Document::FILE_FILTER);

    const QString filename = QFileDialog::getSaveFileName(
        this, tr("Save FrameSet"), _document->filename(), filter);

    if (!filename.isNull()) {
        _document->saveDocument(filename);
    }
}
