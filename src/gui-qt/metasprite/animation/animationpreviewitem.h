/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/animation/previewstate.h"
#include <QGraphicsItem>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractDocument;

namespace Animation {

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationPreviewItem : public QGraphicsItem {
    using NameReference = UnTech::MetaSprite::NameReference;
    using Region = MSA::PreviewState::Region;

public:
    explicit AnimationPreviewItem(const MSA::Animation::map_t* map,
                                  QGraphicsItem* parent = nullptr);
    ~AnimationPreviewItem() = default;

    const idstring& animationId() const { return _animationId; }
    const MSA::PreviewState& state() const { return _state; }
    const NameReference& frame() const { return _frame; }

    void setAnimation(const idstring& animationId);
    void setVelocityFp(const point& p) { _state.setVelocityFp(p); }
    void setRegion(const Region& r) { _state.setRegion(r); }

    void processDisplayFrame();
    void nextAnimationFrame();
    void resetAnimation();

    void sync();

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

    virtual QVariant itemChange(GraphicsItemChange change,
                                const QVariant& value) override;

private:
    point wrapPosition(const point& pos) const;

private:
    idstring _animationId;
    MSA::PreviewState _state;
    NameReference _frame;
};
}
}
}
}
