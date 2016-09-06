#pragma once

#include "models/metasprite.h"
#include <list>
#include <string>

namespace UnTech {
namespace MetaSpriteCompiler {

class ErrorList {
public:
    ErrorList();

    const std::list<std::string>& errors() const { return _errors; }
    const std::list<std::string>& warnings() const { return _warnings; }

public:
    void addError(const std::string& message);
    void addError(const MetaSprite::FrameSet& frameSet, const std::string& message);
    void addError(const MetaSprite::Frame&, const std::string& message);
    void addError(const MetaSpriteCommon::Animation&, const std::string& message);
    void addWarning(const std::string& message);
    void addWarning(const MetaSprite::FrameSet& frameSet, const std::string& message);
    void addWarning(const MetaSprite::Frame& frame, const std::string& message);

private:
    std::list<std::string> _errors;
    std::list<std::string> _warnings;
};
}
}
