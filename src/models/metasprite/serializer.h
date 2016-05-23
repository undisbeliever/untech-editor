#pragma once

#include <memory>
#include <ostream>
#include <string>

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
void readFile(FrameSet& frameSet, const std::string& filename);

// ::TODO remove when completed utsi2utms command line argument parsing"
void writeFile(const FrameSet& frameSet, std::ostream& file);

void writeFile(const FrameSet& frameSet, const std::string& filename);
}
}
}
