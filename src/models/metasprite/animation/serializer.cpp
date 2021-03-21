/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "serializer.h"
#include <cassert>

using namespace UnTech::Xml;

namespace Ani = UnTech::MetaSprite::Animation;

namespace UnTech {
namespace MetaSprite {
namespace Animation {

void readAnimationFrame(const Xml::XmlTag& tag, AnimationFrame& aFrame);

void readAnimation(XmlReader& xml, const XmlTag& tag, NamedList<Animation>& animations)
{
    assert(tag.name == "animation");

    animations.insert_back();
    Animation& animation = animations.back();

    animation.name = tag.getAttributeId("id");

    animation.durationFormat = tag.getAttributeEnum<DurationFormat>("durationformat");

    animation.oneShot = tag.getAttributeBoolean("oneshot");

    if (tag.hasAttribute("next")) {
        animation.nextAnimation = tag.getAttributeId("next");
    }

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == "aframe") {
            animation.frames.emplace_back();

            AnimationFrame& aFrame = animation.frames.back();
            readAnimationFrame(childTag, aFrame);
        }
        else {
            throw unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }
}

inline void readAnimationFrame(const XmlTag& tag, AnimationFrame& aFrame)
{
    assert(tag.name == "aframe");

    aFrame.frame.name = tag.getAttributeId("frame");
    aFrame.frame.hFlip = tag.getAttributeBoolean("hflip");
    aFrame.frame.vFlip = tag.getAttributeBoolean("vflip");
    aFrame.duration = tag.getAttributeUint8("duration");
}

/*
 * WRITER
 * ======
 */

inline void writeAnimationFrame(XmlWriter& xml, const AnimationFrame& aFrame)
{
    xml.writeTag("aframe");

    xml.writeTagAttribute("frame", aFrame.frame.name);
    xml.writeTagAttribute("hflip", aFrame.frame.hFlip);
    xml.writeTagAttribute("vflip", aFrame.frame.vFlip);
    xml.writeTagAttribute("duration", aFrame.duration);

    xml.writeCloseTag();
}

void writeAnimations(XmlWriter& xml, const NamedList<Animation>& animations)
{
    for (const Animation& animation : animations) {
        xml.writeTag("animation");

        xml.writeTagAttribute("id", animation.name);
        xml.writeTagAttributeEnum("durationformat", animation.durationFormat);

        if (animation.oneShot) {
            xml.writeTagAttribute("oneshot", animation.oneShot);
        }
        else {
            if (animation.nextAnimation.isValid()) {
                xml.writeTagAttribute("next", animation.nextAnimation);
            }
        }

        for (const auto& aFrame : animation.frames) {
            writeAnimationFrame(xml, aFrame);
        }

        xml.writeCloseTag();
    }
}
}
}
}
