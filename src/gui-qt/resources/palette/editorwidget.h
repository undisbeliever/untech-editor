/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../animationtimer.h"
#include "gui-qt/abstracteditorwidget.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <memory>

namespace UnTech {
namespace GuiQt {
class PropertyListView;

namespace Resources {
namespace Palette {

namespace Ui {
class EditorWidget;
}
class ResourceItem;
class PaletteGraphicsItem;
class PalettePropertyManager;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget();

    virtual QString windowStateName() const final;

    void setResourceItem(ResourceItem* item);
    virtual bool setResourceItem(AbstractResourceItem* abstractItem) final;

private:
    void updateFrameLabel();
    void centerGraphicsItem();
    void clearGui();

private slots:
    void onPaletteDataChanged();

    void onAnimationStarted();
    void onAnimationFrameAdvance();
    void onAnimationStopped();

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;

    // Used by the DockWidgets
    PalettePropertyManager* const _propertyManager;

    QGraphicsScene* const _graphicsScene;

    ResourceItem* _palette;
    PaletteGraphicsItem* _graphicsItem;

    AnimationTimer _animationTimer;
};

class PaletteGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    static constexpr int PALETTE_SCALE = 24;
    static constexpr int LINE_WIDTH = 1;
    static constexpr int FRAME_LINE_WIDTH = 2;
    static constexpr int FRAME_OVERHANG = PALETTE_SCALE / 2;

    static const QColor LINE_COLOR;
    static const QColor FRAME_LINE_COLOR;

public:
    explicit PaletteGraphicsItem(ResourceItem* item);
    ~PaletteGraphicsItem() = default;

public slots:
    void updatePixmap();

public:
    void setFrameIndex(int index);
    void nextAnimationFrame() { setFrameIndex(_frameIndex + 1); }
    int frameIndex() const { return _frameIndex; }

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

private:
    ResourceItem* const _palette;
    QPixmap _pixmap;

    // if < 0 then the whole image is shown
    int _frameIndex;
};
}
}
}
}
