/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameset-exportorder.h"
#include "metasprite-error.h"
#include "models/common/validateunique.h"

namespace UnTech::MetaSprite {

template <typename... Args>
static std::unique_ptr<ExportOrderError> exportNameError(const bool isStillFrame, unsigned enIndex, Args... msg)
{
    const EoErrorType type = isStillFrame ? EoErrorType::STILL_FRAMES : EoErrorType::ANIMATIONS;

    return std::make_unique<ExportOrderError>(type, enIndex,
                                              stringBuilder(msg...));
}

template <typename... Args>
static std::unique_ptr<ExportOrderError> altError(const bool isStillFrame, unsigned enIndex, unsigned altIndex, Args... msg)
{
    const EoErrorType type = isStillFrame ? EoErrorType::STILL_FRAMES_ALT : EoErrorType::ANIMATIONS_ALT;

    return std::make_unique<ExportOrderError>(type, enIndex, altIndex,
                                              stringBuilder(msg...));
}

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

static bool validateAlternativesUnique(const bool isStillFrame,
                                       const unsigned enIndex,
                                       const std::vector<NameReference>& alts,
                                       const std::u8string& typeName,
                                       const idstring& aName,
                                       UnTech::ErrorList& err)
{
    bool valid = true;
    auto addAltError = [&](const unsigned altIndex, const auto... message) {
        err.addError(altError(isStillFrame, enIndex, altIndex, message...));
        valid = false;
    };

    for (auto it = alts.begin(); it != alts.end(); it++) {
        const unsigned index = std::distance(alts.cbegin(), it);
        const NameReference& alt = *it;

        if (alt.name.isValid() == false) {
            addAltError(index, u8"Missing alternative name");
        }

        auto jit = std::find(alts.begin(), it, alt);
        if (jit != it) {
            addAltError(index, u8"Duplicate ", typeName, u8" alternative for ", aName, u8": ", alt.name, alt.flipStringSuffix());
        }
    }

    return valid;
}

bool validateExportOrder(const FrameSetExportOrder& input, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... message) {
        err.addErrorString(message...);
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError(u8"Missing export order name");
    }

    if (input.stillFrames.empty() && input.animations.empty()) {
        addError(u8"Expected at least one still frame or animation");
    }

    if (input.stillFrames.size() > MAX_EXPORT_NAMES) {
        addError(u8"Too many stillFrames");
    }

    if (input.animations.size() > MAX_EXPORT_NAMES) {
        addError(u8"Too many animations");
    }

    valid &= validateNamesUnique(input.stillFrames, u8"export frame", [&](unsigned i, auto... msg) {
        err.addError(exportNameError(true, i, msg...));
    });
    valid &= validateNamesUnique(input.animations, u8"export animation", [&](unsigned i, auto... msg) {
        err.addError(exportNameError(false, i, msg...));
    });

    unsigned enIndex = 0;

    for (auto& sf : input.stillFrames) {
        valid &= validateAlternativesUnique(true, enIndex, sf.alternatives, u8"export frame", sf.name, err);
        enIndex++;
    }
    for (auto& ani : input.animations) {
        valid &= validateAlternativesUnique(false, enIndex, ani.alternatives, u8"export animation", ani.name, err);
        enIndex++;
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
            errorList.addErrorString(u8"Cannot find frame ", en.name);
            valid = false;
        }
    }

    for (auto& en : eo.animations) {
        if (en.animationExists(frameSet.animations) == false) {
            errorList.addErrorString(u8"Cannot find animation ", en.name);
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

}
