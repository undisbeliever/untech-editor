#include "animationserializer.h"
#include <cassert>

using namespace UnTech::Xml;

namespace UnTech {
namespace MetaSpriteCommon {

inline void readAnimationInstruction(const XmlTag* tag, AnimationInstruction& inst);

void readAnimation(XmlReader& xml, const XmlTag* tag, Animation::list_t& animations)
{
    assert(tag->name == "animation");

    std::string id = tag->getAttributeId("id");
    if (animations.nameExists(id)) {
        throw tag->buildError("animation id already exists");
    }

    Animation* animation = animations.create(id);
    if (animation == nullptr) {
        throw std::logic_error("Could not create Animation");
    }

    std::unique_ptr<XmlTag> childTag;

    while ((childTag = xml.parseTag())) {
        if (childTag->name == "instruction") {
            AnimationInstruction& inst = animation->instructions().create();
            readAnimationInstruction(childTag.get(), inst);
        }
        else {
            throw childTag->buildUnknownTagError();
        }

        xml.parseCloseTag();
    }
}

inline void readAnimationInstruction(const XmlTag* tag, AnimationInstruction& inst)
{
    assert(tag->name == "instruction");

    auto op = tag->getAttributeSimpleClass<AnimationBytecode>("op");

    inst.setOperation(op);

    if (op.usesParameter()) {
        int p = tag->getAttributeInteger("parameter");
        inst.setParameter(p);

        if (inst.parameter() != p) {
            tag->buildError("Invalid parameter");
        }
    }

    if (op.usesGotoLabel()) {
        inst.setGotoLabel(tag->getAttributeId("goto"));
    }

    if (op.usesFrame()) {
        FrameReference ref;
        ref.frameName = tag->getAttributeId("frame");
        ref.hFlip = tag->getAttributeBoolean("hflip");
        ref.vFlip = tag->getAttributeBoolean("vflip");

        inst.setFrame(ref);
    }
}

/*
 * WRITER
 * ======
 */

inline void writeAnimationInstruction(XmlWriter& xml, const AnimationInstruction& instruction)
{
    xml.writeTag("instruction");

    const auto op = instruction.operation();

    xml.writeTagAttributeSimpleClass("op", op);

    if (op.usesParameter()) {
        xml.writeTagAttribute("parameter", instruction.parameter());
    }

    if (op.usesGotoLabel()) {
        xml.writeTagAttribute("goto", instruction.gotoLabel());
    }

    if (op.usesFrame()) {
        FrameReference ref = instruction.frame();

        xml.writeTagAttribute("frame", ref.frameName);
        xml.writeTagAttribute("hflip", ref.hFlip);
        xml.writeTagAttribute("vflip", ref.vFlip);
    }

    xml.writeCloseTag();
}

void writeAnimations(XmlWriter& xml, const Animation::list_t& animations)
{
    for (const auto& it : animations) {
        xml.writeTag("animation");

        xml.writeTagAttribute("id", it.first);

        for (const auto& inst : it.second.instructions()) {
            writeAnimationInstruction(xml, inst);
        }

        xml.writeCloseTag();
    }
}
}
}
