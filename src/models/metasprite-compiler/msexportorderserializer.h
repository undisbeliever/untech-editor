#ifndef _UNTECH_MODELS_METASPRITEFORMAT_FRAMESETEXPORTORDERSERIALIZER_H
#define _UNTECH_MODELS_METASPRITEFORMAT_FRAMESETEXPORTORDERSERIALIZER_H

#include <string>

namespace UnTech {
namespace MetaSpriteCompiler {

class MsExportOrder;

namespace Serializer {

// NOTE: msExportOrder MUST be empty
void readFile(MsExportOrder& msExportOrder, const std::string& filename);

void writeFile(const MsExportOrder& msExportOrder, const std::string& filename);
}
}
}

#endif
