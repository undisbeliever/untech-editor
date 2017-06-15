/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "document.h"
#include "framesetdock.h"
#include "gui-qt/metasprite/spriteimporter/mainwindow.ui.h"

#include <QFileDialog>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    _frameSetDock = new FrameSetDock(this);
    addDockWidget(Qt::RightDockWidgetArea, _frameSetDock);

    setDocument(nullptr);

    connect(_ui->actionNew, SIGNAL(triggered()), this, SLOT(onActionNew()));
    connect(_ui->actionOpen, SIGNAL(triggered()), this, SLOT(onActionOpen()));
    connect(_ui->actionSave, SIGNAL(triggered()), this, SLOT(onActionSave()));
    connect(_ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(onActionSaveAs()));
    connect(_ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow() = default;

void MainWindow::setDocument(std::unique_ptr<Document> document)
{
    auto oldDocument = std::move(_document);

    _document = std::move(document);

    _frameSetDock->setDocument(_document.get());

    _ui->actionSave->setEnabled(_document != nullptr);
    _ui->actionSaveAs->setEnabled(_document != nullptr);
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
