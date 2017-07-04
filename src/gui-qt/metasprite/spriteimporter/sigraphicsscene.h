/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QWidget>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;

class SiGraphicsScene : public QGraphicsScene {
    Q_OBJECT

public:
    SiGraphicsScene(QWidget* parent = nullptr);
    ~SiGraphicsScene() = default;

    void setDocument(Document* document);

public slots:
    void updateFrameSetPixmap();

private:
    Document* _document;

    QGraphicsPixmapItem* _frameSetPixmap;
};
}
}
}
}
