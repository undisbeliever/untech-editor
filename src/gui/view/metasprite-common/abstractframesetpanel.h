#pragma once
#include "gui/controllers/metasprite-common.h"
#include "gui/view/common/enumclasschoice.h"
#include "gui/view/common/namedlistnamectrl.h"
#include <wx/tglbtn.h>
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
    EnumClassChoice<MSC::TilesetType>* _tilesetType;
    wxTextCtrl* _exportOrderFilename;
    wxToggleButton* _exportOrderButton;
    wxTextCtrl* _frameSetType;
};
}
}
}
