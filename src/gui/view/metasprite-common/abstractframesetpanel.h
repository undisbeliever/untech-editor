#pragma once
#include "gui/controllers/metasprite-common.h"
#include "gui/view/common/namedlistnamectrl.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSpriteCommon {

namespace MSC = UnTech::MetaSpriteCommon;

class AbstractFrameSetPanel : public wxPanel {
public:
    AbstractFrameSetPanel(wxWindow* parent, int wxWindowID,
                          MSC::AbstractFrameSetController& controller);

private:
    void UpdateGui();

private:
    MSC::AbstractFrameSetController& _controller;

    NamedListNameCtrl* _name;
    wxChoice* _tilesetType;
    wxTextCtrl* _exportOrderFilename;
    wxTextCtrl* _frameSetType;
};
}
}
}
