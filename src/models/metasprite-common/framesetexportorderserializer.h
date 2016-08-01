#pragma once

#include <string>

namespace UnTech {
namespace MetaSpriteCommon {
namespace FrameSetExportOrder {

class ExportOrder;

// NOTE: FrameSet MUST be empty
void readFile(ExportOrder& exportOrder, const std::string& filename);

void writeFile(const ExportOrder& exportOrder, const std::string& filename);
}
}
}