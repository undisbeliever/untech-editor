#pragma once
#include "gui/controllers/sprite-importer.h"
#include <wx/notebook.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class Sidebar : public wxNotebook {
public:
    Sidebar(wxWindow* parent, int wxWindowID,
            SI::SpriteImporterController& controller);
};
}
}
}
