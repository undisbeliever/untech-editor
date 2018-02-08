/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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
class Document;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class SiAnimationPreviewItem : public Animation::AnimationPreviewItem {
    Q_OBJECT

    static constexpr unsigned IMAGE_SIZE = 256;

public:
    explicit SiAnimationPreviewItem(LayerSettings* layerSettings, Style* style,
                                    const Document* document);
    ~SiAnimationPreviewItem() = default;

protected:
    virtual const void* setFrame(const idstring& frameName) final;
    virtual void drawFrame(QPainter* painter) final;

private slots:
    void onFrameObjectsChanged(const void* framePtr);

private:
    void drawFrameObjects();

private:
    LayerSettings* _layerSettings;
    Style* _style;
    const Document* _document;

    const SI::Frame* _frame;
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

    virtual SiAnimationPreviewItem* createPreviewItem(const AbstractMsDocument* document);

private:
    LayerSettings* _layerSettings;
    Style* _style;
};
}
}
}
}
