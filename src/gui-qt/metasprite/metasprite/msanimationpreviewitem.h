/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metasprite/animation/animationpreviewitem.h"
#include "models/metasprite/metasprite.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class Style;
class LayerSettings;

namespace MetaSprite {
class ResourceItem;
class TilesetPixmaps;

namespace MS = UnTech::MetaSprite::MetaSprite;

class MsAnimationPreviewItem : public Animation::AnimationPreviewItem {
    Q_OBJECT

public:
    explicit MsAnimationPreviewItem(LayerSettings* layerSettings,
                                    Style* style,
                                    TilesetPixmaps* _tilesetPixmaps,
                                    const ResourceItem* resourceItem);
    ~MsAnimationPreviewItem() = default;

protected:
    virtual size_t getFrameIndex(const idstring& frameName) final;
    virtual void drawFrame(QPainter* painter) final;

private:
    LayerSettings* const _layerSettings;
    Style* const _style;
    TilesetPixmaps* const _tilesetPixmaps;
    const ResourceItem* const _resourceItem;
};

class MsAnimationPreviewItemFactory : public QObject,
                                      public Animation::AnimationPreviewItemFactory {
    Q_OBJECT

public:
    MsAnimationPreviewItemFactory(LayerSettings* layerSettings,
                                  TilesetPixmaps* tilesetPixmaps,
                                  QWidget* parent = nullptr);
    ~MsAnimationPreviewItemFactory() = default;

    virtual MsAnimationPreviewItem* createPreviewItem(const AbstractMsResourceItem* abstractItem);

private:
    LayerSettings* const _layerSettings;
    TilesetPixmaps* const _tilesetPixmaps;
    Style* const _style;
};
}
}
}
}
