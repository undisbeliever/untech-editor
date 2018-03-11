/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/abstractdocument.h"
#include "models/common/idstring.h"
#include "models/metasprite/animation/animation.h"
#include <QObject>
#include <QUndoStack>
#include <set>

namespace UnTech {
namespace GuiQt {
class AbstractIdmapListModel;

namespace MetaSprite {
namespace Animation {
class AnimationListModel;
class AnimationFramesModel;
class AnimationFramesManager;
}
struct SelectedItem;
class AbstractSelection;

namespace MSA = UnTech::MetaSprite::Animation;

class AbstractMsDocument : public GuiQt::AbstractDocument {
    Q_OBJECT

public:
    explicit AbstractMsDocument(QObject* parent = nullptr);
    ~AbstractMsDocument() = default;

protected:
    void initModels();

public:
    virtual MSA::Animation::map_t* animations() const = 0;

    virtual AbstractSelection* selection() const = 0;
    virtual AbstractIdmapListModel* frameListModel() const = 0;

    auto* animationListModel() const { return _animationListModel; }
    auto* animationFramesManager() const { return _animationFramesManager; }

signals:
    void frameSetDataChanged();

    void frameDataChanged(const void* frame);
    void frameTileHitboxChanged(const void* frame);
    void frameMapChanged();
    void frameAdded(const void* frame);
    void frameAboutToBeRemoved(const void* frame);
    void frameRenamed(const void* frame, const idstring& newId);

    void frameObjectChanged(const void* frame, unsigned index);
    void actionPointChanged(const void* frame, unsigned index);
    void entityHitboxChanged(const void* frame, unsigned index);

    void frameObjectListChanged(const void* frame);
    void actionPointListChanged(const void* frame);
    void entityHitboxListChanged(const void* frame);

    void frameObjectAboutToBeRemoved(const void* frame, unsigned index);
    void actionPointAboutToBeRemoved(const void* frame, unsigned index);
    void entityHitboxAboutToBeRemoved(const void* frame, unsigned index);

    void frameObjectAdded(const void* frame, unsigned index);
    void actionPointAdded(const void* frame, unsigned index);
    void entityHitboxAdded(const void* frame, unsigned index);

    void frameContentsMoved(const void* frame,
                            const std::set<SelectedItem>& oldPositions, int offset);

    void animationDataChanged(const void* animation);
    void animationMapChanged();
    void animationAdded(const void* animation);
    void animationAboutToBeRemoved(const void* animation);
    void animationRenamed(const void* animation, const idstring& newId);

    void animationFrameChanged(const void* animation, unsigned index);
    void animationFrameListChanged(const void* animation);
    void animationFrameAdded(const void* animation, unsigned index);
    void animationFrameAboutToBeRemoved(const void* animation, unsigned index);
    void animationFrameMoved(const void* animation, unsigned oldPos, unsigned newPos);

private:
    Animation::AnimationListModel* _animationListModel;
    Animation::AnimationFramesManager* _animationFramesManager;
};
}
}
}
