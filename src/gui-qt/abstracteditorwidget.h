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
class PropertyListManager;

class AbstractEditorWidget : public QMainWindow {
    Q_OBJECT

public:
    AbstractEditorWidget(QWidget* parent);
    ~AbstractEditorWidget() = default;

    virtual QWidget* statusBarWidget() const;
    virtual ZoomSettings* zoomSettings() const;
    virtual void populateMenu(QMenu* editMenu, QMenu* viewMenu);

    virtual QString windowStateName() const = 0;
    virtual bool setResourceItem(AbstractResourceItem* item) = 0;

    virtual void onErrorDoubleClicked(const ErrorListItem&);

protected:
    QDockWidget* createDockWidget(QWidget* widget, const QString& title, const QString& objectName);
    QDockWidget* createPropertyDockWidget(PropertyListManager* manager, const QString& title, const QString& objectName);

signals:
    void zoomSettingsChanged();
};
}
}
