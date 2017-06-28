/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationlistmodel.h"
#include "gui-qt/metasprite/abstractdocument.h"

using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationListModel::AnimationListModel(QObject* parent)
    : AbstractIdmapListModel(parent)
    , _document(nullptr)
{
}

void AnimationListModel::setDocument(AbstractDocument* document)
{
    Q_ASSERT(document != nullptr);

    if (_document) {
        _document->disconnect(this);
    }
    _document = document;

    buildLists(*_document->animations());
}
