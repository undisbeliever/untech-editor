#pragma once

#include "../metasprite.h"
#include "../spriteimporter.h"
#include <list>
#include <memory>
#include <string>

namespace UnTech {
namespace MetaSprite {

class Utsi2Utms {
public:
    Utsi2Utms();

    std::unique_ptr<MetaSprite::FrameSet> convert(const SpriteImporter::FrameSet& siFrameSet);

    const std::list<std::string>& errors() const { return _errors; }
    const std::list<std::string>& warnings() const { return _warnings; }

protected:
    void addError(const std::string& message);
    void addError(const SpriteImporter::FrameSet& frameSet, const std::string& message);
    void addError(const SpriteImporter::FrameSet& frameSet,
                  const std::string& frame, const std::string& message);
    void addWarning(const std::string& message);
    void addWarning(const SpriteImporter::FrameSet& frameSet, const std::string& message);
    void addWarning(const SpriteImporter::FrameSet& frameSet,
                    const std::string& frame, const std::string& message);
    void addWarningObj(const SpriteImporter::FrameSet& frameSet,
                       const std::string& frame, unsigned objectId,
                       const std::string& message);

private:
    std::list<std::string> _errors;
    std::list<std::string> _warnings;

    bool _hasError;
};
}
}
