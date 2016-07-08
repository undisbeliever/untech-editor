#pragma once
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSpriteCommon {

class AbstractFrameSetPanel : public wxPanel {
public:
    AbstractFrameSetPanel(wxWindow* parent, int wxWindowID);

private:
    wxTextCtrl* _name;
    wxChoice* _tilesetType;
    wxTextCtrl* _exportOrderFilename;
    wxTextCtrl* _frameSetType;
};
}
}
}
