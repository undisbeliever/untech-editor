/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "abstractresourceitem.h"
#include "common.h"
#include "document.h"
#include "resourcestreedock.h"
#include "gui-qt/common/graphics/zoomsettings.h"
#include "palette/palettecentralwidget.h"

#include <QLabel>
#include <QStatusBar>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources;

MainWindow::MainWindow(QWidget* parent)
    : AbstractMainWindow(parent)
    , _zoomSettings(new ZoomSettings(3.0, ZoomSettings::NTSC, this))
{
    _resourcesTreeDock = new ResourcesTreeDock(this);
    addDockWidget(Qt::LeftDockWidgetArea, _resourcesTreeDock);

    _propertiesDock = new QDockWidget(tr("Properties"), this);
    _propertiesDock->setObjectName(QStringLiteral("PropertiesDock"));
    _propertiesDock->setFeatures(QDockWidget::DockWidgetMovable);
    _propertiesDock->setMinimumSize(150, 150);
    _propertiesStackedWidget = new QStackedWidget(this);
    _propertiesDock->setWidget(_propertiesStackedWidget);
    addDockWidget(Qt::RightDockWidgetArea, _propertiesDock);

    _centralStackedWidget = new QStackedWidget(this);
    this->setCentralWidget(_centralStackedWidget);

    auto addCW = [this](auto* centralWidget) {
        _resourceWidgets.append(centralWidget);
        _centralStackedWidget->addWidget(centralWidget);
    };

    // ::NOTE Order MUST match ResourceTypeIndex::

    _centralStackedWidget->addWidget(new QLabel("Blank GUI", this));
    _propertiesStackedWidget->addWidget(new QLabel("Blank Properties", this));

    addCW(new PaletteCentralWidget(this));
    _propertiesStackedWidget->addWidget(new QLabel("Palette Properties", this));

    _centralStackedWidget->addWidget(new QLabel("MetaTile Tileset GUI", this));
    _propertiesStackedWidget->addWidget(new QLabel("MetaTile Tileset properties", this));

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

void MainWindow::documentChangedEvent(AbstractDocument* abstractDocument,
                                      AbstractDocument* abstractOldDocument)
{
    Document* document = qobject_cast<Document*>(abstractDocument);
    Document* oldDocument = qobject_cast<Document*>(abstractOldDocument);

    if (oldDocument) {
        oldDocument->disconnect(this);
    }

    _document = document;
    _resourcesTreeDock->setDocument(document);

    _centralStackedWidget->setCurrentIndex(0);
    _propertiesStackedWidget->setCurrentIndex(0);

    if (document) {
        connect(document, &Document::selectedResourceChanged,
                this, &MainWindow::onSelectedResourceChanged);
    }
}

void MainWindow::onSelectedResourceChanged()
{
    if (_document == nullptr) {
        _centralStackedWidget->setCurrentIndex(0);
        _propertiesStackedWidget->setCurrentIndex(0);
    }
    AbstractResourceItem* item = _document->selectedResource();

    if (item) {
        int index = (int)item->resourceTypeIndex() + 1;

        _centralStackedWidget->setCurrentIndex(index);
        _propertiesStackedWidget->setCurrentIndex(index);
    }
    else {
        _centralStackedWidget->setCurrentIndex(0);
        _propertiesStackedWidget->setCurrentIndex(0);
    }

    for (auto* widget : _resourceWidgets) {
        widget->setResourceItem(item);
    }
}
