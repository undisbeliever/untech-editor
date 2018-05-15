/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metaspriteproject.h"
#include "gui-qt/abstractresourceitem.h"
#include "models/common/idstring.h"
#include "models/metasprite/animation/animation.h"
#include <QObject>
#include <QUndoStack>
#include <set>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class FrameSetResourceList;

namespace Animation {
class AnimationFramesList;
class AnimationsMap;
}

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace MSA = UnTech::MetaSprite::Animation;

class AbstractMsDocument : public AbstractExternalResourceItem {
    Q_OBJECT

public:
    explicit AbstractMsDocument(FrameSetResourceList* parent, size_t index);
    ~AbstractMsDocument() = default;

public:
    MetaSpriteProject* project() const { return static_cast<MetaSpriteProject*>(_project); }

    virtual MSA::Animation::map_t* animations() const = 0;

    virtual QStringList frameNames() const = 0;

    Animation::AnimationFramesList* animationFramesList() const { return _animationFramesList; }
    Animation::AnimationsMap* animationsMap() const { return _animationsMap; }

protected:
    inline const auto& frameSetList() const
    {
        return project()->metaSpriteProject()->frameSets;
    }

    inline auto& frameSetFile() const
    {
        return project()->metaSpriteProject()->frameSets.at(index());
    }

    void compileMsFrameset(const MS::FrameSet* frameSet,
                           UnTech::MetaSprite::ErrorList& errList);

    void appendToErrorList(RES::ErrorList& errList,
                           const UnTech::MetaSprite::ErrorList& msErrorList);

signals:
    void frameSetDataChanged();
    void frameSetNameChanged();
    void frameSetExportOrderChanged();

private:
    Animation::AnimationsMap* const _animationsMap;
    Animation::AnimationFramesList* const _animationFramesList;
};
}
}
}
