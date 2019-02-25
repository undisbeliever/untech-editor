/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertytablemanager.h"
#include "models/metasprite/animation/animation.h"
#include <QStringList>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractMsDocument;

namespace Animation {

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationFramesManager : public PropertyTableManager {
    Q_OBJECT

public:
    const static QStringList FLIP_STRINGS;

    enum PropertyId {
        FRAME,
        FLIP,
        DURATION,
        DURATION_STRING = -2
    };

public:
    explicit AnimationFramesManager(QObject* parent = nullptr);
    ~AnimationFramesManager() = default;

    void setDocument(AbstractMsDocument* document);

    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const final;

    virtual int rowCount() const final;
    virtual QVariant data(int index, int id) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

    virtual bool canInsertItem() final;
    virtual bool canCloneItem(int index) final;

    virtual bool insertItem(int index) final;
    virtual bool cloneItem(int index) final;
    virtual bool removeItem(int index) final;
    virtual bool moveItem(int from, int to) final;

private slots:
    void onSelectedAnimationChanged();
    void onAnimationDataChanged(size_t animationIndex);
    void onAnimationFrameChanged(size_t animationIndex, unsigned index);
    void onAnimationFrameAdded(size_t animationIndex, unsigned index);
    void onAnimationFrameAboutToBeRemoved(size_t animationIndex, unsigned index);
    void onAnimationFrameMoved(size_t animationIndex, unsigned oldPos, unsigned newPos);

private:
    const MSA::Animation* selectedAnimation() const;

private:
    AbstractMsDocument* _document;
};
}
}
}
}
