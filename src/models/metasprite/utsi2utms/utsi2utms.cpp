/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "utsi2utms.h"
#include "frameconverter.hpp"
#include "paletteconverter.hpp"
#include "tileextractor.hpp"
#include "models/common/imagecache.h"
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
    size_t initialErrorCount = errorList.errorCount();
    auto hasError = [initialErrorCount, this]() {
        return initialErrorCount != errorList.errorCount();
    };

    auto msFrameSet = std::make_unique<MS::FrameSet>();

    auto image = ImageCache::loadPngImage(siFrameSet.imageFilename);

    if (image->empty()) {
        errorList.addError("Unable to load image: " + image->errorString());
        return nullptr;
    }

    msFrameSet->name = siFrameSet.name;
    msFrameSet->tilesetType = siFrameSet.tilesetType;
    msFrameSet->exportOrder = siFrameSet.exportOrder;

    msFrameSet->animations.reserve(siFrameSet.animations.size());
    for (auto& a : siFrameSet.animations) {
        msFrameSet->animations.insert_back(a);
    }

    PaletteConverter paletteConverter(siFrameSet, *image, *msFrameSet, errorList);
    paletteConverter.process();

    if (hasError()) {
        return nullptr;
    }

    TileExtractor tileExtractor(siFrameSet, *image, *msFrameSet, errorList,
                                paletteConverter.colorMap());

    // Process frames
    std::list<FrameConverter> frameConverters;

    msFrameSet->frames.resize(siFrameSet.frames.size());

    for (unsigned i = 0; i < siFrameSet.frames.size(); i++) {
        const SI::Frame& siFrame = siFrameSet.frames.at(i);
        MS::Frame& msFrame = msFrameSet->frames.at(i);

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
