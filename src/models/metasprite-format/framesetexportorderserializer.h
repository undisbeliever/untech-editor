#ifndef _UNTECH_MODELS_METASPRITEFORMAT_FRAMESETEXPORTORDERSERIALIZER_H
#define _UNTECH_MODELS_METASPRITEFORMAT_FRAMESETEXPORTORDERSERIALIZER_H

#include <string>

namespace UnTech {
namespace MetaSpriteFormat {
namespace FrameSetExportOrder {

class ExportOrder;

// NOTE: FrameSet MUST be empty
void readFile(ExportOrder& exportOrder, const std::string& filename);

void writeFile(const ExportOrder& exportOrder, const std::string& filename);
}
}
}

#endif