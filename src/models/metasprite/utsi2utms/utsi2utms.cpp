/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "utsi2utms.h"
#include "frameconverter.hpp"
#include "paletteconverter.hpp"
#include "tileextractor.hpp"
#include "models/common/imagecache.h"

namespace UnTech::MetaSprite {
namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;
}

namespace UnTech::MetaSprite::Utsi2UtmsPrivate {

static std::unique_ptr<MS::FrameSet> utsi2utms(const SI::FrameSet& siFrameSet, ErrorList& errorList)
{
    size_t initialErrorCount = errorList.errorCount();
    auto hasError = [&]() {
        return initialErrorCount != errorList.errorCount();
    };

    auto msFrameSet = std::make_unique<MS::FrameSet>();

    auto image = ImageCache::loadPngImage(siFrameSet.imageFilename);

    if (image->empty()) {
        errorList.addErrorString(u8"Unable to load image: ", image->errorString());
        return nullptr;
    }

    msFrameSet->name = siFrameSet.name;
    msFrameSet->tilesetType = siFrameSet.tilesetType;
    msFrameSet->exportOrder = siFrameSet.exportOrder;

    msFrameSet->animations.reserve(siFrameSet.animations.size());
    for (auto& a : siFrameSet.animations) {
        msFrameSet->animations.insert_back(a);
    }

    ColorMapT colorMap;
    std::tie(msFrameSet->palettes, colorMap) = buildPalette(siFrameSet, *image, errorList);

    if (hasError()) {
        return nullptr;
    }

    TileExtractor tileExtractor(*msFrameSet, *image, colorMap);
    processFrames(msFrameSet->frames, tileExtractor, siFrameSet.frames, siFrameSet.grid, errorList);

    if (hasError()) {
        return nullptr;
    }

    return msFrameSet;
}

}

namespace UnTech::MetaSprite {

// ::TODO add a section to untech-editor-gui that allows you to see the output of utsi2utms ::
// :::    and allows the user to save it to a file (if he/she wishes).::

std::unique_ptr<MS::FrameSet> utsi2utms(SI::FrameSet& siFrameSet, ErrorList& errorList)
{
    bool valid = validate(siFrameSet, errorList);
    if (!valid) {
        return nullptr;
    }

    return Utsi2UtmsPrivate::utsi2utms(siFrameSet, errorList);
}

}
