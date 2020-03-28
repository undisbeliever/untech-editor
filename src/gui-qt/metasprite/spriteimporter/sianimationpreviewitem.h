/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metasprite/animation/animationpreviewitem.h"
#include "models/metasprite/spriteimporter.h"

#include <QImage>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class Style;
class LayerSettings;

namespace SpriteImporter {
class ResourceItem;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class SiAnimationPreviewItem : public Animation::AnimationPreviewItem {
    Q_OBJECT

    static constexpr unsigned IMAGE_SIZE = 256;

public:
    explicit SiAnimationPreviewItem(LayerSettings* layerSettings, Style* style,
                                    const ResourceItem* resourceItem);
    ~SiAnimationPreviewItem() = default;

protected:
    virtual size_t getFrameIndex(const idstring& frameName) final;
    virtual void drawFrame(QPainter* painter) final;

private slots:
    void onFrameObjectsChanged(size_t frameIndex);

private:
    void drawFrameObjects(const SI::Frame& frame);

private:
    LayerSettings* const _layerSettings;
    Style* const _style;

    const ResourceItem* _resourceItem;

    QImage _frameObjects;
    bool _frameObjectsDirty;
};

class SiAnimationPreviewItemFactory : public QObject,
                                      public Animation::AnimationPreviewItemFactory {
    Q_OBJECT

public:
    SiAnimationPreviewItemFactory(LayerSettings* layerSettings,
                                  QWidget* parent = nullptr);
    ~SiAnimationPreviewItemFactory() = default;

    virtual SiAnimationPreviewItem* createPreviewItem(const AbstractMsResourceItem* abstractResourceItem);

private:
    LayerSettings* const _layerSettings;
    Style* const _style;
};
}
}
}
}
