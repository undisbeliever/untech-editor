/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "serializer.h"
#include <cassert>

using namespace UnTech::Xml;

namespace Ani = UnTech::MetaSprite::Animation;

namespace UnTech {
namespace MetaSprite {
namespace Animation {

void readAnimationFrame(const Xml::XmlTag* tag, AnimationFrame& aFrame);

void readAnimation(XmlReader& xml, const XmlTag* tag, Animation::map_t& animations)
{
    assert(tag->name == "animation");

    idstring id = tag->getAttributeUniqueId("id", animations);
    Animation& animation = animations.create(id);

    animation.durationFormat = tag->getAttributeSimpleClass<DurationFormat>("durationformat");

    animation.oneShot = tag->getAttributeBoolean("oneshot");

    if (tag->hasAttribute("next")) {
        animation.nextAnimation = tag->getAttributeId("next");
    }

    std::unique_ptr<XmlTag> childTag;

    while ((childTag = xml.parseTag())) {
        if (childTag->name == "aframe") {
            animation.frames.emplace_back();

            AnimationFrame& aFrame = animation.frames.back();
            readAnimationFrame(childTag.get(), aFrame);
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }
}

inline void readAnimationFrame(const XmlTag* tag, AnimationFrame& aFrame)
{
    assert(tag->name == "aframe");

    aFrame.frame.name = tag->getAttributeId("frame");
    aFrame.frame.hFlip = tag->getAttributeBoolean("hflip");
    aFrame.frame.vFlip = tag->getAttributeBoolean("vflip");
    aFrame.duration = tag->getAttributeUint8("duration");
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

void writeAnimations(XmlWriter& xml, const Animation::map_t& animations)
{
    for (const auto& it : animations) {
        xml.writeTag("animation");

        const idstring& id = it.first;
        const Animation& animation = it.second;

        xml.writeTagAttribute("id", id);

        xml.writeTagAttributeSimpleClass("durationformat", animation.durationFormat);

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
