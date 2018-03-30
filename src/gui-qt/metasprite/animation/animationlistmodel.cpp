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
    if (_document) {
        _document->disconnect(this);
    }
    _document = document;

    if (_document) {
        buildLists(*_document->animations());

        connect(_document, &AbstractMsDocument::animationAdded,
                this, &AnimationListModel::addIdstring);
        connect(_document, &AbstractMsDocument::animationAboutToBeRemoved,
                this, &AnimationListModel::removeIdstring);
        connect(_document, &AbstractMsDocument::animationRenamed,
                this, &AnimationListModel::renameIdstring);
    }
    else {
        clear();
    }
}
