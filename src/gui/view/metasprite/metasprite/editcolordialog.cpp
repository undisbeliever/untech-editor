#include "editcolordialog.h"
#include "gui/view/defaults.h"
#include <cassert>
#include <sigc++/connection.h>
#include <wx/colordlg.h>

using namespace UnTech;
using namespace UnTech::View::MetaSprite::MetaSprite;

// ::TODO move to Snes Namespace::

EditColorDialog::EditColorDialog(wxWindow* parent,
                                 MS::PaletteController& controller,
                                 unsigned colorId)
    : wxDialog(parent, wxID_ANY, "Edit Color",
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxCAPTION | wxCENTRE)
    , _controller(controller)
    , _colorId(colorId)
{
    assert(colorId < 16);

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    auto* hSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(hSizer, wxSizerFlags().Expand().DoubleBorder());

    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(3, 3, defBorder, defBorder * 2);
    hSizer->Add(grid, wxSizerFlags(3).Expand());

    grid->AddGrowableCol(1, 1);

    wxSize textSize(SMALL_TEXT_DIGITS_WIDTH, -1);

    _red = new wxSlider(this, wxID_ANY, 0, 0, COLOR_MAX);
    _redText = new wxTextCtrl(this, wxID_ANY,
                              wxEmptyString, wxDefaultPosition, textSize,
                              wxTE_PROCESS_ENTER);

    grid->Add(new wxStaticText(this, wxID_ANY, "Red:"), wxSizerFlags().Bottom());
    grid->Add(_red, wxSizerFlags().Expand());
    grid->Add(_redText);

    _green = new wxSlider(this, wxID_ANY, 0, 0, COLOR_MAX);
    _greenText = new wxTextCtrl(this, wxID_ANY,
                                wxEmptyString, wxDefaultPosition, textSize,
                                wxTE_PROCESS_ENTER);

    grid->Add(new wxStaticText(this, wxID_ANY, "Green:"), wxSizerFlags().Bottom());
    grid->Add(_green, wxSizerFlags().Expand());
    grid->Add(_greenText);

    _blue = new wxSlider(this, wxID_ANY, 0, 0, COLOR_MAX);
    _blueText = new wxTextCtrl(this, wxID_ANY,
                               wxEmptyString, wxDefaultPosition, textSize,
                               wxTE_PROCESS_ENTER);

    grid->Add(new wxStaticText(this, wxID_ANY, "Blue:"), wxSizerFlags().Bottom());
    grid->Add(_blue, wxSizerFlags().Expand());
    grid->Add(_blueText);

    _colorButton = new wxToggleButton(this, wxID_ANY, wxEmptyString);
    hSizer->Add(_colorButton, wxSizerFlags(1).Expand().DoubleBorder(wxLEFT));

    sizer->Add(CreateSeparatedButtonSizer(wxOK | wxCANCEL | wxNO_DEFAULT),
               wxSizerFlags().Expand().DoubleBorder());

    SetAutoLayout(true);
    sizer->Fit(this);

    UpdateGui();

    // EVENTS
    // ------
    this->Bind(wxEVT_SLIDER, &EditColorDialog::OnSliderChanged, this);
    this->Bind(wxEVT_TEXT, &EditColorDialog::OnTextChanged, this);

    this->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
        this->NavigateIn();
        this->NavigateIn();
    });

    this->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& event) {
        UpdateGui();
        event.Skip();
    });

    _colorButton->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
        const auto& palette = _controller.selected();

        if (_colorButton->GetValue() == true) {
            Snes::SnesColor c = palette.color(_colorId);

            wxColour wc(c.rgb().rgb());
            wc = wxGetColourFromUser(this, wc, "Select Color");

            if (wc.IsOk()) {
                c.setRgb(UnTech::rgba(wc.Red(), wc.Green(), wc.Blue()));

                _controller.selected_setColor(_colorId, c);
            }

            _colorButton->SetValue(false);
        }
    });
}

int EditColorDialog::ShowModal()
{
    auto& undoStack = _controller.baseController().undoStack();

    // build signal
    sigc::connection con = _controller.signal_dataChanged().connect(
        sigc::mem_fun(*this, &EditColorDialog::UpdateGui));

    int ret = wxDialog::ShowModal();

    if (ret == wxID_OK) {
        // commit color
        undoStack.dontMergeNextAction();
    }
    else {
        // restore color
        undoStack.undo();
    }

    // remove signal
    con.disconnect();

    return ret;
}

void EditColorDialog::UpdateGui()
{
    const auto& palette = _controller.selected();
    const Snes::SnesColor& c = palette.color(_colorId);

    _red->SetValue(c.red());
    _redText->ChangeValue(wxString::Format("%d", c.red()));
    _green->SetValue(c.green());
    _greenText->ChangeValue(wxString::Format("%d", c.green()));
    _blue->SetValue(c.blue());
    _blueText->ChangeValue(wxString::Format("%d", c.blue()));

    _colorButton->SetBackgroundColour(c.rgb().rgb());
}

void EditColorDialog::OnSliderChanged(wxCommandEvent&)
{
    const auto& palette = _controller.selected();
    Snes::SnesColor c = palette.color(_colorId);

    c.setRed(_red->GetValue());
    c.setGreen(_green->GetValue());
    c.setBlue(_blue->GetValue());

    _controller.selected_setColor(_colorId, c);
}

void EditColorDialog::OnTextChanged(wxCommandEvent& e)
{
    auto process = [&](wxTextCtrl* text, wxSlider* slider) {
        long v = 0;
        if (text->GetValue().ToLong(&v)) {
            if (v >= 0 && v < long(COLOR_MAX)) {
                slider->SetValue(v);
                OnSliderChanged(e);
            }
        }
    };

    if (e.GetEventObject() == _redText) {
        process(_redText, _red);
    }
    else if (e.GetEventObject() == _greenText) {
        process(_greenText, _green);
    }
    else if (e.GetEventObject() == _blueText) {
        process(_blueText, _blue);
    }
}
