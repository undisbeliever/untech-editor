/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractmsdocument.h"
#include "framesetresourcelist.h"
#include "animation/animationaccessors.h"
#include "animation/animationframesmanager.h"
#include "models/metasprite/compiler/compiler.h"

using namespace UnTech::GuiQt::MetaSprite;

AbstractMsDocument::AbstractMsDocument(FrameSetResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _animationsMap(new Animation::AnimationsMap(this))
    , _animationFramesList(new Animation::AnimationFramesList(this))
{
    connect(this, &AbstractMsDocument::frameSetDataChanged,
            this, &AbstractResourceItem::dataChanged);

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

void AbstractMsDocument::compileMsFrameset(const MS::FrameSet* frameSet,
                                           UnTech::MetaSprite::ErrorList& errList)
{
    using namespace UnTech::MetaSprite::Compiler;

    if (frameSet) {
        try {
            const auto project = this->project()->metaSpriteProject();
            Q_ASSERT(project);

            const auto* exportOrder = project->exportOrders.find(frameSet->exportOrder);
            validateFrameSetAndBuildTilesets(*frameSet, exportOrder, errList);
        }
        catch (std::exception& ex) {
            errList.addError(*frameSet, ex.what());
        }
    }
}

void AbstractMsDocument::appendToErrorList(RES::ErrorList& errList,
                                           const UnTech::MetaSprite::ErrorList& msErrorList)
{
    auto appendToErr = [&](const auto& list) {
        for (auto& e : list) {
            std::stringstream ss;
            ss << e;
            errList.addError(ss.str());
        }
    };

    appendToErr(msErrorList.errors);
    appendToErr(msErrorList.warnings);
}
