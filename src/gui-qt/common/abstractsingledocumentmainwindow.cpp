/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractsingledocumentmainwindow.h"
#include "aboutdialog.h"
#include "abstractdocument.h"
#include "openrecentmenu.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>

using namespace UnTech::GuiQt;

AbstractSingleDocumentMainWindow::AbstractSingleDocumentMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _document(nullptr)
    , _undoGroup(new QUndoGroup(this))
{
    _fileMenu = menuBar()->addMenu(tr("&File"));
    {
        _fileMenu->addAction(QIcon(":/icons/new.svg"), tr("&New"),
                             this, &AbstractSingleDocumentMainWindow::onMenuNew,
                             Qt::CTRL + Qt::Key_N);
        _fileMenu->addAction(QIcon(":/icons/open.svg"), tr("&Open"),
                             this, &AbstractSingleDocumentMainWindow::onMenuOpen,
                             Qt::CTRL + Qt::Key_O);

        _openRecentMenu = new OpenRecentMenu(_fileMenu);
        connect(_openRecentMenu, &OpenRecentMenu::recentFileSelected,
                this, &AbstractSingleDocumentMainWindow::onMenuOpenRecent);
        _fileMenu->addMenu(_openRecentMenu);

        _fileMenu->addSeparator();
        _saveAction = _fileMenu->addAction(QIcon(":/icons/save.svg"), tr("&Save"),
                                           this, &AbstractSingleDocumentMainWindow::onMenuSave,
                                           Qt::CTRL + Qt::Key_S);
        _saveAsAction = _fileMenu->addAction(tr("Save &As"), this,
                                             &AbstractSingleDocumentMainWindow::onMenuSaveAs,
                                             Qt::CTRL + Qt::SHIFT + Qt::Key_S);
        _fileMenu->addSeparator();
        _fileMenu->addAction(tr("&Quit"), this,
                             &AbstractSingleDocumentMainWindow::close);
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
                              this, &AbstractSingleDocumentMainWindow::onMenuAbout);
    }

    _saveAction->setEnabled(false);
    _saveAsAction->setEnabled(false);

    connect(_undoGroup, &QUndoGroup::cleanChanged,
            this, &AbstractSingleDocumentMainWindow::updateWindowTitle);
}

AbstractSingleDocumentMainWindow::~AbstractSingleDocumentMainWindow() = default;

void AbstractSingleDocumentMainWindow::newDocument()
{
    setDocument(createDocumentInstance());
}

void AbstractSingleDocumentMainWindow::loadDocument(const QString& filename)
{
    std::unique_ptr<AbstractDocument> doc = createDocumentInstance();
    if (doc->loadDocument(filename)) {
        setDocument(std::move(doc));
        _openRecentMenu->addFilename(filename);
    }
}

void AbstractSingleDocumentMainWindow::setDocument(std::unique_ptr<AbstractDocument> document)
{
    auto oldDocument = std::move(_document);

    if (oldDocument) {
        oldDocument->disconnect(this);
    }
    _document = std::move(document);

    _saveAction->setEnabled(_document != nullptr);
    _saveAsAction->setEnabled(_document != nullptr);

    if (_document != nullptr) {
        _undoGroup->addStack(_document->undoStack());
        _document->undoStack()->setActive();

        connect(_document.get(), &AbstractDocument::filenameChanged,
                this, &AbstractSingleDocumentMainWindow::updateWindowTitle);
    }

    documentChangedEvent(_document.get(), oldDocument.get());

    updateWindowTitle();
}

void AbstractSingleDocumentMainWindow::updateWindowTitle()
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

void AbstractSingleDocumentMainWindow::onMenuNew()
{
    if (unsavedChangesDialog() == false) {
        return;
    }
    newDocument();
}

void AbstractSingleDocumentMainWindow::onMenuOpen()
{
    if (unsavedChangesDialog() == false) {
        return;
    }

    std::unique_ptr<AbstractDocument> doc = createDocumentInstance();

    const QString filename = QFileDialog::getOpenFileName(
        this, tr("Open"), QString(), doc->fileFilter(),
        nullptr, QFileDialog::DontUseNativeDialog);

    if (!filename.isNull()) {
        loadDocument(filename);
    }
}

void AbstractSingleDocumentMainWindow::onMenuOpenRecent(QString filename)
{
    Q_ASSERT(!filename.isEmpty());

    if (unsavedChangesDialog() == false) {
        return;
    }
    loadDocument(filename);
}

bool AbstractSingleDocumentMainWindow::onMenuSave()
{
    Q_ASSERT(_document != nullptr);

    if (_document->filename().isEmpty()) {
        return onMenuSaveAs();
    }
    else {
        bool s = _document->saveDocument(_document->filename());
        if (s) {
            _openRecentMenu->addFilename(_document->filename());
        }
        return s;
    }
}

bool AbstractSingleDocumentMainWindow::onMenuSaveAs()
{
    Q_ASSERT(_document != nullptr);

    QFileDialog saveDialog(this, "Save");
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setNameFilter(_document->fileFilter());
    saveDialog.setDefaultSuffix(_document->defaultFileExtension());
    saveDialog.setOption(QFileDialog::DontUseNativeDialog);

    if (!_document->filename().isEmpty()) {
        saveDialog.selectFile(_document->filename());
    }
    saveDialog.exec();

    if (saveDialog.result() == QDialog::Accepted) {
        QString filename = saveDialog.selectedFiles().first();

        bool s = _document->saveDocument(filename);
        if (s) {
            _openRecentMenu->addFilename(_document->filename());
        }
        return s;
    }

    return false;
}

void AbstractSingleDocumentMainWindow::onMenuAbout()
{
    AboutDialog().exec();
}

bool AbstractSingleDocumentMainWindow::unsavedChangesDialog()
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

void AbstractSingleDocumentMainWindow::readSettings()
{
    QSettings settings;

    if (!settings.contains("window_state")) {
        // required to prevent docks from resizing on layout change
        saveSettings();
    }

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("window_state").toByteArray());
}

void AbstractSingleDocumentMainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("window_state", saveState());
}

void AbstractSingleDocumentMainWindow::closeEvent(QCloseEvent* event)
{
    if (unsavedChangesDialog()) {
        event->accept();
        saveSettings();
    }
    else {
        event->ignore();
    }
}
