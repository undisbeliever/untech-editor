/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "nullframesetresourceitem.h"
#include "framesetresourcelist.h"

using FrameSetType = UnTech::MetaSprite::Project::FrameSetType;
using namespace UnTech::GuiQt::MetaSprite;

NullFrameSetResourceItem::NullFrameSetResourceItem(FrameSetResourceList* parent, size_t index)
    : AbstractInternalResourceItem(parent, index)
{
    Q_ASSERT(index < project()->metaSpriteProject()->frameSets.size());

    const auto& fs = project()->metaSpriteProject()->frameSets.at(index);
    Q_ASSERT(fs.type != FrameSetType::METASPRITE && fs.type != FrameSetType::SPRITE_IMPORTER);

    setName(tr("(null FrameSet)"));
}

bool NullFrameSetResourceItem::compileResource(RES::ErrorList&)
{
    return true;
}