/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "document.h"
#include "gui-qt/common/graphics/zoomsettings.h"

#include <QStatusBar>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources;

MainWindow::MainWindow(QWidget* parent)
    : AbstractMainWindow(parent)
    , _zoomSettings(new ZoomSettings(3.0, ZoomSettings::NTSC, this))
{
    documentChangedEvent(nullptr, nullptr);

    setupMenubar();
    setupStatusbar();
    readSettings();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupMenubar()
{
    _zoomSettings->populateMenu(_viewMenu);
    _viewMenu->addSeparator();
}

void MainWindow::setupStatusbar()
{
    _aspectRatioComboBox = new QComboBox(this);
    _zoomSettings->setAspectRatioComboBox(_aspectRatioComboBox);
    statusBar()->addPermanentWidget(_aspectRatioComboBox);

    _zoomComboBox = new QComboBox(this);
    _zoomSettings->setZoomComboBox(_zoomComboBox);
    statusBar()->addPermanentWidget(_zoomComboBox);
}

std::unique_ptr<AbstractDocument> MainWindow::createDocumentInstance()
{
    return std::make_unique<Document>();
}

void MainWindow::documentChangedEvent(AbstractDocument* /* abstractDocument */,
                                      AbstractDocument* /* abstractOldDocument */)
{
}
