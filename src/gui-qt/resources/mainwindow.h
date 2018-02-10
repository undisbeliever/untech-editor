/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/abstractmainwindow.h"
#include <QComboBox>
#include <QStackedWidget>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;
class ZoomableGraphicsView;

namespace Resources {
class Document;
class ResourcesTreeDock;

class MainWindow : public AbstractMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupMenubar();
    void setupStatusbar();

protected:
    virtual void documentChangedEvent(AbstractDocument* document,
                                      AbstractDocument* oldDocument) final;
    virtual std::unique_ptr<AbstractDocument> createDocumentInstance() final;

private slots:
    void onSelectedResourceChanged();

private:
    Document* _document;
    ZoomSettings* _zoomSettings;

    QComboBox* _aspectRatioComboBox;
    QComboBox* _zoomComboBox;

    QStackedWidget* _centralStackedWidget;
    QStackedWidget* _propertiesStackedWidget;

    ResourcesTreeDock* _resourcesTreeDock;
    QDockWidget* _propertiesDock;
};
}
}
}