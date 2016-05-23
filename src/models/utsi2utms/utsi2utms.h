#pragma once

#include "models/metasprite/document.h"
#include "models/sprite-importer/document.h"
#include "models/sprite-importer/frameobject.h"
#include <list>
#include <memory>
#include <string>

namespace UnTech {

class Utsi2Utms {
public:
    Utsi2Utms();

    std::unique_ptr<MetaSprite::MetaSpriteDocument> convert(SpriteImporter::SpriteImporterDocument& si);

    const std::list<std::string>& errors() const { return _errors; }
    const std::list<std::string>& warnings() const { return _warnings; }

protected:
    void addError(const std::string& message);
    void addError(const SpriteImporter::FrameSet& frameSet, const std::string& message);
    void addError(const SpriteImporter::Frame& frameSet, const std::string& message);
    void addWarning(const std::string& message);
    void addWarning(const SpriteImporter::FrameSet& frameSet, const std::string& message);
    void addWarning(const SpriteImporter::Frame& frame, const std::string& message);
    void addWarning(const SpriteImporter::FrameObject& frameObj, const std::string& message);

private:
    std::list<std::string> _errors;
    std::list<std::string> _warnings;

    bool _hasError;
};
}
