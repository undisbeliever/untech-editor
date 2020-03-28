/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/listaccessortablemanager.h"
#include "gui-qt/common/properties/propertylistmanager.h"
#include "models/metasprite/animation/animation.h"
#include <QStringList>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractMsResourceItem;

namespace Animation {
class AnimationsList;
class AnimationFramesList;

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationManager : public PropertyListManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        DURATION_FORMAT,
        ONE_SHOT,
        NEXT_ANIMATION,
    };

public:
    explicit AnimationManager(QObject* parent = nullptr);
    ~AnimationManager() = default;

    void setResourceItem(AbstractMsResourceItem* entityHitboxList);

    virtual QVariant data(int id) const final;
    virtual void updateParameters(int id, QVariant& param1, QVariant& param2) const final;
    virtual bool setData(int id, const QVariant& value) final;

private slots:
    void onSelectedAnimationChanged();
    void onFrameDataChanged(size_t animationIndex);

private:
    AnimationsList* _animationsList;
};

class AnimationFramesManager : public Accessor::ListAccessorTableManager {
    Q_OBJECT

public:
    const static QStringList FLIP_STRINGS;

    enum PropertyId {
        FRAME,
        FLIP,
        DURATION,
        DURATION_STRING = -2
    };

private:
    AbstractMsResourceItem* _resourceItem;

public:
    explicit AnimationFramesManager(QObject* parent = nullptr);
    ~AnimationFramesManager() = default;

    void setResourceItem(AbstractMsResourceItem* resourceItem);

    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const final;

    virtual QVariant data(int index, int id) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

private:
    const MSA::Animation* selectedAnimation() const;
};
}
}
}
}
