/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/resources/animationtimer.h"
#include "models/resources/error-list.h"
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QPixmap>
#include <QVector>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;
class AbstractResourceItem;

namespace Resources {
namespace Ui {
class AnimationFramesInputWidget;
}
class AnimationFramesInputGraphicsItem;
class AnimationFramesInputWidget;

namespace RES = UnTech::Resources;

class AnimationFramesInputWidget : public QWidget {
    Q_OBJECT

public:
    AnimationFramesInputWidget(QWidget* parent = nullptr, ZoomSettings* ZoomSettings = nullptr);
    ~AnimationFramesInputWidget();

    ZoomSettings* zoomSettings() const;
    void setZoomSettings(ZoomSettings* zoomSettings);

    void setResourceItem(AbstractResourceItem* item);

private:
    void updateFrameLabel();
    void clearGui();

private slots:
    void onMtTilesetDataChanged();

    void onAnimationFrameAdvance();
    void onPreviousClicked();
    void onNextClicked();

private:
    std::unique_ptr<Ui::AnimationFramesInputWidget> const _ui;
    QGraphicsScene* const _graphicsScene;

    AbstractResourceItem* _tileset;
    AnimationFramesInputGraphicsItem* _graphicsItem;

    Resources::AnimationTimer _animationTimer;
};

class AnimationFramesInputGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    static const QColor GRID_COLOR;
    static const QColor ERROR_COLOR;

public:
    AnimationFramesInputGraphicsItem(AbstractResourceItem* item);
    ~AnimationFramesInputGraphicsItem() = default;

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

    static const QString& toolTipForType(const RES::ErrorList::InvalidTileReason& reason);

private slots:
    void updateInvalidTiles();
    void loadPixmaps();

private:
    AbstractResourceItem* const _resourceItem;
    const unsigned _gridSize;

    QGraphicsItem* _commonErrors;
    QList<QGraphicsItem*> _frameErrors;

    QVector<QPixmap> _pixmaps;
    int _animationFrameIndex;
};
}
}
}
