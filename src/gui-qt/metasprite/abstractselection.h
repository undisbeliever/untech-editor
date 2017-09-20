/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include "models/metasprite/animation/animation.h"
#include <QObject>
#include <QVector>
#include <set>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractMsDocument;

namespace MSA = UnTech::MetaSprite::Animation;

struct SelectedItem {
    enum Type {
        NONE,
        FRAME_OBJECT,
        ACTION_POINT,
        ENTITY_HITBOX,
        TILE_HITBOX
    };
    Type type;
    size_t index;

    bool operator==(const SelectedItem& o) const
    {
        return std::tie(type, index) == std::tie(o.type, o.index);
    }
    bool operator<(const SelectedItem& o) const
    {
        return std::tie(type, index) < std::tie(o.type, o.index);
    }
};

std::set<SelectedItem> moveSelectedItems(
    const std::set<SelectedItem>& items, int offset);

class AbstractSelection : public QObject {
    Q_OBJECT

public:
    explicit AbstractSelection(QObject* parent = nullptr);
    ~AbstractSelection() = default;

    void setDocument(AbstractMsDocument* document);

    bool hasSelectedFrame() const { return _selectedFramePtr != nullptr; }
    const idstring& selectedFrameId() const { return _selectedFrameId; }

    MSA::Animation* selectedAnimation() const { return _selectedAnimation; }
    const idstring& selectedAnimationId() const { return _selectedAnimationId; }
    int selectedAnimationFrame() const { return _selectedAnimationFrame; }

    const std::set<SelectedItem>& selectedItems() const { return _selectedItems; }
    void setSelectedItems(const std::set<SelectedItem>&);

    void selectFrame(const idstring& id);
    void unselectFrame();

    void selectAnimation(const idstring& id);
    void unselectAnimation();
    void selectAnimationFrame(int index);
    void unselectAnimationFrame() { selectAnimationFrame(-1); }

    bool canCloneSelectedItems() const;
    bool canRaiseSelectedItems() const;
    bool canLowerSelectedItems() const;

    void selectFrameObject(unsigned index);
    void selectActionPoint(unsigned index);
    void selectEntityHitbox(unsigned index);

    bool isFrameObjectSelected() const;
    bool isActionPointSelected() const;
    bool isEntityHitboxSelected() const;

protected:
    virtual const void* setSelectedFrame(const idstring& id) = 0;
    virtual unsigned nObjectsInSelectedFrame() const = 0;
    virtual unsigned nActionPointsInSelectedFrame() const = 0;
    virtual unsigned nEntityHitboxesInSelectedFrame() const = 0;

signals:
    void selectedFrameChanged();
    void selectedItemsChanged();

    void selectedAnimationChanged();
    void selectedAnimationFrameChanged();

private slots:
    void onFrameAboutToBeRemoved(const void* frame);
    void onFrameRenamed(const void* frame, const idstring& newId);

    void onFrameObjectAboutToBeRemoved(const void* frame, unsigned index);
    void onActionPointAboutToBeRemoved(const void* frame, unsigned index);
    void onEntityHitboxAboutToBeRemoved(const void* frame, unsigned index);

    void onFrameContentsMoved(const void* frame,
                              const std::set<SelectedItem>& oldPositions, int offset);

    void onAnimationAboutToBeRemoved(const void* animation);
    void onAnimationRenamed(const void* animation, const idstring& newId);

    void onAnimationFrameAboutToBeRemoved(const void* animation, unsigned index);
    void onAnimationFrameMoved(const void* animation, unsigned oldPos, unsigned newPos);

protected:
    AbstractMsDocument* _document;
    const void* _selectedFramePtr;
    idstring _selectedFrameId;
    std::set<SelectedItem> _selectedItems;

    MSA::Animation* _selectedAnimation;
    idstring _selectedAnimationId;
    int _selectedAnimationFrame;
};
}
}
}

Q_DECLARE_METATYPE(UnTech::GuiQt::MetaSprite::SelectedItem);
