#include "abstractframesetpanel.h"
#include "gui/view/defaults.h"
#include "models/metasprite-common/framesetexportorder.h"

using namespace UnTech::View::MetaSpriteCommon;

AbstractFrameSetPanel::AbstractFrameSetPanel(wxWindow* parent, int wxWindowID,
                                             MSC::AbstractFrameSetController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(4, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _name = new NamedListNameCtrl(this, wxID_ANY,
                                  wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                  wxTE_PROCESS_ENTER);
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Name:")));
    grid->Add(_name, wxSizerFlags().Expand());

    _tilesetType = new EnumClassChoice<MSC::TilesetType>(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Tileset Type:")));
    grid->Add(_tilesetType, wxSizerFlags(1).Expand());

    _exportOrderFilename = new wxTextCtrl(this, wxID_ANY);
    _exportOrderFilename->Disable();
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Export Order:")));
    grid->Add(_exportOrderFilename, wxSizerFlags(1).Expand());

    _frameSetType = new wxTextCtrl(this, wxID_ANY);
    _frameSetType->Disable();
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("FrameSet Type:")));
    grid->Add(_frameSetType, wxSizerFlags(1).Expand());

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

    _tilesetType->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        _controller.selected_setTilesetType(_tilesetType->GetValue());
    });

    // ::TODO _exportOrderFilename::
}

void AbstractFrameSetPanel::UpdateGui()
{
    const MSC::AbstractFrameSet* frameSet = _controller.selected();

    if (frameSet) {
        this->Enable();

        _name->ChangeValue(frameSet->name());
        _tilesetType->SetValue(frameSet->tilesetType());

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
