/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractmainwindow.h"
#include "abstractdocument.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>

using namespace UnTech::GuiQt;

AbstractMainWindow::AbstractMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _document(nullptr)
    , _undoGroup(new QUndoGroup(this))
{
    _fileMenu = menuBar()->addMenu(tr("&File"));
    {
        _fileMenu->addAction(tr("&New"), this,
                             &AbstractMainWindow::onMenuNew,
                             Qt::CTRL + Qt::Key_N);
        _fileMenu->addAction(tr("&Open"), this,
                             &AbstractMainWindow::onMenuOpen,
                             Qt::CTRL + Qt::Key_O);
        _fileMenu->addSeparator();
        _saveAction = _fileMenu->addAction(tr("&Save"), this,
                                           &AbstractMainWindow::onMenuSave,
                                           Qt::CTRL + Qt::Key_S);
        _saveAsAction = _fileMenu->addAction(tr("Save &As"), this,
                                             &AbstractMainWindow::onMenuSaveAs,
                                             Qt::CTRL + Qt::SHIFT + Qt::Key_S);
        _fileMenu->addSeparator();
        _fileMenu->addAction(tr("&Quit"), this,
                             &AbstractMainWindow::close);
    }

    _editMenu = menuBar()->addMenu(tr("&Edit"));
    {
        QAction* undoAction = _undoGroup->createUndoAction(this);
        QAction* redoAction = _undoGroup->createRedoAction(this);
        undoAction->setShortcuts(QKeySequence::Undo);
        redoAction->setShortcuts(QKeySequence::Redo);
        _editMenu->addAction(undoAction);
        _editMenu->addAction(redoAction);
    }

    _viewMenu = menuBar()->addMenu(tr("&View"));

    _saveAction->setEnabled(false);
    _saveAsAction->setEnabled(false);

    connect(_undoGroup, &QUndoGroup::cleanChanged,
            this, &AbstractMainWindow::updateWindowTitle);
}

AbstractMainWindow::~AbstractMainWindow() = default;

void AbstractMainWindow::newDocument()
{
    setDocument(createDocumentInstance());
}

void AbstractMainWindow::loadDocument(const QString& filename)
{
    std::unique_ptr<AbstractDocument> doc = createDocumentInstance();
    if (doc->loadDocument(filename)) {
        setDocument(std::move(doc));
    }
}

void AbstractMainWindow::setDocument(std::unique_ptr<AbstractDocument> document)
{
    auto oldDocument = std::move(_document);

    if (_document) {
        _document->disconnect(this);
    }
    _document = std::move(document);

    _saveAction->setEnabled(_document != nullptr);
    _saveAsAction->setEnabled(_document != nullptr);

    if (_document != nullptr) {
        _undoGroup->addStack(_document->undoStack());
        _document->undoStack()->setActive();

        connect(_document.get(), &AbstractDocument::filenameChanged,
                this, &AbstractMainWindow::updateWindowTitle);
    }

    documentChangedEvent(_document.get(), oldDocument.get());

    updateWindowTitle();
}

void AbstractMainWindow::updateWindowTitle()
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

void AbstractMainWindow::onMenuNew()
{
    if (unsavedChangesDialog() == false) {
        return;
    }
    newDocument();
}

void AbstractMainWindow::onMenuOpen()
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    std::unique_ptr<AbstractDocument> doc = createDocumentInstance();

    const QString filename = QFileDialog::getOpenFileName(
        this, tr("Open"), QString(), doc->fileFilter());

    if (!filename.isNull()) {
        if (doc->loadDocument(filename)) {
            setDocument(std::move(doc));
        }
    }
}

bool AbstractMainWindow::onMenuSave()
{
    Q_ASSERT(_document != nullptr);

    if (_document->filename().isEmpty()) {
        return onMenuSaveAs();
    }
    else {
        return _document->saveDocument(_document->filename());
    }
}

bool AbstractMainWindow::onMenuSaveAs()
{
    Q_ASSERT(_document != nullptr);

    const QString filename = QFileDialog::getSaveFileName(
        this, tr("Save"),
        _document->filename(), _document->fileFilter());

    if (!filename.isNull()) {
        return _document->saveDocument(filename);
    }

    return false;
}

bool AbstractMainWindow::unsavedChangesDialog()
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

void AbstractMainWindow::closeEvent(QCloseEvent* event)
{
    if (unsavedChangesDialog()) {
        event->accept();
    }
    else {
        event->ignore();
    }
}
