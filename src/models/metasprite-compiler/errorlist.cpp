#include "errorlist.h"
#include <sstream>

using namespace UnTech::MetaSpriteCompiler;
namespace MS = UnTech::MetaSprite;
namespace MSC = UnTech::MetaSpriteCommon;

ErrorList::ErrorList()
    : _errors()
    , _warnings()
{
}

void ErrorList::addError(const std::string& message)
{
    _errors.push_back(message);
}

void ErrorList::addError(const MS::FrameSet& frameSet, const std::string& message)
{
    std::stringstream out;

    out << frameSet.name()
        << ": "
        << message;

    _errors.push_back(out.str());
}

void ErrorList::addError(const MS::Frame& frame, const std::string& message)
{
    std::stringstream out;

    const MS::FrameSet& fs = frame.frameSet();

    out << fs.name() << "." << fs.frames().getName(frame).value()
        << ": " << message;

    _errors.push_back(out.str());
}

void ErrorList::addError(const MSC::Animation& ani, const std::string& message)
{
    std::stringstream out;

    const auto& fs = ani.frameSet();

    out << fs.name() << "." << fs.animations().getName(ani).value()
        << ": " << message;

    _errors.push_back(out.str());
}

void ErrorList::addWarning(const std::string& message)
{
    _warnings.push_back(message);
}

void ErrorList::addWarning(const MS::FrameSet& frameSet, const std::string& message)
{
    std::stringstream out;

    out << frameSet.name() << ": " << message;

    _warnings.push_back(out.str());
}

void ErrorList::addWarning(const MS::Frame& frame, const std::string& message)
{
    std::stringstream out;

    const MS::FrameSet& fs = frame.frameSet();

    out << fs.name() << "." << fs.frames().getName(frame).value()
        << ": " << message;

    _warnings.push_back(out.str());
}
