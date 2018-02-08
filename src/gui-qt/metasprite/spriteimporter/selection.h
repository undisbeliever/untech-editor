/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/metasprite/abstractselection.h"
#include "models/metasprite/spriteimporter.h"
#include <QObject>
#include <QVector>
#include <set>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class Selection : public AbstractSelection {
    Q_OBJECT

public:
    explicit Selection(QObject* parent = nullptr);
    ~Selection() = default;

    void setDocument(Document* document);

    void selectFrame(const SI::Frame* frame);

    inline void selectFrame(const idstring& frameId)
    {
        return AbstractSelection::selectFrame(frameId);
    }

    SI::Frame* selectedFrame() const { return _selectedFrame; }

protected:
    virtual const void* setSelectedFrame(const idstring& id) final;
    virtual unsigned nObjectsInSelectedFrame() const final;
    virtual unsigned nActionPointsInSelectedFrame() const final;
    virtual unsigned nEntityHitboxesInSelectedFrame() const final;

private:
    Document* _document;

    SI::Frame* _selectedFrame;
};
}
}
}
}
