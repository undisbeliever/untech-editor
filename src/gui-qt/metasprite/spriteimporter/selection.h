/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/spriteimporter.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class Selection : public QObject {
    Q_OBJECT

public:
    explicit Selection(QObject* parent = nullptr);
    ~Selection() = default;

    void setDocument(Document* document);

    SI::Frame* selectedFrame() const { return _selectedFrame; }
    const idstring& selectedFrameId() const { return _selectedFrameId; }

    void unselectFrame();
    void selectFrame(const idstring& id);

signals:
    void selectedFrameChanged();

private slots:
    void onFrameAboutToBeRemoved(const SI::Frame*);

private:
    Document* _document;

    SI::Frame* _selectedFrame;
    idstring _selectedFrameId;
};
}
}
}
}
