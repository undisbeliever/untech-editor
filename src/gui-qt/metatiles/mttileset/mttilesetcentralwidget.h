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

namespace MetaTiles {
namespace Ui {
class MtTilesetCentralWidget;
}
class MtTilesetResourceItem;
class MtTilesetGraphicsItem;

namespace RES = UnTech::Resources;

class MtTilesetCentralWidget : public QWidget {
    Q_OBJECT

public:
    MtTilesetCentralWidget(QWidget* parent,
                           ZoomSettings* ZoomSettings);
    ~MtTilesetCentralWidget();

    ZoomSettings* zoomSettings() const;

    void setResourceItem(MtTilesetResourceItem* item);

private:
    void updateFrameLabel();
    void clearGui();

private slots:
    void onMtTilesetDataChanged();

    void onAnimationFrameAdvance();
    void onPreviousClicked();
    void onNextClicked();

private:
    std::unique_ptr<Ui::MtTilesetCentralWidget> const _ui;
    QGraphicsScene* const _graphicsScene;

    MtTilesetResourceItem* _tileset;
    MtTilesetGraphicsItem* _graphicsItem;

    Resources::AnimationTimer _animationTimer;
};

class MtTilesetGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    static const QColor GRID_COLOR;
    static const QColor ERROR_COLOR;

public:
    MtTilesetGraphicsItem(MtTilesetResourceItem* item);
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

    static const QString& toolTipForType(const RES::ErrorList::InvalidTileReason& reason);

private slots:
    void updateInvalidTiles();
    void loadPixmaps();

private:
    MtTilesetResourceItem* const _tileset;

    QGraphicsItem* _commonErrors;
    QList<QGraphicsItem*> _frameErrors;

    QVector<QPixmap> _pixmaps;
    int _animationFrameIndex;
};
}
}
}
