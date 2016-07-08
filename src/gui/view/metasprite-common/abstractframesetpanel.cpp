#include "abstractframesetpanel.h"
#include "gui/view/defaults.h"
#include "models/metasprite-common/framesetexportorder.h"

using namespace UnTech::View::MetaSpriteCommon;

AbstractFrameSetPanel::AbstractFrameSetPanel(wxWindow* parent, int wxWindowID)
    : wxPanel(parent, wxWindowID)
{
    auto* grid = new wxFlexGridSizer(4, 2, DEFAULT_HGAP, DEFAULT_VGAP);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _name = new wxTextCtrl(this, wxID_ANY);
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
}
