/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "models/common/idstring.h"
#include "models/metasprite/animation/animation.h"
#include "models/metasprite/framesetfile.h"
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
    virtual MSA::Animation::map_t* animations() const = 0;

    virtual QStringList frameNames() const = 0;
    virtual unsigned nPalettes() const = 0;

    Animation::AnimationFramesList* animationFramesList() const { return _animationFramesList; }
    Animation::AnimationsMap* animationsMap() const { return _animationsMap; }

protected:
    inline const auto& frameSetList() const
    {
        return _frameSetFiles;
    }

    inline auto& frameSetFile()
    {
        return _frameSetFiles.at(index());
    }

    void compileMsFrameset(const MS::FrameSet* frameSet, ErrorList& errList);

signals:
    void frameSetDataChanged();
    void frameSetExportOrderChanged();

private:
    std::vector<UnTech::MetaSprite::FrameSetFile>& _frameSetFiles;

    Animation::AnimationsMap* const _animationsMap;
    Animation::AnimationFramesList* const _animationFramesList;
};
}
}
}
