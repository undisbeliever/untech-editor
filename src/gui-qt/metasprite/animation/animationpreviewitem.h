/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/animation/previewstate.h"
#include <QGraphicsObject>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractMsDocument;

namespace Animation {

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationPreviewItem : public QGraphicsObject {
    Q_OBJECT

    using NameReference = UnTech::MetaSprite::NameReference;
    using Region = MSA::PreviewState::Region;

public:
    explicit AnimationPreviewItem(const AbstractMsDocument* document,
                                  QGraphicsItem* parent = nullptr);
    ~AnimationPreviewItem() = default;

    const MSA::PreviewState& state() const { return _state; }
    size_t frameIndex() const { return _frameIndex; }

    void setAnimationIndex(size_t animationIndex);
    void setVelocityFp(const point& p) { _state.setVelocityFp(p); }
    void setRegion(const Region& r) { _state.setRegion(r); }

    void processDisplayFrame();
    void nextAnimationFrame();
    void resetAnimation();

    void sync();

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

    virtual QVariant itemChange(GraphicsItemChange change,
                                const QVariant& value) final;

protected slots:
    // Subclasses are responsible for connecting the
    // document changed signals to these slots.

    void onFrameAdded();
    void onFrameAboutToBeRemoved(size_t frameIndex);
    void onFrameDataAndContentsChanged(size_t frameIndex);

private:
    point wrapPosition(const point& pos) const;

protected:
    // Called on every `sync()`
    // Returns the index of the frame
    // Returns INT_MAX or greater if the frame does not exist
    virtual size_t getFrameIndex(const idstring& frameName) = 0;

    // Draws the frame onto the painter.
    // Will only be called on valid frames.
    // The QPainter is already flipped to match PreviewState.
    virtual void drawFrame(QPainter* painter) = 0;

private:
    const AbstractMsDocument* const _document;

    size_t _animationIndex;
    MSA::PreviewState _state;

    NameReference _prevFrame;

    size_t _frameIndex;
};

class AnimationPreviewItemFactory {
public:
    virtual AnimationPreviewItem* createPreviewItem(const AbstractMsDocument*) = 0;
};
}
}
}
}
