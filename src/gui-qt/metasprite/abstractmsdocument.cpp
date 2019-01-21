/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractmsdocument.h"
#include "framesetresourcelist.h"
#include "animation/animationaccessors.h"
#include "animation/animationframesmanager.h"
#include "gui-qt/project.h"
#include "models/metasprite/compiler/compiler.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::MetaSprite;

AbstractMsDocument::AbstractMsDocument(FrameSetResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _frameSetFiles(parent->frameSetFiles())
    , _animationsMap(new Animation::AnimationsMap(this))
    , _animationFramesList(new Animation::AnimationFramesList(this))
{
    connect(this, &AbstractMsDocument::frameSetDataChanged,
            this, &AbstractResourceItem::dataChanged);

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);
}

void AbstractMsDocument::compileMsFrameset(const MS::FrameSet* frameSet, ErrorList& errList)
{
    using namespace UnTech::MetaSprite::Compiler;

    if (frameSet) {
        try {
            const auto project = this->project()->projectFile();
            Q_ASSERT(project);

            const auto* exportOrder = project->frameSetExportOrders.find(frameSet->exportOrder);
            validateFrameSetAndBuildTilesets(*frameSet, exportOrder, errList);
        }
        catch (std::exception& ex) {
            errList.addError(ex.what());
        }
    }
}
