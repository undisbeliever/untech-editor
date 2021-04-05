/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources-serializer.h"
#include "models/common/atomicofstream.h"
#include "models/common/enummap.h"
#include "models/common/xml/xmlreader.h"
#include "models/metatiles/metatiles-serializer.h"
#include <cassert>

using namespace UnTech::Xml;

namespace UnTech::Resources {

static const EnumMap<BgMode> bgModeEnumMap = {
    { "0", BgMode::MODE_0 },
    { "1", BgMode::MODE_1 },
    { "1bg3", BgMode::MODE_1_BG3_PRIOTITY },
    { "2", BgMode::MODE_2 },
    { "3", BgMode::MODE_3 },
    { "4", BgMode::MODE_4 },
};

static const EnumMap<LayerType> layerTypeEnumMap = {
    { "", LayerType::None },
    { "bg-image", LayerType::BackgroundImage },
    { "mt-tileset", LayerType::MetaTileTileset },
    { "text", LayerType::TextConsole },
};

void readPalette(const XmlTag& tag, NamedList<PaletteInput>& palettes)
{
    assert(tag.name == "palette");

    palettes.insert_back();
    auto& palette = palettes.back();

    palette.name = tag.getAttributeId("name");
    palette.paletteImageFilename = tag.getAttributeFilename("image");
    palette.rowsPerFrame = tag.getAttributeUnsigned("rows-per-frame");
    palette.skipFirstFrame = tag.getAttributeBoolean("skip-first");

    if (tag.hasAttribute("animation-delay")) {
        palette.animationDelay = tag.getAttributeUnsigned("animation-delay");
    }
}

void writePalettes(XmlWriter& xml, const NamedList<PaletteInput>& palettes)
{
    for (const auto& p : palettes) {
        xml.writeTag("palette");
        xml.writeTagAttribute("name", p.name);
        xml.writeTagAttributeFilename("image", p.paletteImageFilename);
        xml.writeTagAttribute("rows-per-frame", p.rowsPerFrame);
        xml.writeTagAttribute("skip-first", p.skipFirstFrame);

        if (p.animationDelay > 0) {
            xml.writeTagAttribute("animation-delay", p.animationDelay);
        }

        xml.writeCloseTag();
    }
}

void readBackgroundImage(const XmlTag& tag, NamedList<BackgroundImageInput>& backgroundImages)
{
    assert(tag.name == "background-image");

    backgroundImages.insert_back();
    auto& bi = backgroundImages.back();

    bi.name = tag.getAttributeId("name");
    bi.bitDepth = tag.getAttributeUnsigned("bit-depth");
    bi.imageFilename = tag.getAttributeFilename("image");
    bi.conversionPlette = tag.getAttributeOptionalId("palette");
    bi.firstPalette = tag.getAttributeUnsigned("first-palette");
    bi.nPalettes = tag.getAttributeUnsigned("npalettes");
    bi.defaultOrder = tag.getAttributeUnsigned("default-order");
}

void writeBackgroundImages(XmlWriter& xml, const NamedList<BackgroundImageInput>& backgroundImages)
{
    for (const auto& bi : backgroundImages) {
        xml.writeTag("background-image");
        xml.writeTagAttribute("name", bi.name);
        xml.writeTagAttribute("bit-depth", bi.bitDepth);
        xml.writeTagAttributeFilename("image", bi.imageFilename);
        xml.writeTagAttributeOptional("palette", bi.conversionPlette);
        xml.writeTagAttribute("first-palette", bi.firstPalette);
        xml.writeTagAttribute("npalettes", bi.nPalettes);
        xml.writeTagAttribute("default-order", unsigned(bi.defaultOrder));
        xml.writeCloseTag();
    }
}

void readAnimationFramesInput(AnimationFramesInput& afi, XmlReader& xml, const XmlTag& tag)
{
    assert(tag.name == "animation-frames");
    assert(afi.frameImageFilenames.empty());

    afi.conversionPalette = tag.getAttributeOptionalId("palette");

    afi.bitDepth = tag.getAttributeUnsigned("bit-depth", 2, 8);
    afi.animationDelay = tag.getAttributeUnsigned("animation-delay");
    afi.addTransparentTile = tag.getAttributeBoolean("add-transparent-tile");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == "frame") {
            afi.frameImageFilenames.emplace_back(childTag.getAttributeFilename("image"));
        }
        else {
            throw xml_error(xml, "Expected <frame> tag");
        }

        xml.parseCloseTag();
    }
}

void writeAnimationFramesInput(XmlWriter& xml, const AnimationFramesInput& afi)
{
    xml.writeTag("animation-frames");

    xml.writeTagAttribute("palette", afi.conversionPalette);
    xml.writeTagAttribute("bit-depth", afi.bitDepth);
    xml.writeTagAttribute("animation-delay", afi.animationDelay);
    xml.writeTagAttribute("add-transparent-tile", afi.addTransparentTile);

    for (const std::filesystem::path& fiFilename : afi.frameImageFilenames) {
        xml.writeTag("frame");
        xml.writeTagAttributeFilename("image", fiFilename);
        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

void readSceneSetting(const XmlTag& tag, NamedList<SceneSettingsInput>& sceneSettings)
{
    assert(tag.name == "scene-setting");

    sceneSettings.insert_back();
    auto& ssi = sceneSettings.back();

    ssi.name = tag.getAttributeOptionalId("name");
    ssi.bgMode = tag.getAttributeEnum("bg-mode", bgModeEnumMap);
    ssi.layerTypes.at(0) = tag.getAttributeEnum("layer0-type", layerTypeEnumMap);
    ssi.layerTypes.at(1) = tag.getAttributeEnum("layer1-type", layerTypeEnumMap);
    ssi.layerTypes.at(2) = tag.getAttributeEnum("layer2-type", layerTypeEnumMap);
    ssi.layerTypes.at(3) = tag.getAttributeEnum("layer3-type", layerTypeEnumMap);
}

void writeSceneSettings(XmlWriter& xml, const NamedList<SceneSettingsInput>& sceneSettings)
{
    for (const auto& ssi : sceneSettings) {
        xml.writeTag("scene-setting");

        xml.writeTagAttribute("name", ssi.name);
        xml.writeTagAttributeEnum("bg-mode", ssi.bgMode, bgModeEnumMap);
        xml.writeTagAttributeEnum("layer0-type", ssi.layerTypes.at(0), layerTypeEnumMap);
        xml.writeTagAttributeEnum("layer1-type", ssi.layerTypes.at(1), layerTypeEnumMap);
        xml.writeTagAttributeEnum("layer2-type", ssi.layerTypes.at(2), layerTypeEnumMap);
        xml.writeTagAttributeEnum("layer3-type", ssi.layerTypes.at(3), layerTypeEnumMap);

        xml.writeCloseTag();
    }
}

void readScene(const XmlTag& tag, NamedList<SceneInput>& scenes)
{
    assert(tag.name == "scene");

    scenes.insert_back();
    auto& scene = scenes.back();

    scene.name = tag.getAttributeOptionalId("name");
    scene.sceneSettings = tag.getAttributeOptionalId("settings");
    scene.palette = tag.getAttributeOptionalId("palette");
    scene.layers.at(0) = tag.getAttributeOptionalId("layer0");
    scene.layers.at(1) = tag.getAttributeOptionalId("layer1");
    scene.layers.at(2) = tag.getAttributeOptionalId("layer2");
    scene.layers.at(3) = tag.getAttributeOptionalId("layer3");
}

void writeScenes(XmlWriter& xml, const NamedList<SceneInput>& scenes)
{
    for (const auto& scene : scenes) {
        xml.writeTag("scene");

        xml.writeTagAttribute("name", scene.name);
        xml.writeTagAttribute("settings", scene.sceneSettings);
        xml.writeTagAttribute("palette", scene.palette);
        xml.writeTagAttributeOptional("layer0", scene.layers.at(0));
        xml.writeTagAttributeOptional("layer1", scene.layers.at(1));
        xml.writeTagAttributeOptional("layer2", scene.layers.at(2));
        xml.writeTagAttributeOptional("layer3", scene.layers.at(3));

        xml.writeCloseTag();
    }
}

}
