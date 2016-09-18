#pragma once

#include "metasprite.h"
#include "spriteimporter.h"
#include <deque>
#include <iostream>
#include <string>

namespace UnTech {
namespace MetaSprite {

struct ErrorList {
    enum class FrameSetType {
        SPRITE_IMPORTER,
        SPRITE_IMPORTER_FRAME,
        SPRITE_IMPORTER_ANIMATION,
        METASPRITE,
        METASPRITE_FRAME,
        METASPRITE_ANIMATION
    };
    struct Error {
        const FrameSetType frameSetType;
        const std::string frameSetName;
        const std::string childName;
        const std::string message;

        Error(const FrameSetType frameSetType, const std::string& frameSetName,
              const std::string& childName, const std::string& message);
    };

    std::deque<Error> errors;
    std::deque<Error> warnings;

    void addError(const MetaSprite::FrameSet&, const std::string& message);
    void addError(const MetaSprite::FrameSet&, const MetaSprite::Frame&, const std::string& message);
    void addError(const MetaSprite::FrameSet&, const Animation::Animation&, const std::string& message);
    void addWarning(const MetaSprite::FrameSet&, const std::string& message);
    void addWarning(const MetaSprite::FrameSet&, const MetaSprite::Frame&, const std::string& message);
    void addWarning(const MetaSprite::FrameSet&, const Animation::Animation&, const std::string& message);

    void addError(const SpriteImporter::FrameSet&, const std::string& message);
    void addError(const SpriteImporter::FrameSet&, const SpriteImporter::Frame&, const std::string& message);
    void addError(const SpriteImporter::FrameSet&, const Animation::Animation&, const std::string& message);
    void addWarning(const SpriteImporter::FrameSet&, const std::string& message);
    void addWarning(const SpriteImporter::FrameSet&, const SpriteImporter::Frame&, const std::string& message);
    void addWarning(const SpriteImporter::FrameSet&, const Animation::Animation&, const std::string& message);
    void addWarningObj(const SpriteImporter::FrameSet&, const SpriteImporter::Frame&,
                       unsigned objectId, const std::string& message);
};

std::ostream& operator<<(std::ostream& out, const ErrorList::Error& err);
}
}
