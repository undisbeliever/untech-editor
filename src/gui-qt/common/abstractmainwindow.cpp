/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractmainwindow.h"
#include "aboutdialog.h"
#include "abstractdocument.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>

using namespace UnTech::GuiQt;

const int AbstractMainWindow::MAX_OPEN_RECENT_SIZE = 6;

AbstractMainWindow::AbstractMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _document(nullptr)
    , _undoGroup(new QUndoGroup(this))
{
    _fileMenu = menuBar()->addMenu(tr("&File"));
    {
        _fileMenu->addAction(QIcon(":/icons/new.svg"), tr("&New"),
                             this, &AbstractMainWindow::onMenuNew,
                             Qt::CTRL + Qt::Key_N);
        _fileMenu->addAction(QIcon(":/icons/open.svg"), tr("&Open"),
                             this, &AbstractMainWindow::onMenuOpen,
                             Qt::CTRL + Qt::Key_O);

        auto* openRecentMenu = _fileMenu->addMenu(tr("Open &Recent"));
        openRecentMenu->setToolTipsVisible(true);
        for (int i = 0; i < MAX_OPEN_RECENT_SIZE; i++) {
            _openRecentActions.append(openRecentMenu->addAction(QString()));
        }
        connect(openRecentMenu, &QMenu::triggered,
                this, &AbstractMainWindow::onMenuOpenRecent);

        _fileMenu->addSeparator();
        _saveAction = _fileMenu->addAction(QIcon(":/icons/save.svg"), tr("&Save"),
                                           this, &AbstractMainWindow::onMenuSave,
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
        undoAction->setIcon(QIcon(":/icons/undo.svg"));
        undoAction->setShortcuts(QKeySequence::Undo);
        redoAction->setIcon(QIcon(":/icons/redo.svg"));
        redoAction->setShortcuts(QKeySequence::Redo);
        _editMenu->addAction(undoAction);
        _editMenu->addAction(redoAction);
    }

    _viewMenu = menuBar()->addMenu(tr("&View"));

    _aboutMenu = menuBar()->addMenu(tr("&Help"));
    {
        _aboutMenu->addAction(tr("About"),
                              this, &AbstractMainWindow::onMenuAbout);
    }

    _saveAction->setEnabled(false);
    _saveAsAction->setEnabled(false);

    updateOpenRecentMenu();

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
        addToRecentFilesList(filename);
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
        loadDocument(filename);
    }
}

void AbstractMainWindow::onMenuOpenRecent(QAction* action)
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    QString filename = action->data().toString();
    if (!filename.isEmpty()) {
        loadDocument(filename);
    }
}

bool AbstractMainWindow::onMenuSave()
{
    Q_ASSERT(_document != nullptr);

    if (_document->filename().isEmpty()) {
        return onMenuSaveAs();
    }
    else {
        bool s = _document->saveDocument(_document->filename());
        if (s) {
            addToRecentFilesList(_document->filename());
        }
        return s;
    }
}

bool AbstractMainWindow::onMenuSaveAs()
{
    Q_ASSERT(_document != nullptr);

    const QString filename = QFileDialog::getSaveFileName(
        this, tr("Save"),
        _document->filename(), _document->fileFilter());

    if (!filename.isEmpty()) {
        bool s = _document->saveDocument(filename);
        if (s) {
            addToRecentFilesList(_document->filename());
        }
        return s;
    }

    return false;
}

void AbstractMainWindow::onMenuAbout()
{
    AboutDialog().exec();
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

void AbstractMainWindow::addToRecentFilesList(const QString& filename)
{
    QSettings settings;
    QStringList files = settings.value("recent_files").toStringList();

    files.removeAll(filename);
    files.prepend(filename);
    while (files.size() > MAX_OPEN_RECENT_SIZE) {
        files.removeLast();
    }

    settings.setValue("recent_files", files);

    updateOpenRecentMenu();
}

void AbstractMainWindow::updateOpenRecentMenu()
{
    QSettings settings;
    const QStringList files = settings.value("recent_files").toStringList();

    for (int i = 0; i < MAX_OPEN_RECENT_SIZE; i++) {
        QAction* action = _openRecentActions.at(i);

        action->setVisible(i < files.size());
        if (i < files.size()) {
            const QString& file = files.at(i);
            action->setText(QFileInfo(file).fileName());
            action->setToolTip(file);
            action->setData(file);
        }
    }
}

void AbstractMainWindow::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("window_state").toByteArray());
}

void AbstractMainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("window_state", saveState());
}

void AbstractMainWindow::closeEvent(QCloseEvent* event)
{
    if (unsavedChangesDialog()) {
        event->accept();
        saveSettings();
    }
    else {
        event->ignore();
    }
}
