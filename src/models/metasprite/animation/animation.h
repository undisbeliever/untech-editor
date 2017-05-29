/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "bytecode.h"
#include "../common.h"
#include "models/common/capped_vector.h"
#include "models/common/idmap.h"
#include "models/common/idstring.h"
#include <cassert>
#include <string>

namespace UnTech {
namespace MetaSprite {
namespace MetaSprite {
struct FrameSet;
}
namespace SpriteImporter {
struct FrameSet;
}

namespace Animation {
struct Animation;

struct Instruction {
    typedef capped_vector<Instruction, MAX_ANIMATION_INSTRUCTIONS> list_t;

    Bytecode operation;
    idstring gotoLabel;
    NameReference frame;
    int parameter;

    Instruction() = default;

    bool isValid(const MetaSprite::FrameSet&, const Animation&, unsigned pos) const;
    bool isValid(const SpriteImporter::FrameSet&, const Animation&, unsigned pos) const;

private:
    template <class FrameSetT>
    bool _isValid(const FrameSetT&, const Animation&, unsigned pos) const;
};

struct Animation {
    const static unsigned INSTRUCTION_TIMEOUT = 5;

    typedef idmap<Animation> map_t;

    Instruction::list_t instructions;

    Animation() = default;

    bool isValid(const MetaSprite::FrameSet&) const;
    bool isValid(const SpriteImporter::FrameSet&) const;

    // Returns false if more than `INSTRUCTION_TIMEOUT` successive instructions
    // occur without a Wait/Pause instruction.
    // Used to detect slow bytecode and infinite loops.
    bool isTimely(const Animation::map_t&) const;

private:
    template <class FrameSetT>
    bool _isValid(const FrameSetT&) const;
};

class ProgramCounter {
    friend struct std::hash<ProgramCounter>;

public:
    ProgramCounter(const Animation::map_t&, const Animation&);

    // Throws runtime_error if the current instruction is a GOTO instruction
    // with an invalid parameter.
    void step();

    unsigned index() const { return _index; }

    const Animation& animation() const
    {
        assert(_animation != nullptr);
        return *_animation;
    }

    const Instruction& instruction() const
    {
        assert(_animation != nullptr);
        return _animation->instructions.at(_index);
    }

    bool operator==(const ProgramCounter& o) const
    {
        return std::tie(_animation, _index) == std::tie(o._animation, o._index);
    }

private:
    const Animation::map_t& _animations;

    const Animation* _animation;
    unsigned _index;
};
}
}
}

namespace std {
template <>
struct hash<::UnTech::MetaSprite::Animation::ProgramCounter> {
    size_t operator()(const ::UnTech::MetaSprite::Animation::ProgramCounter& pc) const
    {
        return std::hash<int>()(pc._index)
               ^ std::hash<const void*>()(&pc._animation);
    }
};
}
