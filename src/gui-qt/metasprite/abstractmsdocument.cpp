/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractmsdocument.h"
#include "framesetresourcelist.h"
#include "animation/animationframesmanager.h"
#include "animation/animationlistmodel.h"
#include "models/metasprite/compiler/compiler.h"

using namespace UnTech::GuiQt::MetaSprite;

AbstractMsDocument::AbstractMsDocument(FrameSetResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
{
    connect(this, &AbstractMsDocument::frameSetDataChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::frameDataChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::frameMapChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::frameObjectChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::actionPointChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::entityHitboxChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::frameObjectListChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::actionPointListChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::entityHitboxListChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::animationDataChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::animationMapChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::animationFrameChanged,
            this, &AbstractResourceItem::dataChanged);
    connect(this, &AbstractMsDocument::animationFrameListChanged,
            this, &AbstractResourceItem::dataChanged);
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

void AbstractMsDocument::compileMsFrameset(const MS::FrameSet* frameSet,
                                           UnTech::MetaSprite::ErrorList& errList)
{
    using Compiler = UnTech::MetaSprite::Compiler::Compiler;

    if (frameSet) {
        try {
            Compiler compiler(errList, 8192);
            compiler.processFrameSet(*frameSet);
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
