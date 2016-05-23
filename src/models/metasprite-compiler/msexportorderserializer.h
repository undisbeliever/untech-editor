#pragma once

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
