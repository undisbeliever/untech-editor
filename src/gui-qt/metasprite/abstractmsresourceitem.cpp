/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractmsresourceitem.h"
#include "framesetresourcelist.h"
#include "animation/accessors.h"
#include "animation/managers.h"
#include "gui-qt/metasprite/actionpoints/resourceitem.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"
#include "models/metasprite/compiler/compiler.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::MetaSprite;

namespace MS = UnTech::MetaSprite::MetaSprite;

AbstractMsResourceItem::AbstractMsResourceItem(FrameSetResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
    , _frameSetFiles(parent->frameSetFiles())
    , _animationsList(new Animation::AnimationsList(this))
    , _animationFramesList(new Animation::AnimationFramesList(this))
{
    connect(this, &AbstractMsResourceItem::frameSetDataChanged,
            this, &AbstractResourceItem::dataChanged);

    connect(this, &AbstractResourceItem::dataChanged,
            this, &AbstractResourceItem::markUnchecked);

    connect(this, &AbstractMsResourceItem::frameSetExportOrderChanged,
            this, &AbstractMsResourceItem::onFrameSetExportOrderChanged);
}

void AbstractMsResourceItem::compileMsFrameset(const MS::FrameSet* frameSet, ErrorList& errList)
{
    using namespace UnTech::MetaSprite::Compiler;

    if (frameSet) {
        try {
            const auto& actionPointMapping = project()->staticResources()->actionPoints()->actionPointMapping();

            const auto projectFile = project()->projectFile();
            Q_ASSERT(projectFile);

            const auto* exportOrder = projectFile->frameSetExportOrders.find(frameSet->exportOrder);
            // exportOrder can be null
            validateFrameSetAndBuildTilesets(*frameSet, exportOrder, actionPointMapping, errList);
        }
        catch (std::exception& ex) {
            errList.addErrorString(ex.what());
        }
    }
}

void AbstractMsResourceItem::onFrameSetExportOrderChanged()
{
    const idstring& exportOrder = this->exportOrder();

    QVector<Dependency> dependencies;
    dependencies.append({ ResourceTypeIndex::STATIC, project()->staticResources()->actionPoints()->name() });

    if (exportOrder.isValid()) {
        dependencies.append({ ResourceTypeIndex::MS_EXPORT_ORDER, QString::fromStdString(exportOrder) });
    }

    setDependencies(dependencies);
}
