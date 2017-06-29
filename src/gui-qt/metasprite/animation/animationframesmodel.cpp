/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationframesmodel.h"
#include "gui-qt/metasprite/abstractdocument.h"
#include "gui-qt/metasprite/abstractselection.h"

using namespace UnTech::GuiQt::MetaSprite::Animation;

const QString AnimationFramesModel::FLIP_STRINGS[4] = {
    QString(),
    QString::fromUtf8("hFlip"),
    QString::fromUtf8("vFlip"),
    QString::fromUtf8("hvFlip")
};

AnimationFramesModel::AnimationFramesModel(QObject* parent)
    : QAbstractItemModel(parent)
    , _document(nullptr)
    , _animation(nullptr)
{
}

void AnimationFramesModel::setDocument(AbstractDocument* document)
{
    Q_ASSERT(document != nullptr);

    if (_document) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    onSelectedAnimationChanged();
    connect(_document->selection(), &AbstractSelection::selectedAnimationChanged,
            this, &AnimationFramesModel::onSelectedAnimationChanged);

    connect(_document, &AbstractDocument::animationDataChanged,
            this, &AnimationFramesModel::onAnimationDataChanged);
}

void AnimationFramesModel::onSelectedAnimationChanged()
{
    MSA::Animation* animation = _document->selection()->selectedAnimation();

    if (_animation != animation) {
        beginResetModel();
        _animation = animation;
        endResetModel();
    }
}

void AnimationFramesModel::onAnimationDataChanged(const void* animation)
{
    if (animation == _animation) {
        if (_animation && _animation->frames.size() > 0) {
            int nRows = _animation->frames.size();

            emit dataChanged(createIndex(0, int(Column::DURATION)),
                             createIndex(nRows, int(Column::DURATION)),
                             { Qt::DisplayRole });
        }
    }
}

QModelIndex AnimationFramesModel::toModelIndex(int row) const
{
    if (_animation == nullptr
        || row < 0 || (unsigned)row >= _animation->frames.size()) {

        return QModelIndex();
    }
    return createIndex(row, 0);
}

QModelIndex AnimationFramesModel::index(int row, int column, const QModelIndex& parent) const
{
    if (_animation == nullptr
        || parent.isValid()
        || row < 0 || (unsigned)row >= _animation->frames.size()
        || column < 0 || column >= N_COLUMNS) {

        return QModelIndex();
    }
    return createIndex(row, column);
}

QModelIndex AnimationFramesModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int AnimationFramesModel::columnCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return N_COLUMNS;
    }
    return 0;
}

int AnimationFramesModel::rowCount(const QModelIndex& parent) const
{
    if (_animation && !parent.isValid()) {
        return _animation->frames.size();
    }
    return 0;
}

Qt::ItemFlags AnimationFramesModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()
        || index.column() >= N_COLUMNS) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant AnimationFramesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractItemModel::headerData(section, orientation, role);
    }

    switch ((Column)section) {
    case Column::FRAME:
        return tr("Frame");

    case Column::FLIP:
        return tr("Flip");

    case Column::DURATION:
        return tr("Duration");
    }
    return QVariant();
}

inline MSA::AnimationFrame* AnimationFramesModel::toAnimationFrame(const QModelIndex& index) const
{
    if (_animation == nullptr
        || !index.isValid()
        || index.row() < 0 || (unsigned)index.row() >= _animation->frames.size()) {
        return nullptr;
    }
    return &_animation->frames.at(index.row());
}

QVariant AnimationFramesModel::data(const QModelIndex& index, int role) const
{
    if (const auto* aFrame = toAnimationFrame(index)) {
        unsigned flipIndex = (aFrame->frame.vFlip << 1) | aFrame->frame.hFlip;

        if (role == Qt::DisplayRole) {
            switch ((Column)index.column()) {
            case Column::FRAME:
                return QString::fromStdString(aFrame->frame.name);

            case Column::FLIP:
                return FLIP_STRINGS[flipIndex];

            case Column::DURATION:
                return QString::fromStdString(
                    _animation->durationFormat.durationToString(aFrame->duration));
            }
        }
    }

    return QVariant();
}
