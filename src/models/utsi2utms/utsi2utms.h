#ifndef _UNTECH_MODELS_UTSI2UTMS_UTSI2UTMS_H
#define _UNTECH_MODELS_UTSI2UTMS_UTSI2UTMS_H

#include "models/metasprite/document.h"
#include "models/sprite-importer/document.h"
#include <list>
#include <memory>
#include <string>

namespace UnTech {

class Utsi2Utms {
public:
    Utsi2Utms();

    std::unique_ptr<MetaSprite::MetaSpriteDocument> convert(const SpriteImporter::SpriteImporterDocument* si);

    const std::list<std::string>& errors() const { return _errors; }
    const std::list<std::string>& warnings() const { return _warnings; }

protected:
    void addError(const std::string& message);
    void addError(const std::shared_ptr<SpriteImporter::FrameSet> frameSet, const std::string& message);
    void addError(const std::shared_ptr<SpriteImporter::Frame> frameSet, const std::string& message);
    void addWarning(const std::string& message);
    void addWarning(const std::shared_ptr<SpriteImporter::FrameSet> frameSet, const std::string& message);
    void addWarning(const std::shared_ptr<SpriteImporter::Frame> frameSet, const std::string& message);

private:
    std::list<std::string> _errors;
    std::list<std::string> _warnings;

    bool _hasError;
};
}
#endif
