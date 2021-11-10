/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "serializer.h"
#include <cassert>

using namespace UnTech::Xml;

namespace UnTech::MetaSprite::Animation {

namespace Ani = UnTech::MetaSprite::Animation;

static const EnumMap<DurationFormat> durationFormatEnumMap = {
    { u8"FRAME", DurationFormat::FRAME },
    { u8"TIME", DurationFormat::TIME },
    { u8"DISTANCE_VERTICAL", DurationFormat::DISTANCE_VERTICAL },
    { u8"DISTANCE_HORIZONTAL", DurationFormat::DISTANCE_HORIZONTAL }
};

void readAnimationFrame(const Xml::XmlTag& tag, AnimationFrame& aFrame);

void readAnimation(XmlReader& xml, const XmlTag& tag, NamedList<Animation>& animations)
{
    assert(tag.name == u8"animation");

    animations.insert_back();
    Animation& animation = animations.back();

    animation.name = tag.getAttributeId(u8"id");

    animation.durationFormat = tag.getAttributeEnum(u8"durationformat", durationFormatEnumMap);

    animation.oneShot = tag.getAttributeBoolean(u8"oneshot");

    if (tag.hasAttribute(u8"next")) {
        animation.nextAnimation = tag.getAttributeId(u8"next");
    }

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"aframe") {
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
    assert(tag.name == u8"aframe");

    aFrame.frame.name = tag.getAttributeId(u8"frame");
    aFrame.frame.hFlip = tag.getAttributeBoolean(u8"hflip");
    aFrame.frame.vFlip = tag.getAttributeBoolean(u8"vflip");
    aFrame.duration = tag.getAttributeUint8(u8"duration");
}

/*
 * WRITER
 * ======
 */

inline void writeAnimationFrame(XmlWriter& xml, const AnimationFrame& aFrame)
{
    xml.writeTag(u8"aframe");

    xml.writeTagAttribute(u8"frame", aFrame.frame.name);
    xml.writeTagAttribute(u8"hflip", aFrame.frame.hFlip);
    xml.writeTagAttribute(u8"vflip", aFrame.frame.vFlip);
    xml.writeTagAttribute(u8"duration", aFrame.duration);

    xml.writeCloseTag();
}

void writeAnimations(XmlWriter& xml, const NamedList<Animation>& animations)
{
    for (const Animation& animation : animations) {
        xml.writeTag(u8"animation");

        xml.writeTagAttribute(u8"id", animation.name);
        xml.writeTagAttributeEnum(u8"durationformat", animation.durationFormat, durationFormatEnumMap);

        if (animation.oneShot) {
            xml.writeTagAttribute(u8"oneshot", animation.oneShot);
        }
        else {
            if (animation.nextAnimation.isValid()) {
                xml.writeTagAttribute(u8"next", animation.nextAnimation);
            }
        }

        for (const auto& aFrame : animation.frames) {
            writeAnimationFrame(xml, aFrame);
        }

        xml.writeCloseTag();
    }
}

}
