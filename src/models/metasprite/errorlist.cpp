/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "errorlist.h"
#include <sstream>

using namespace UnTech::MetaSprite;
using FST = ErrorList::FrameSetType;

const std::string emptyString;

ErrorList::Error::Error(const FrameSetType frameSetType, const std::string& frameSetName,
                        const std::string& childName, const std::string& message)
    : frameSetType(frameSetType)
    , frameSetName(frameSetName)
    , childName(childName)
    , message(message)
{
}

std::ostream& UnTech::MetaSprite::operator<<(std::ostream& out, const ErrorList::Error& e)
{
    switch (e.frameSetType) {
    case FST::SPRITE_IMPORTER:
    case FST::SPRITE_IMPORTER_FRAME:
    case FST::SPRITE_IMPORTER_ANIMATION:
        out << "SpriteImporter";
        break;
    case FST::METASPRITE:
    case FST::METASPRITE_FRAME:
    case FST::METASPRITE_ANIMATION:
        out << "MetaSprite";
        break;
    }

    if (!e.frameSetName.empty()) {
        out << "." << e.frameSetName;
    }

    switch (e.frameSetType) {
    case FST::SPRITE_IMPORTER:
    case FST::METASPRITE:
        break;
    case FST::SPRITE_IMPORTER_FRAME:
    case FST::METASPRITE_FRAME:
        out << ".Frames";
        break;
    case FST::SPRITE_IMPORTER_ANIMATION:
    case FST::METASPRITE_ANIMATION:
        out << ".Animations";
        break;
    }

    if (!e.childName.empty()) {
        out << "." << e.childName;
    }

    out << ": " << e.message;

    return out;
}

std::string _nameOf(const MetaSprite::FrameSet& fs, const MetaSprite::Frame& frame)
{
    for (const auto& it : fs.frames) {
        if (&frame == &it.second) {
            return it.first;
        }
    }
    return emptyString;
}

std::string _nameOf(const SpriteImporter::FrameSet& fs, const SpriteImporter::Frame& frame)
{
    for (const auto& it : fs.frames) {
        if (&frame == &it.second) {
            return it.first;
        }
    }
    return emptyString;
}

std::string _nameOf(const MetaSprite::FrameSet& fs, const Animation::Animation& ani)
{
    for (const auto& it : fs.animations) {
        if (&ani == &it.second) {
            return it.first;
        }
    }
    return emptyString;
}

std::string _nameOf(const SpriteImporter::FrameSet& fs, const Animation::Animation& ani)
{
    for (const auto& it : fs.animations) {
        if (&ani == &it.second) {
            return it.first;
        }
    }
    return emptyString;
}

void ErrorList::addError(const MetaSprite::FrameSet& fs, const std::string& message)
{
    errors.emplace_back(FST::METASPRITE, fs.name, emptyString, message);
}

void ErrorList::addError(const MetaSprite::FrameSet& fs, const MetaSprite::Frame& frame,
                         const std::string& message)
{
    std::string frameName = _nameOf(fs, frame);
    errors.emplace_back(FST::METASPRITE_FRAME, fs.name, frameName, message);
}

void ErrorList::addError(const MetaSprite::FrameSet& fs, const Animation::Animation& ani,
                         const std::string& message)
{
    std::string aniName = _nameOf(fs, ani);
    errors.emplace_back(FST::METASPRITE_ANIMATION, fs.name, aniName, message);
}

void ErrorList::addWarning(const MetaSprite::FrameSet& fs, const std::string& message)
{
    errors.emplace_back(FST::METASPRITE, fs.name, emptyString, message);
}

void ErrorList::addWarning(const MetaSprite::FrameSet& fs, const MetaSprite::Frame& frame,
                           const std::string& message)
{
    std::string frameName = _nameOf(fs, frame);
    warnings.emplace_back(FST::METASPRITE_FRAME, fs.name, frameName, message);
}

void ErrorList::addWarning(const MetaSprite::FrameSet& fs, const Animation::Animation& ani,
                           const std::string& message)
{
    std::string aniName = _nameOf(fs, ani);
    warnings.emplace_back(FST::METASPRITE_ANIMATION, fs.name, aniName, message);
}

void ErrorList::addError(const SpriteImporter::FrameSet& fs, const std::string& message)
{
    errors.emplace_back(FST::SPRITE_IMPORTER, fs.name, emptyString, message);
}

void ErrorList::addError(const SpriteImporter::FrameSet& fs, const SpriteImporter::Frame& frame,
                         const std::string& message)
{
    std::string frameName = _nameOf(fs, frame);
    errors.emplace_back(FST::SPRITE_IMPORTER_FRAME, fs.name, frameName, message);
}

void ErrorList::addError(const SpriteImporter::FrameSet& fs, const Animation::Animation& ani,
                         const std::string& message)
{
    std::string aniName = _nameOf(fs, ani);
    errors.emplace_back(FST::SPRITE_IMPORTER_ANIMATION, fs.name, aniName, message);
}

void ErrorList::addWarning(const SpriteImporter::FrameSet& fs, const std::string& message)
{
    errors.emplace_back(FST::SPRITE_IMPORTER, fs.name, emptyString, message);
}

void ErrorList::addWarning(const SpriteImporter::FrameSet& fs, const SpriteImporter::Frame& frame,
                           const std::string& message)
{
    std::string frameName = _nameOf(fs, frame);
    warnings.emplace_back(FST::SPRITE_IMPORTER_FRAME, fs.name, frameName, message);
}

void ErrorList::addWarning(const SpriteImporter::FrameSet& fs, const Animation::Animation& ani,
                           const std::string& message)
{
    std::string aniName = _nameOf(fs, ani);
    warnings.emplace_back(FST::SPRITE_IMPORTER_ANIMATION, fs.name, aniName, message);
}

void ErrorList::addWarningObj(const SpriteImporter::FrameSet& fs, const SpriteImporter::Frame& frame,
                              unsigned objectId, const std::string& message)
{
    std::stringstream msg;
    msg << "(object " << objectId << "): " << message;

    std::string frameName = _nameOf(fs, frame);
    warnings.emplace_back(FST::SPRITE_IMPORTER_FRAME, fs.name, frameName, msg.str());
}
