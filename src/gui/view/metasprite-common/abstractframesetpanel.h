#pragma once
#include "gui/controllers/metasprite-common.h"
#include "gui/view/common/enumclasschoice.h"
#include "gui/view/common/namedlistnamectrl.h"
#include "gui/view/common/textandtogglebuttonctrl.h"
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
    TextAndToggleButtonCtrl* _exportOrder;
    wxTextCtrl* _frameSetType;
};
}
}
}
