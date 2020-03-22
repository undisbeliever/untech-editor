/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animation-frames-input.h"
#include "background-image.h"
#include "scenes.h"
#include "models/common/namedlist.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace Resources {

// raises exception on error
void readPalette(const Xml::XmlTag* tag, NamedList<PaletteInput>& palettes);

// raises exception on error
void writePalettes(Xml::XmlWriter& xml, const NamedList<PaletteInput>& palettes);

// raises exception on error
void readBackgroundImage(const Xml::XmlTag* tag, NamedList<BackgroundImageInput>& backgroundImages);

// raises exception on error
void writeBackgroundImages(Xml::XmlWriter& xml, const NamedList<BackgroundImageInput>& backgroundImages);

// raises exception on error
// `afi` must be empty, xml/tag points to an <animation-frames> tag
void readAnimationFramesInput(AnimationFramesInput& afi,
                              Xml::XmlReader& xml, const Xml::XmlTag* tag);

// raises exception on error
void writeAnimationFramesInput(Xml::XmlWriter& xml, const AnimationFramesInput& afi);

// raises exception on error
void readSceneSetting(const Xml::XmlTag* tag, NamedList<SceneSettingsInput>& sceneSettings);

// raises exception on error
void writeSceneSettings(Xml::XmlWriter& xml, const NamedList<SceneSettingsInput>& sceneSettings);

// raises exception on error
void readScene(const Xml::XmlTag* tag, NamedList<SceneInput>& scenes);

// raises exception on error
void writeScenes(Xml::XmlWriter& xml, const NamedList<SceneInput>& scenes);

}
}
