#pragma once
#include "gui/controllers/metasprite.h"
#include <wx/slider.h>
#include <wx/tglbtn.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

// This dialog assumes it is executed modally
// And will be responsible for calling the controller's setcolor routine

class EditColorDialog : public wxDialog {
public:
    const static unsigned COLOR_MAX = 31;

public:
    EditColorDialog(wxWindow* parent,
                    MS::PaletteController& controller,
                    unsigned colorId);

    virtual int ShowModal() override;

private:
    void UpdateGui();

    void OnSliderChanged(wxCommandEvent&);
    void OnTextChanged(wxCommandEvent&);

private:
    MS::PaletteController& _controller;
    const unsigned _colorId;

    wxSlider* _red;
    wxTextCtrl* _redText;
    wxSlider* _green;
    wxTextCtrl* _greenText;
    wxSlider* _blue;
    wxTextCtrl* _blueText;

    wxToggleButton* _colorButton;
};
}
}
}
