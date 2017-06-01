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
#include <string>
#include <vector>

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
    typedef idmap<Animation> map_t;

    Instruction::list_t instructions;

    Animation() = default;

    bool isValid(const MetaSprite::FrameSet&) const;
    bool isValid(const SpriteImporter::FrameSet&) const;

private:
    template <class FrameSetT>
    bool _isValid(const FrameSetT&) const;
};
}
}
}
