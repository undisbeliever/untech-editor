/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstracteditorwidget.h"
#include "abstractresourceitem.h"
#include "gui-qt/common/properties/propertylistview.h"
#include "gui-qt/common/properties/propertytableview.h"

using namespace UnTech::GuiQt;

AbstractEditorWidget::AbstractEditorWidget(QWidget* parent)
    : QMainWindow(parent)
{
}

ZoomSettings* AbstractEditorWidget::zoomSettings() const
{
    return nullptr;
}

void AbstractEditorWidget::populateMenu(QMenu*, QMenu*)
{
}

QWidget* AbstractEditorWidget::statusBarWidget() const
{
    return nullptr;
}

void AbstractEditorWidget::onErrorDoubleClicked(const UnTech::ErrorListItem&)
{
}

QDockWidget* AbstractEditorWidget::createDockWidget(QWidget* widget, const QString& title, const QString& objectName)
{
    QDockWidget* dockWidget = qobject_cast<QDockWidget*>(widget);

    if (dockWidget == nullptr) {
        dockWidget = new QDockWidget();
        dockWidget->setWidget(widget);
    }

    dockWidget->setWindowTitle(title);
    dockWidget->setObjectName(objectName);

    return dockWidget;
}

QDockWidget* AbstractEditorWidget::createPropertyDockWidget(PropertyListManager* manager, const QString& title, const QString& objectName)
{
    return createDockWidget(new PropertyListView(manager), title, objectName);
}
