/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../animationtimer.h"
#include "gui-qt/abstracteditorwidget.h"
#include <QGraphicsItemGroup>
#include <memory>

namespace UnTech {
namespace GuiQt {
class PropertyListView;
class ZoomSettingsManager;

namespace Resources {
namespace BackgroundImage {

namespace Ui {
class EditorWidget;
}
class ResourceItem;
class BackgroundImageGraphicsItem;
class BackgroundImagePropertyManager;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

public:
    explicit EditorWidget(ZoomSettingsManager* zoomManager, QWidget* parent = nullptr);
    ~EditorWidget();

    virtual QString windowStateName() const final;
    virtual ZoomSettings* zoomSettings() const final;

    virtual bool setResourceItem(AbstractResourceItem* abstractItem) final;

private slots:
    void onResourceComplied();
    void updateAnimationFrameLabelText();
    void updatePixmap();
    void updateInvalidTiles();

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;

    // Used by the DockWidgets
    BackgroundImagePropertyManager* const _propertyManager;

    QGraphicsScene* const _graphicsScene;
    BackgroundImageGraphicsItem* const _biGraphicsItem;
    QGraphicsPixmapItem* const _pixmapItem;

    ResourceItem* _resourceItem;
    QGraphicsItemGroup* _errorGraphicsItem;

    AnimationTimer _animationTimer;
};

}
}
}
}
