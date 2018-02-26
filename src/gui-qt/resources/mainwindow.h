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
class ZoomSettings;
class OpenRecentMenu;

namespace Resources {
namespace Ui {
class MainWindow;
}
class Document;
class AbstractResourceWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void loadDocument(const QString& filename);
    void setDocument(std::unique_ptr<Document>&& document);

private slots:
    void updateWindowTitle();

    void onSelectedResourceChanged();

private:
    bool unsavedChangesDialog();

private slots:
    void onMenuNew();
    void onMenuOpen();
    void onMenuOpenRecent(QString filename);
    bool onMenuSave();
    bool onMenuSaveAs();

    void onMenuAbout();

protected:
    virtual void closeEvent(QCloseEvent* event) override;

    void readSettings();
    void saveSettings();

private:
    std::unique_ptr<Document> _document;

    std::unique_ptr<Ui::MainWindow> _ui;

    ZoomSettings* _zoomSettings;
    QUndoGroup* _undoGroup;
    QVector<AbstractResourceWidget*> _resourceWidgets;
};
}
}
}
