/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameset-exportorder.h"
#include "models/common/errorlist.h"
#include "models/common/validateunique.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

template <class ListT>
static inline bool _testExists(const FrameSetExportOrder::ExportName& en, const ListT& list)
{
    if (list.find(en.name)) {
        return true;
    }

    auto it = std::find_if(en.alternatives.begin(), en.alternatives.end(),
                           [&](auto& a) { return list.find(a.name); });
    return it != en.alternatives.end();
}

bool FrameSetExportOrder::ExportName::frameExists(const NamedList<MetaSprite::Frame>& frameList) const
{
    return _testExists(*this, frameList);
}

bool FrameSetExportOrder::ExportName::frameExists(const NamedList<SpriteImporter::Frame>& frameList) const
{
    return _testExists(*this, frameList);
}

bool FrameSetExportOrder::ExportName::animationExists(const NamedList<Animation::Animation>& animationList) const
{
    return _testExists(*this, animationList);
}

static bool validateAlternativesUnique(const std::vector<NameReference>& alts,
                                       const std::string& typeName,
                                       const std::string& aName,
                                       UnTech::ErrorList& err)
{
    bool valid = true;
    auto addAltError = [&](const NameReference& alt, const auto... message) {
        err.addError(std::make_unique<ListItemError>(&alt, message...));
        valid = false;
    };

    for (auto it = alts.begin(); it != alts.end(); it++) {
        const NameReference& alt = *it;

        if (alt.name.isValid() == false) {
            addAltError(alt, "Missing alternative name");
        }

        auto jit = std::find(alts.begin(), it, alt);
        if (jit != it) {
            addAltError(alt, "Duplicate ", typeName, " alternative for ", aName, ": ", alt.str());
        }
    }

    return valid;
}

bool FrameSetExportOrder::validate(UnTech::ErrorList& err) const
{
    bool valid = true;
    auto addError = [&](const auto... message) {
        err.addError(std::make_unique<ListItemError>(this, message...));
        valid = false;
    };

    if (name.isValid() == false) {
        addError("Missing export order name");
    }

    if (stillFrames.empty() && animations.empty()) {
        addError("Expected at least one still frame or animation");
    }

    if (stillFrames.size() > MAX_EXPORT_NAMES) {
        addError("Too many stillFrames");
    }

    if (animations.size() > MAX_EXPORT_NAMES) {
        addError("Too many animations");
    }

    valid &= validateNamesUnique(stillFrames, "export frame", err);
    valid &= validateNamesUnique(animations, "export animation", err);

    for (auto& sf : stillFrames) {
        valid &= validateAlternativesUnique(sf.alternatives, "export frame", sf.name, err);
    }
    for (auto& ani : animations) {
        valid &= validateAlternativesUnique(ani.alternatives, "export animation", ani.name, err);
    }

    return valid;
}

template <class FrameSetT>
static bool _testFrameSet(const FrameSetExportOrder& eo, const FrameSetT& frameSet,
                          ErrorList& errorList)
{
    bool valid = true;

    for (auto& en : eo.stillFrames) {
        if (en.frameExists(frameSet.frames) == false) {
            errorList.addErrorString("Cannot find frame ", en.name);
            valid = false;
        }
    }

    for (auto& en : eo.animations) {
        if (en.animationExists(frameSet.animations) == false) {
            errorList.addErrorString("Cannot find animation ", en.name);
            valid = false;
        }
    }

    return valid;
}

bool FrameSetExportOrder::testFrameSet(const MetaSprite::FrameSet& frameSet, ErrorList& errorList) const
{
    return _testFrameSet(*this, frameSet, errorList);
}

bool FrameSetExportOrder::testFrameSet(const SpriteImporter::FrameSet& frameSet, ErrorList& errorList) const
{
    return _testFrameSet(*this, frameSet, errorList);
}
