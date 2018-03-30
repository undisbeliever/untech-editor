/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractmsdocument.h"
#include "framesetresourcelist.h"
#include "animation/animationframesmanager.h"
#include "animation/animationlistmodel.h"

using namespace UnTech::GuiQt::MetaSprite;

AbstractMsDocument::AbstractMsDocument(FrameSetResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
{
}

QStringList AbstractMsDocument::animationList() const
{
    QStringList al;

    if (const auto* aniMap = this->animations()) {
        al.reserve(aniMap->size());
        for (const auto& it : *aniMap) {
            al.append(QString::fromStdString(it.first));
        }
    }

    return al;
}
