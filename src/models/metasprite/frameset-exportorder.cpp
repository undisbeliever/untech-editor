/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameset-exportorder.h"
#include "models/common/validateunique.h"
#include "models/resources/error-list.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

template <class MapT>
static inline bool _testExists(const FrameSetExportOrder::ExportName& en, const MapT& map)
{
    if (map.contains(en.name)) {
        return true;
    }

    auto it = std::find_if(en.alternatives.begin(), en.alternatives.end(),
                           [&](auto& a) { return map.contains(a.name); });
    return it != en.alternatives.end();
}

bool FrameSetExportOrder::ExportName::frameExists(const MetaSprite::Frame::map_t& frameMap) const
{
    return _testExists(*this, frameMap);
}

bool FrameSetExportOrder::ExportName::frameExists(const SpriteImporter::Frame::map_t& frameMap) const
{
    return _testExists(*this, frameMap);
}

bool FrameSetExportOrder::ExportName::animationExists(const Animation::Animation::map_t& animationMap) const
{
    return _testExists(*this, animationMap);
}

static bool validateAlternativesUnique(const std::vector<NameReference>& alts,
                                       const std::string& typeName,
                                       const std::string& aName,
                                       Resources::ErrorList& err)
{
    bool valid = true;

    for (auto it = alts.begin(); it != alts.end(); it++) {
        const NameReference& alt = *it;

        auto jit = std::find(it + 1, alts.end(), alt);
        if (jit != alts.end()) {
            err.addError("Duplicate " + typeName + " alternative for " + aName + ": " + alt.str());
            valid = false;
        }
    }

    return valid;
}

bool FrameSetExportOrder::validate(Resources::ErrorList& err) const
{
    bool valid = true;

    if (name.isValid() == false) {
        err.addError("Missing export order name");
        valid = false;
    }

    if (stillFrames.empty() && animations.empty()) {
        err.addError("Expected at least one still frame or animation");
        valid = false;
    }

    if (stillFrames.size() > MAX_EXPORT_NAMES) {
        err.addError("Too many stillFrames");
        valid = false;
    }

    if (animations.size() > MAX_EXPORT_NAMES) {
        err.addError("Too many animations");
        valid = false;
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
static bool _testFrameSet(const FrameSetExportOrder& eo, const FrameSetT& frameSet, ErrorList& errorList)
{
    bool valid = true;

    for (auto& en : eo.stillFrames) {
        if (en.frameExists(frameSet.frames) == false) {
            errorList.addError(frameSet, "Cannot find frame " + en.name);
            valid = false;
        }
    }

    for (auto& en : eo.animations) {
        if (en.animationExists(frameSet.animations) == false) {
            errorList.addError(frameSet, "Cannot find animation " + en.name);
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
