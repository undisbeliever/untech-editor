/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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

struct SelectedItem {
    enum Type {
        NONE,
        FRAME_OBJECT,
        ACTION_POINT,
        ENTITY_HITBOX
    };
    Type type;
    unsigned index;

    bool operator==(const SelectedItem& o) const
    {
        return std::tie(type, index) == std::tie(o.type, o.index);
    }
    bool operator<(const SelectedItem& o) const
    {
        return std::tie(type, index) < std::tie(o.type, o.index);
    }
};

class Selection : public QObject {
    Q_OBJECT

public:
    explicit Selection(QObject* parent = nullptr);
    ~Selection() = default;

    void setDocument(Document* document);

    SI::Frame* selectedFrame() const { return _selectedFrame; }
    const idstring& selectedFrameId() const { return _selectedFrameId; }

    const std::set<SelectedItem>& selectedItems() const { return _selectedItems; }
    void setSelectedItems(const std::set<SelectedItem>&);

    void unselectFrame();
    void selectFrame(const idstring& id);

signals:
    void selectedFrameChanged();
    void selectedItemsChanged();

private slots:
    void onFrameAboutToBeRemoved(const SI::Frame*);
    void onFrameRenamed(const SI::Frame*, const idstring& newId);

private:
    Document* _document;

    SI::Frame* _selectedFrame;
    idstring _selectedFrameId;
    std::set<SelectedItem> _selectedItems;
};
}
}
}
}
