/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources-serializer.h"
#include "models/common/enummap.h"
#include "models/common/xml/xmlreader.h"
#include "models/metatiles/metatiles-serializer.h"
#include <cassert>

using namespace UnTech::Xml;

namespace UnTech::Resources {

static const EnumMap<BgMode> bgModeEnumMap = {
    { u8"0", BgMode::MODE_0 },
    { u8"1", BgMode::MODE_1 },
    { u8"1bg3", BgMode::MODE_1_BG3_PRIOTITY },
    { u8"2", BgMode::MODE_2 },
    { u8"3", BgMode::MODE_3 },
    { u8"4", BgMode::MODE_4 },
};

static const EnumMap<LayerType> layerTypeEnumMap = {
    { u8"", LayerType::None },
    { u8"bg-image", LayerType::BackgroundImage },
    { u8"mt-tileset", LayerType::MetaTileTileset },
    { u8"text", LayerType::TextConsole },
};

static const EnumMap<Snes::BitDepth> bitDepthEnumMap = {
    { u8"2", Snes::BitDepth::BD_2BPP },
    { u8"4", Snes::BitDepth::BD_4BPP },
    { u8"8", Snes::BitDepth::BD_8BPP },
};

void readPalette(const XmlTag& tag, NamedList<PaletteInput>& palettes)
{
    assert(tag.name == u8"palette");

    palettes.insert_back();
    auto& palette = palettes.back();

    palette.name = tag.getAttributeId(u8"name");
    palette.paletteImageFilename = tag.getAttributeFilename(u8"image");
    palette.rowsPerFrame = tag.getAttributeUnsigned(u8"rows-per-frame");
    palette.skipFirstFrame = tag.getAttributeBoolean(u8"skip-first");

    if (tag.hasAttribute(u8"animation-delay")) {
        palette.animationDelay = tag.getAttributeUnsigned(u8"animation-delay");
    }
}

void writePalettes(XmlWriter& xml, const NamedList<PaletteInput>& palettes)
{
    for (const auto& p : palettes) {
        xml.writeTag(u8"palette");
        xml.writeTagAttribute(u8"name", p.name);
        xml.writeTagAttributeFilename(u8"image", p.paletteImageFilename);
        xml.writeTagAttribute(u8"rows-per-frame", p.rowsPerFrame);
        xml.writeTagAttribute(u8"skip-first", p.skipFirstFrame);

        if (p.animationDelay > 0) {
            xml.writeTagAttribute(u8"animation-delay", p.animationDelay);
        }

        xml.writeCloseTag();
    }
}

void readBackgroundImage(const XmlTag& tag, NamedList<BackgroundImageInput>& backgroundImages)
{
    assert(tag.name == u8"background-image");

    backgroundImages.insert_back();
    auto& bi = backgroundImages.back();

    bi.name = tag.getAttributeId(u8"name");
    bi.bitDepth = tag.getAttributeEnum(u8"bit-depth", bitDepthEnumMap);
    bi.imageFilename = tag.getAttributeFilename(u8"image");
    bi.conversionPlette = tag.getAttributeOptionalId(u8"palette");
    bi.firstPalette = tag.getAttributeUnsigned(u8"first-palette");
    bi.nPalettes = tag.getAttributeUnsigned(u8"npalettes");
    bi.defaultOrder = tag.getAttributeUnsigned(u8"default-order");
}

void writeBackgroundImages(XmlWriter& xml, const NamedList<BackgroundImageInput>& backgroundImages)
{
    for (const auto& bi : backgroundImages) {
        xml.writeTag(u8"background-image");
        xml.writeTagAttribute(u8"name", bi.name);
        xml.writeTagAttributeEnum(u8"bit-depth", bi.bitDepth, bitDepthEnumMap);
        xml.writeTagAttributeFilename(u8"image", bi.imageFilename);
        xml.writeTagAttributeOptional(u8"palette", bi.conversionPlette);
        xml.writeTagAttribute(u8"first-palette", bi.firstPalette);
        xml.writeTagAttribute(u8"npalettes", bi.nPalettes);
        xml.writeTagAttribute(u8"default-order", unsigned(bi.defaultOrder));
        xml.writeCloseTag();
    }
}

void readAnimationFramesInput(AnimationFramesInput& afi, XmlReader& xml, const XmlTag& tag)
{
    assert(tag.name == u8"animation-frames");
    assert(afi.frameImageFilenames.empty());

    afi.conversionPalette = tag.getAttributeOptionalId(u8"palette");

    afi.bitDepth = tag.getAttributeEnum(u8"bit-depth", bitDepthEnumMap);
    afi.animationDelay = tag.getAttributeUnsigned(u8"animation-delay");
    afi.addTransparentTile = tag.getAttributeBoolean(u8"add-transparent-tile");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"frame") {
            afi.frameImageFilenames.emplace_back(childTag.getAttributeFilename(u8"image"));
        }
        else {
            throw xml_error(xml, u8"Expected <frame> tag");
        }

        xml.parseCloseTag();
    }
}

void writeAnimationFramesInput(XmlWriter& xml, const AnimationFramesInput& afi)
{
    xml.writeTag(u8"animation-frames");

    xml.writeTagAttribute(u8"palette", afi.conversionPalette);
    xml.writeTagAttributeEnum(u8"bit-depth", afi.bitDepth, bitDepthEnumMap);
    xml.writeTagAttribute(u8"animation-delay", afi.animationDelay);
    xml.writeTagAttribute(u8"add-transparent-tile", afi.addTransparentTile);

    for (const std::filesystem::path& fiFilename : afi.frameImageFilenames) {
        xml.writeTag(u8"frame");
        xml.writeTagAttributeFilename(u8"image", fiFilename);
        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

void readSceneSetting(const XmlTag& tag, NamedList<SceneSettingsInput>& sceneSettings)
{
    assert(tag.name == u8"scene-setting");

    sceneSettings.insert_back();
    auto& ssi = sceneSettings.back();

    ssi.name = tag.getAttributeOptionalId(u8"name");
    ssi.bgMode = tag.getAttributeEnum(u8"bg-mode", bgModeEnumMap);
    ssi.layerTypes.at(0) = tag.getAttributeEnum(u8"layer0-type", layerTypeEnumMap);
    ssi.layerTypes.at(1) = tag.getAttributeEnum(u8"layer1-type", layerTypeEnumMap);
    ssi.layerTypes.at(2) = tag.getAttributeEnum(u8"layer2-type", layerTypeEnumMap);
    ssi.layerTypes.at(3) = tag.getAttributeEnum(u8"layer3-type", layerTypeEnumMap);
}

void writeSceneSettings(XmlWriter& xml, const NamedList<SceneSettingsInput>& sceneSettings)
{
    for (const auto& ssi : sceneSettings) {
        xml.writeTag(u8"scene-setting");

        xml.writeTagAttribute(u8"name", ssi.name);
        xml.writeTagAttributeEnum(u8"bg-mode", ssi.bgMode, bgModeEnumMap);
        xml.writeTagAttributeEnum(u8"layer0-type", ssi.layerTypes.at(0), layerTypeEnumMap);
        xml.writeTagAttributeEnum(u8"layer1-type", ssi.layerTypes.at(1), layerTypeEnumMap);
        xml.writeTagAttributeEnum(u8"layer2-type", ssi.layerTypes.at(2), layerTypeEnumMap);
        xml.writeTagAttributeEnum(u8"layer3-type", ssi.layerTypes.at(3), layerTypeEnumMap);

        xml.writeCloseTag();
    }
}

void readScene(const XmlTag& tag, NamedList<SceneInput>& scenes)
{
    assert(tag.name == u8"scene");

    scenes.insert_back();
    auto& scene = scenes.back();

    scene.name = tag.getAttributeOptionalId(u8"name");
    scene.sceneSettings = tag.getAttributeOptionalId(u8"settings");
    scene.palette = tag.getAttributeOptionalId(u8"palette");
    scene.layers.at(0) = tag.getAttributeOptionalId(u8"layer0");
    scene.layers.at(1) = tag.getAttributeOptionalId(u8"layer1");
    scene.layers.at(2) = tag.getAttributeOptionalId(u8"layer2");
    scene.layers.at(3) = tag.getAttributeOptionalId(u8"layer3");
}

void writeScenes(XmlWriter& xml, const NamedList<SceneInput>& scenes)
{
    for (const auto& scene : scenes) {
        xml.writeTag(u8"scene");

        xml.writeTagAttribute(u8"name", scene.name);
        xml.writeTagAttribute(u8"settings", scene.sceneSettings);
        xml.writeTagAttribute(u8"palette", scene.palette);
        xml.writeTagAttributeOptional(u8"layer0", scene.layers.at(0));
        xml.writeTagAttributeOptional(u8"layer1", scene.layers.at(1));
        xml.writeTagAttributeOptional(u8"layer2", scene.layers.at(2));
        xml.writeTagAttributeOptional(u8"layer3", scene.layers.at(3));

        xml.writeCloseTag();
    }
}

}
