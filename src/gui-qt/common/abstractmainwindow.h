/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QMainWindow>
#include <QUndoGroup>
#include <memory>

namespace UnTech {
namespace GuiQt {

class AbstractDocument;
class OpenRecentMenu;

class AbstractMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit AbstractMainWindow(QWidget* parent = nullptr);
    ~AbstractMainWindow();

    AbstractDocument* document() const { return _document.get(); }

    void newDocument();
    void loadDocument(const QString& filename);

protected:
    virtual void closeEvent(QCloseEvent* event) override;

    virtual void documentChangedEvent(AbstractDocument* document, AbstractDocument* oldDocument) = 0;
    virtual std::unique_ptr<AbstractDocument> createDocumentInstance() = 0;

    // MUST be called in the child class after setting up the GUI
    void readSettings();

    // will automatically be called in closeEvent
    void saveSettings();

private:
    void setDocument(std::unique_ptr<AbstractDocument> document);
    bool unsavedChangesDialog();

private slots:
    void updateWindowTitle();

    void onMenuNew();
    void onMenuOpen();
    void onMenuOpenRecent(QString filename);
    bool onMenuSave();
    bool onMenuSaveAs();

    void onMenuAbout();

private:
    std::unique_ptr<AbstractDocument> _document;
    QUndoGroup* _undoGroup;

    OpenRecentMenu* _openRecentMenu;
    QAction* _saveAction;
    QAction* _saveAsAction;

protected:
    QMenu* _fileMenu;
    QMenu* _editMenu;
    QMenu* _viewMenu;
    QMenu* _aboutMenu;
};
}
}
