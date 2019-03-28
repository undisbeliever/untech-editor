/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QWidget>

namespace UnTech {
struct ErrorListItem;

namespace GuiQt {
class ZoomSettings;
class AbstractResourceItem;

class AbstractEditorWidget : public QWidget {
    Q_OBJECT

public:
    AbstractEditorWidget(QWidget* parent);
    ~AbstractEditorWidget() = default;

    // The returned QDockWidgets are owned and managed by the MainWindow class.
    // If the QDockWidgets is not added to mainWindow then they will be automatically
    // added to the RightDockWidgetArea.
    virtual QList<QDockWidget*> createDockWidgets(QMainWindow* mainWindow);

    virtual QWidget* statusBarWidget() const;
    virtual ZoomSettings* zoomSettings() const;
    virtual void populateMenu(QMenu* editMenu, QMenu* viewMenu);

    virtual bool setResourceItem(AbstractResourceItem* item) = 0;

    virtual void onErrorDoubleClicked(const ErrorListItem&);

protected:
    QDockWidget* createDockWidget(QWidget* widget, const QString& title, const QString& objectName);

signals:
    void zoomSettingsChanged();
};
}
}
