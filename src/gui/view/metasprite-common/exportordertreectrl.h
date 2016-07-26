#pragma once
#include "gui/controllers/metasprite-common.h"
#include <wx/treectrl.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSpriteCommon {

namespace MSC = UnTech::MetaSpriteCommon;

class ExportOrderTreeCtrl : public wxTreeCtrl {
public:
    ExportOrderTreeCtrl(wxWindow* parent, int wxWindowID,
                        MSC::AbstractFrameSetController& controller);

    void BuildTree();
    void UpdateTreeFrames();
    void UpdateTreeAnimations();

    // This slot must be connected to:
    //   frame list changed signal
    //   frame item renamed signal
    //   frame list data changed signal
    auto& slot_frameNameChanged() { return _slot_frameNameChanged; }

private:
    MSC::AbstractFrameSetController& _controller;

    sigc::signal<void> _slot_frameNameChanged;
};
}
}
}
