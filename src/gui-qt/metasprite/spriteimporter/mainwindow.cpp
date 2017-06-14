/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "gui-qt/metasprite/spriteimporter/mainwindow.ui.h"

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    connect(_ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow() = default;
