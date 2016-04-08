#ifndef _UNTECH_MODELS_METASPRITE_SERIALIZER_H
#define _UNTECH_MODELS_METASPRITE_SERIALIZER_H

#include <string>
#include <ostream>
#include <memory>

/**
 * YOU SHOULD NOT CALL THIS CLASS DIRECTLY.
 *
 * It is called by the MetaSpriteDocument class.
 */

namespace UnTech {
namespace MetaSprite {

class FrameSet;

namespace Serializer {

// NOTE: FrameSet MUST be empty
void readFile(std::shared_ptr<FrameSet> frameSet, const std::string& filename);

void writeFile(const FrameSet& frameSet, std::ostream& file);
void writeFile(const FrameSet& frameSet, const std::string& filename);
}
}
}

#endif
