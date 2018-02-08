/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/animation/animation.h"
#include <QAbstractItemModel>
#include <QStringList>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractMsDocument;

namespace Animation {
class ChangeAnimationFrame;
class AddRemoveAnimationFrame;
class RaiseAnimationFrame;
class LowerAnimationFrame;

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationFramesModel : public QAbstractItemModel {
    Q_OBJECT

public:
    const static QStringList FLIP_STRINGS;

    enum class Column {
        FRAME,
        FLIP,
        DURATION,
    };
    constexpr static int N_COLUMNS = 3;

public:
    explicit AnimationFramesModel(QObject* parent = nullptr);
    ~AnimationFramesModel() = default;

    void setDocument(AbstractMsDocument* document);

    QModelIndex toModelIndex(int row) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const final;

    virtual QVariant data(const QModelIndex& index, int role) const final;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) final;

protected:
    friend class ChangeAnimationFrame;
    void setAnimationFrame(MSA::Animation* animation, unsigned index, const MSA::AnimationFrame& value);

    friend class AddRemoveAnimationFrame;
    void insertAnimationFrame(MSA::Animation* animation, unsigned index, const MSA::AnimationFrame& value);
    void removeAnimationFrame(MSA::Animation* animation, unsigned index);

    friend class RaiseAnimationFrame;
    void raiseAnimationFrame(MSA::Animation* animation, unsigned index);

    friend class LowerAnimationFrame;
    void lowerAnimationFrame(MSA::Animation* animation, unsigned index);

private:
    MSA::AnimationFrame* toAnimationFrame(const QModelIndex& index) const;

private slots:
    void onSelectedAnimationChanged();
    void onAnimationDataChanged(const void* animation);

private:
    AbstractMsDocument* _document;
    MSA::Animation* _animation;
};
}
}
}
}
