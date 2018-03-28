/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstracteditor.h"

#include <QMenu>

using namespace UnTech::GuiQt;

QWidget* AbstractEditor::propertyWidget() const
{
    return nullptr;
}

QWidget* AbstractEditor::statusBarWidget() const
{
    return nullptr;
}

void AbstractEditor::populateMenu(QMenu*, QMenu*)
{
}
