/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/animation/animation.h"
#include <QAbstractItemModel>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractDocument;

namespace Animation {

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationFramesModel : public QAbstractItemModel {
    Q_OBJECT

    const static QString FLIP_STRINGS[4];

public:
    enum class Column {
        FRAME,
        FLIP,
        DURATION,
    };
    constexpr static int N_COLUMNS = 3;

public:
    explicit AnimationFramesModel(QObject* parent = nullptr);
    ~AnimationFramesModel() = default;

    void setDocument(AbstractDocument* document);

    QModelIndex toModelIndex(int row) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const final;

    virtual QVariant data(const QModelIndex& index, int role) const final;

private:
    MSA::AnimationFrame* toAnimationFrame(const QModelIndex& index) const;

private slots:
    void onSelectedAnimationChanged();
    void onAnimationDataChanged(const void* animation);

private:
    AbstractDocument* _document;
    MSA::Animation* _animation;
};
}
}
}
}
