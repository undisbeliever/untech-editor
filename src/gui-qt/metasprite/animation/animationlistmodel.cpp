/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationlistmodel.h"
#include "gui-qt/metasprite/abstractmsdocument.h"

using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationListModel::AnimationListModel(QObject* parent)
    : AbstractIdmapListModel(parent)
    , _document(nullptr)
{
}

void AnimationListModel::setDocument(AbstractMsDocument* document)
{
    Q_ASSERT(document != nullptr);

    if (_document) {
        _document->disconnect(this);
    }
    _document = document;

    buildLists(*_document->animations());
}

void AnimationListModel::insertAnimation(
    const idstring& id, std::unique_ptr<MSA::Animation> animation)
{
    auto& animations = *_document->animations();
    const MSA::Animation* animationPtr = animation.get();

    insertMapItem(animations, id, std::move(animation));

    emit _document->animationAdded(animationPtr);
    emit _document->animationMapChanged();
}

std::unique_ptr<MSA::Animation> AnimationListModel::removeAnimation(const idstring& id)
{
    auto& animations = *_document->animations();

    emit _document->animationAboutToBeRemoved(animations.getPtr(id));
    auto ret = removeMapItem(animations, id);
    emit _document->animationMapChanged();

    return ret;
}

void AnimationListModel::renameAnimation(const idstring& oldId, const idstring& newId)
{
    auto& animations = *_document->animations();

    renameMapItem(animations, oldId, newId);

    emit _document->animationRenamed(animations.getPtr(newId), newId);
    emit _document->animationMapChanged();
}
