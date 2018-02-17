/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../abstractresourcewidget.h"
#include "../animationtimer.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;

namespace Resources {
namespace Ui {
class MtTilesetCentralWidget;
}

class Document;
class AbstractResourceItem;
class MtTilesetResourceItem;
class MtTilesetGraphicsItem;

class MtTilesetCentralWidget : public AbstractResourceWidget {
    Q_OBJECT

public:
    MtTilesetCentralWidget(QWidget* parent,
                           ZoomSettings* ZoomSettings);
    ~MtTilesetCentralWidget();

    virtual ResourceTypeIndex resourceTypeIndex() const final;

    virtual void setResourceItem(AbstractResourceItem* item) final;

private:
    void updateFrameLabel();
    void clearGui();

private slots:
    void onMtTilesetDataChanged();

    void onAnimationFrameAdvance();
    void onPreviousClicked();
    void onNextClicked();

private:
    std::unique_ptr<Ui::MtTilesetCentralWidget> _ui;
    QGraphicsScene* _graphicsScene;

    MtTilesetResourceItem* _tileset;
    MtTilesetGraphicsItem* _graphicsItem;

    AnimationTimer _animationTimer;
};

class MtTilesetGraphicsItem : public QGraphicsItem {
public:
    static const QColor GRID_COLOR;

public:
    explicit MtTilesetGraphicsItem(MtTilesetResourceItem* item);
    ~MtTilesetGraphicsItem() = default;

    void reloadAnimationFrame() { setAnimationFrameIndex(_animationFrameIndex); }

    void prevAnimationFrame() { setAnimationFrameIndex(_animationFrameIndex - 1); }
    void nextAnimationFrame() { setAnimationFrameIndex(_animationFrameIndex + 1); }
    int animationFrameIndex() const { return _animationFrameIndex; }

private:
    void setAnimationFrameIndex(int index);

public:
    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

private:
    MtTilesetResourceItem* const _tileset;
    QPixmap _pixmap;

    int _animationFrameIndex;
};
}
}
}
