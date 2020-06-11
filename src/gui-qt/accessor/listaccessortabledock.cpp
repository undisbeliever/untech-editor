/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "listaccessortabledock.h"

#include <QVBoxLayout>

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;

ListAccessorTableDock::ListAccessorTableDock(const QString& windowTitle,
                                             ListAccessorTableManager* manager,
                                             QWidget* parent)
    : QDockWidget(parent)
    , _view(new ListAccessorTableView)
    , _toolBar(new QToolBar(this))
{
    setWindowTitle(windowTitle);

    _view->setPropertyManager(manager);

    QWidget* widget = new QWidget(this);

    auto* layout = new QVBoxLayout(widget);
    widget->setLayout(layout);
    layout->setMargin(0);

    _toolBar->setIconSize(QSize(16, 16));

    layout->addWidget(_view);
    layout->addWidget(_toolBar);

    _view->viewActions()->populate(_toolBar);

    setWidget(widget);
}
