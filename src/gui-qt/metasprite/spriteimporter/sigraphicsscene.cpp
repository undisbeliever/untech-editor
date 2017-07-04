/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "sigraphicsscene.h"
#include "document.h"

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

SiGraphicsScene::SiGraphicsScene(QWidget* parent)
    : QGraphicsScene(parent)
    , _document(nullptr)
{
    _frameSetPixmap = new QGraphicsPixmapItem();
    _frameSetPixmap->setTransformationMode(Qt::FastTransformation);
    addItem(_frameSetPixmap);
}

void SiGraphicsScene::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
    }
    _document = document;

    if (_document) {
        updateFrameSetPixmap();

        connect(_document, &Document::frameSetImageChanged,
                this, &SiGraphicsScene::updateFrameSetPixmap);
    }
    else {
        _frameSetPixmap->setPixmap(QPixmap());
    }
}

void SiGraphicsScene::updateFrameSetPixmap()
{
    if (_document && _document->frameSet()->isImageValid()) {
        const std::string& fn = _document->frameSet()->imageFilename;

        QPixmap p(QString::fromStdString(fn), "PNG");
        _frameSetPixmap->setPixmap(p);
    }
    else {
        _frameSetPixmap->setPixmap(QPixmap());
    }

    setSceneRect(itemsBoundingRect());
}
