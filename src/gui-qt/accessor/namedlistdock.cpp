/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "namedlistdock.h"
#include "abstractaccessors.h"
#include "namedlistview.h"
#include "gui-qt/common/validatoritemdelegate.h"

#include <QToolBar>
#include <QVBoxLayout>

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;

NamedListDock::NamedListDock(QWidget* parent)
    : QDockWidget(parent)
    , _namedListView(new NamedListView(this))
    , _toolBar(new QToolBar(this))
{
    QWidget* widget = new QWidget(this);

    auto* layout = new QVBoxLayout(widget);
    widget->setLayout(layout);
    layout->setMargin(0);

    _toolBar->setIconSize(QSize(16, 16));

    layout->addWidget(_namedListView);
    layout->addWidget(_toolBar);

    _namedListView->namedListActions()->populate(_toolBar);

    setWidget(widget);
}

void NamedListDock::setAccessor(AbstractNamedListAccessor* accessor)
{
    _namedListView->setAccessor(accessor);

    if (accessor) {
        setWindowTitle(accessor->typeNamePlural());
    }

    setEnabled(accessor != nullptr);
}
