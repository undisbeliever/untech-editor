#pragma once
#include "gui/controllers/metasprite-common.h"
#include "gui/view/common/textandtogglebuttonctrl.h"
#include <wx/treectrl.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSpriteCommon {

namespace MSC = UnTech::MetaSpriteCommon;

class ExportSidebarPage : public wxPanel {
public:
    ExportSidebarPage(wxWindow* parent, int wxWindowID,
                      MSC::AbstractFrameSetController& controller);

    void UpdateGui();
    void BuildGuiTree();

private:
    MSC::AbstractFrameSetController& _controller;

    TextAndToggleButtonCtrl* _exportOrder;
    wxTextCtrl* _frameSetType;

    wxTreeCtrl* _exportTree;
};
}
}
}
