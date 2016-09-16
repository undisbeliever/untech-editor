#include "serializer.h"
#include <cassert>

using namespace UnTech::Xml;

namespace Ani = UnTech::MetaSprite::Animation;
template <>
const std::string Ani::Instruction::list_t::HUMAN_TYPE_NAME = "Instruction";

namespace UnTech {
namespace MetaSprite {
namespace Animation {

void readInstruction(const Xml::XmlTag* tag, Instruction& inst);

void readAnimation(XmlReader& xml, const XmlTag* tag, Animation::map_t& animations)
{
    assert(tag->name == "animation");

    std::string id = tag->getAttributeUniqueId("id", animations);
    Animation& animation = animations[id];

    std::unique_ptr<XmlTag> childTag;

    while ((childTag = xml.parseTag())) {
        if (childTag->name == "instruction") {
            animation.instructions.emplace_back();

            Instruction& inst = animation.instructions.back();
            readInstruction(childTag.get(), inst);
        }
        else {
            throw childTag->buildUnknownTagError();
        }

        xml.parseCloseTag();
    }
}

inline void readInstruction(const XmlTag* tag, Instruction& inst)
{
    assert(tag->name == "instruction");

    auto& op = inst.operation;

    op = tag->getAttributeSimpleClass<Bytecode>("op");

    if (op.usesParameter()) {
        inst.parameter = tag->getAttributeInteger("parameter");
    }

    if (op.usesGotoLabel()) {
        inst.gotoLabel = tag->getAttributeId("goto");
    }

    if (op.usesFrame()) {
        inst.frame.name = tag->getAttributeId("frame");
        inst.frame.hFlip = tag->getAttributeBoolean("hflip");
        inst.frame.vFlip = tag->getAttributeBoolean("vflip");
    }
}

/*
 * WRITER
 * ======
 */

inline void writeInstruction(XmlWriter& xml, const Instruction& instruction)
{
    xml.writeTag("instruction");

    const auto op = instruction.operation;

    xml.writeTagAttributeSimpleClass("op", op);

    if (op.usesParameter()) {
        xml.writeTagAttribute("parameter", instruction.parameter);
    }

    if (op.usesGotoLabel()) {
        xml.writeTagAttribute("goto", instruction.gotoLabel);
    }

    if (op.usesFrame()) {
        xml.writeTagAttribute("frame", instruction.frame.name);
        xml.writeTagAttribute("hflip", instruction.frame.hFlip);
        xml.writeTagAttribute("vflip", instruction.frame.vFlip);
    }

    xml.writeCloseTag();
}

void writeAnimations(XmlWriter& xml, const Animation::map_t& animations)
{
    for (const auto& it : animations) {
        xml.writeTag("animation");

        xml.writeTagAttribute("id", it.first);

        for (const auto& inst : it.second.instructions) {
            writeInstruction(xml, inst);
        }

        xml.writeCloseTag();
    }
}
}
}
}
