#pragma once
#include "gui/controllers/metasprite.h"
#include <wx/tglbtn.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class PalettePanel : public wxPanel {
    constexpr static unsigned N_COLORS = 16;

    enum ColorID {
        ID_COLOR_0 = 2000,
        ID_COLOR_15 = 2015
    };

public:
    PalettePanel(wxWindow* parent, int wxWindowID,
                 MS::PaletteController& controller);

private:
    void UpdateGui();

    void on_colorToggled(wxCommandEvent&);
    void on_editColorToggled(wxCommandEvent&);

private:
    MS::PaletteController& _controller;

    wxToggleButton* _editColor;
    wxToggleButton* _colors[N_COLORS];
};
}
}
}
