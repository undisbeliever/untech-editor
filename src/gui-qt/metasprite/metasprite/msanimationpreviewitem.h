/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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
class Document;
class TilesetPixmaps;

namespace MS = UnTech::MetaSprite::MetaSprite;

class MsAnimationPreviewItem : public Animation::AnimationPreviewItem {
    Q_OBJECT

public:
    explicit MsAnimationPreviewItem(LayerSettings* layerSettings,
                                    Style* style,
                                    TilesetPixmaps* _tilesetPixmaps,
                                    const Document* document);
    ~MsAnimationPreviewItem() = default;

protected:
    virtual const void* setFrame(const idstring& frameName) final;
    virtual void drawFrame(QPainter* painter) final;

private:
    LayerSettings* _layerSettings;
    Style* _style;
    TilesetPixmaps* _tilesetPixmaps;
    const Document* _document;

    const MS::Frame* _frame;
};

class MsAnimationPreviewItemFactory : public QObject,
                                      public Animation::AnimationPreviewItemFactory {
    Q_OBJECT

public:
    MsAnimationPreviewItemFactory(LayerSettings* layerSettings,
                                  TilesetPixmaps* tilesetPixmaps,
                                  QWidget* parent = nullptr);
    ~MsAnimationPreviewItemFactory() = default;

    virtual MsAnimationPreviewItem* createPreviewItem(const AbstractDocument* document);

private:
    LayerSettings* _layerSettings;
    TilesetPixmaps* _tilesetPixmaps;
    Style* _style;
};
}
}
}
}
