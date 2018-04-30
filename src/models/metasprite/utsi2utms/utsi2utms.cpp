/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "utsi2utms.h"
#include "frameconverter.hpp"
#include "paletteconverter.hpp"
#include "tileextractor.hpp"
#include <list>

using namespace UnTech;
using namespace UnTech::MetaSprite;
using namespace UnTech::MetaSprite::Utsi2UtmsPrivate;

Utsi2Utms::Utsi2Utms(ErrorList& errorList)
    : errorList(errorList)
{
}

std::unique_ptr<MS::FrameSet> Utsi2Utms::convert(SI::FrameSet& siFrameSet)
{
    bool valid = siFrameSet.validate(errorList);
    if (!valid) {
        return nullptr;
    }

    return process(siFrameSet);
}

std::unique_ptr<MS::FrameSet> Utsi2Utms::process(const SI::FrameSet& siFrameSet)
{
    size_t initialErrorCount = errorList.errors.size();
    auto hasError = [initialErrorCount, this]() {
        return initialErrorCount != errorList.errors.size();
    };

    auto msFrameSet = std::make_unique<MS::FrameSet>();

    msFrameSet->name = siFrameSet.name;
    msFrameSet->tilesetType = siFrameSet.tilesetType;
    msFrameSet->animations = siFrameSet.animations;
    msFrameSet->exportOrder = siFrameSet.exportOrder;

    PaletteConverter paletteConverter(siFrameSet, *msFrameSet, errorList);
    paletteConverter.process();

    if (hasError()) {
        return nullptr;
    }

    TileExtractor tileExtractor(siFrameSet, *msFrameSet, errorList,
                                paletteConverter.colorMap());

    std::list<FrameConverter> frameConverters;

    // Process frames
    for (const auto& frameIt : siFrameSet.frames) {
        const idstring& frameName = frameIt.first;
        const SI::Frame& siFrame = frameIt.second;

        MS::Frame& msFrame = msFrameSet->frames.create(frameName);

        frameConverters.emplace_back(tileExtractor, siFrame, msFrame);
        frameConverters.back().process();
    }

    if (hasError()) {
        return nullptr;
    }

    // Process overlapping tiles
    for (auto& converter : frameConverters) {
        converter.processOverlappingTiles();
    }

    if (hasError()) {
        return nullptr;
    }

    return msFrameSet;
}
