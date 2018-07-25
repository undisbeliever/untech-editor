/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include <QBasicTimer>
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QWidget>
#include <functional>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;

namespace MetaSprite {
class AbstractMsDocument;

namespace Animation {
namespace Ui {
class AnimationPreview;
}
class AnimationDock;
class AnimationListModel;
class AnimationPreviewItem;
class AnimationPreviewItemFactory;

class AnimationPreview : public QWidget {
    Q_OBJECT

public:
    explicit AnimationPreview(AnimationDock* animationDock, QWidget* parent = nullptr);
    ~AnimationPreview();

    ZoomSettings* zoomSettings() const;

    void setItemFactory(AnimationPreviewItemFactory* itemFactory);
    void setZoomSettings(ZoomSettings* zoomSettings);
    void setDocument(AbstractMsDocument* document);

private:
    void removePreviewItem();
    void createPreviewItem();

public slots:
    void clearGui();
    void updateGui();
    void updateSceneRect();

    void onSelectedAnimationChanged();

    void onAnimationFramesChanged();

    void onAnimationComboActivated();
    void onVelocityChanged();
    void onRegionChanged();

    void onZeroXVelocityClicked();
    void onZeroYVelocityClicked();

    void onStepClicked();
    void onSkipClicked();
    void onResetClicked();

    void updateTimer();
    void stopTimer();

public:
    virtual void timerEvent(QTimerEvent* event) override;

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;

private:
    std::unique_ptr<Ui::AnimationPreview> const _ui;
    Accessor::IdmapListModel* const _animationListModel;
    QGraphicsScene* const _graphicsScene;

    AnimationPreviewItemFactory* _itemFactory;
    ZoomSettings* _zoomSettings;

    AbstractMsDocument* _document;

    AnimationPreviewItem* _previewItem;

    QBasicTimer _timer;
    QElapsedTimer _elapsed;
    qint64 _nsSinceLastFrame;
};
}
}
}
}
