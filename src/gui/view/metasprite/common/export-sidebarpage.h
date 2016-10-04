#pragma once
#include "exportordertreectrl.h"
#include "gui/controllers/metasprite-common.h"
#include "gui/view/common/textandtogglebuttonctrl.h"
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

    // This slot must be connected to:
    //   frame list changed signal
    //   frame item renamed signal
    //   frame list data changed signal
    auto& slot_frameNameChanged() { return _exportTree->slot_frameNameChanged(); }

private:
    MSC::AbstractFrameSetController& _controller;

    TextAndToggleButtonCtrl* _exportOrder;
    wxTextCtrl* _frameSetType;

    ExportOrderTreeCtrl* _exportTree;

    sigc::signal<void> _slot_frameNameChanged;
};
}
}
}
