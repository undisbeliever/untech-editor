/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "multipleselectiontabledock.h"
#include "multipleselectiontableview.h"

#include <QVBoxLayout>

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;

/*
 * I included the accessor list in the constructor so the viewActions and
 * toolBar are prefilled on construction (see the
 * `MetaSprite::MetaSprite::Actions` class).
 */

MultipleSelectionTableDock::MultipleSelectionTableDock(const QString& windowTitle,
                                                       const QList<ListAccessorTableManager*>& managers,
                                                       const QStringList& columns,
                                                       QWidget* parent)
    : QDockWidget(parent)
    , _view(new MultipleSelectionTableView(this))
    , _toolBar(new QToolBar(this))
{
    setWindowTitle(windowTitle);

    _view->setPropertyManagers(managers, columns);

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
