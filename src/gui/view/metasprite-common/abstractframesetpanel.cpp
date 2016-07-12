#include "abstractframesetpanel.h"
#include "gui/view/defaults.h"
#include "models/metasprite-common/framesetexportorder.h"

using namespace UnTech::View::MetaSpriteCommon;

AbstractFrameSetPanel::AbstractFrameSetPanel(wxWindow* parent, int wxWindowID,
                                             MSC::AbstractFrameSetController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    auto* grid = new wxFlexGridSizer(4, 2, DEFAULT_HGAP, DEFAULT_VGAP);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _name = new wxTextCtrl(this, wxID_ANY,
                           wxEmptyString, wxDefaultPosition, wxDefaultSize,
                           wxTE_PROCESS_ENTER);
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Name:")));
    grid->Add(_name, 1, wxEXPAND);

    _tilesetType = new wxChoice(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Tileset Type:")));
    grid->Add(_tilesetType, 1, wxEXPAND);

    _exportOrderFilename = new wxTextCtrl(this, wxID_ANY);
    _exportOrderFilename->Disable();
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Export Order:")));
    grid->Add(_exportOrderFilename, 1, wxEXPAND);

    _frameSetType = new wxTextCtrl(this, wxID_ANY);
    _frameSetType->Disable();
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("FrameSet Type:")));
    grid->Add(_frameSetType, 1, wxEXPAND);

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &AbstractFrameSetPanel::UpdateGui));

    _controller.signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &AbstractFrameSetPanel::UpdateGui)));

    // Events
    // ------
    _name->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
        _controller.selected_setName(_name->GetValue().ToStdString());
        this->NavigateIn();
    });
    _name->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
        _controller.selected_setName(_name->GetValue().ToStdString());
        e.Skip();
    });

    // ::TODO _tilesetType::
    // ::TODO _exportOrderFilename::
}

void AbstractFrameSetPanel::UpdateGui()
{
    const MSC::AbstractFrameSet* frameSet = _controller.selected();

    if (frameSet) {
        this->Enable();

        _name->ChangeValue(frameSet->name());
        _tilesetType->SetSelection(0); // ::TODO::

        const auto& fseoDocument = frameSet->exportOrderDocument();
        if (fseoDocument) {
            _exportOrderFilename->ChangeValue(fseoDocument->filename());
            _frameSetType->ChangeValue(fseoDocument->exportOrder().name());
        }
        else {
            _exportOrderFilename->ChangeValue("");
            _frameSetType->ChangeValue("");
        }
    }
    else {
        this->Disable();

        _name->ChangeValue("");
        _tilesetType->SetSelection(wxNOT_FOUND);

        _exportOrderFilename->ChangeValue("");
        _frameSetType->ChangeValue("");
    }
}
