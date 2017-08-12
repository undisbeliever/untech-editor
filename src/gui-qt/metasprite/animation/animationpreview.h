/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QBasicTimer>
#include <QGraphicsScene>
#include <QWidget>
#include <functional>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;

namespace MetaSprite {
class AbstractDocument;

namespace Animation {
namespace Ui {
class AnimationPreview;
}
class AnimationPreviewItem;
class AnimationPreviewItemFactory;

class AnimationPreview : public QWidget {
    Q_OBJECT

public:
    explicit AnimationPreview(QWidget* parent = nullptr);
    ~AnimationPreview();

    void setItemFactory(AnimationPreviewItemFactory* itemFactory);
    void setZoomSettings(ZoomSettings* zoomSettings);
    void setDocument(AbstractDocument* document);

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
    std::unique_ptr<Ui::AnimationPreview> _ui;
    QGraphicsScene* _graphicsScene;
    AnimationPreviewItemFactory* _itemFactory;
    ZoomSettings* _zoomSettings;

    AbstractDocument* _document;

    AnimationPreviewItem* _previewItem;
    QBasicTimer _timer;
};
}
}
}
}
