#pragma once

#include "animationbytecode.h"
#include "models/common/namedlist.h"
#include "models/common/orderedlist.h"
#include "models/document.h"
#include <string>

namespace UnTech {
namespace MetaSpriteCommon {

class AbstractFrameSet;
class AnimationInstruction;

class Animation {
public:
    static const char* TYPE_NAME;
    typedef NamedList<AbstractFrameSet, Animation> list_t;

public:
    Animation(AbstractFrameSet& parent);
    Animation(const Animation&, AbstractFrameSet& parent);

    Animation() = delete;
    Animation(const Animation&) = delete;
    ~Animation() = default;

    inline AbstractFrameSet& frameSet() const { return _frameSet; }

    auto& instructions() { return _instructions; }
    const auto& instructions() const { return _instructions; }

    bool isValid() const;

private:
    AbstractFrameSet& _frameSet;
    OrderedList<Animation, AnimationInstruction> _instructions;
};

struct FrameReference {
    std::string frameName;
    bool hFlip;
    bool vFlip;

    bool operator==(const FrameReference& other)
    {
        return frameName == other.frameName
               && hFlip == other.hFlip && vFlip == other.vFlip;
    }

    bool operator!=(const FrameReference& other)
    {
        return frameName != other.frameName
               || hFlip != other.hFlip || vFlip != other.vFlip;
    }
};

class AnimationInstruction {
public:
    static const char* TYPE_NAME;
    typedef OrderedList<Animation, AnimationInstruction> list_t;

public:
    AnimationInstruction(Animation& parent);
    AnimationInstruction(const AnimationInstruction&, Animation& parent);

    AnimationInstruction() = delete;
    AnimationInstruction(const AnimationInstruction&) = delete;
    ~AnimationInstruction() = default;

    inline Animation& animation() const { return _animation; }

    const AnimationBytecode& operation() const { return _operation; }
    void setOperation(const AnimationBytecode& operation) { _operation = operation; }

    bool usesParameter() const { return _operation.usesParameter(); }
    bool parameterUnsigned() const { return _operation.parameterUnsigned(); }
    int parameter() const { return _parameter; }
    void setParameter(int);

    bool usesFrame() const { return _operation.usesFrame(); }
    const FrameReference& frame() const { return _frame; }
    void setFrame(const FrameReference&);

    bool usesGotoLabel() const { return _operation.usesGotoLabel(); }
    const std::string& gotoLabel() const { return _gotoLabel; }
    void setGotoLabel(const std::string&);

    bool isValid(unsigned position) const;

private:
    Animation& _animation;
    AnimationBytecode _operation;
    std::string _gotoLabel;
    FrameReference _frame;
    int _parameter;
};
}
}
