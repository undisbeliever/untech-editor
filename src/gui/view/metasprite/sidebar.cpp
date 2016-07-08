#include "sidebar.h"
#include "palettepanel.h"
#include "gui/view/common/ms8aabb.h"
#include "gui/view/defaults.h"
#include "gui/view/metasprite-common/abstractframesetpanel.h"

#include <wx/spinctrl.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

class FramePanel : public wxPanel {
public:
    FramePanel(wxWindow* parent, int wxWindowID,
               MS::FrameController& controller);

private:
    void UpdateGui();

private:
    MS::FrameController& _controller;

    wxCheckBox* _solid;
    Ms8RectCtrl* _tileHitbox;
};

class FrameObjectPanel : public wxPanel {
public:
    FrameObjectPanel(wxWindow* parent, int wxWindowID,
                     MS::FrameObjectController& controller);

private:
    void UpdateGui();

private:
    MS::FrameObjectController& _controller;

    Ms8PointCtrl* _location;
    wxSpinCtrl* _tileId;
    wxChoice* _size;
    wxSpinCtrl* _order;
    wxCheckBox* _hFlip;
    wxCheckBox* _vFlip;
};

class ActionPointPanel : public wxPanel {
public:
    ActionPointPanel(wxWindow* parent, int wxWindowID,
                     MS::ActionPointController& controller);

private:
    void UpdateGui();

private:
    MS::ActionPointController& _controller;

    Ms8PointCtrl* _location;
    wxSpinCtrl* _parameter;
};

class EntityHitboxPanel : public wxPanel {
public:
    EntityHitboxPanel(wxWindow* parent, int wxWindowID,
                      MS::EntityHitboxController& controller);

private:
    void UpdateGui();

private:
    MS::EntityHitboxController& _controller;

    Ms8RectCtrl* _aabb;
    wxSpinCtrl* _parameter;
};
}
}
}

using namespace UnTech::View::MetaSprite;
using namespace UnTech::View::MetaSpriteCommon;

// SIDEBAR
// =======

Sidebar::Sidebar(wxWindow* parent, int wxWindowID,
                 MS::MetaSpriteController& controller)
    : wxNotebook(parent, wxWindowID)
{
    this->SetSizeHints(SIDEBAR_WIDTH, -1);

    // FrameSet Panel
    {
        auto* panel = new wxPanel(this);

        auto* sizer = new wxBoxSizer(wxVERTICAL);
        panel->SetSizer(sizer);

        auto* afPanel = new AbstractFrameSetPanel(panel, wxID_ANY, controller.abstractFrameSetController());
        sizer->Add(afPanel, 0, wxEXPAND | wxALL, DEFAULT_BORDER);

        auto* palSizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Palette");
        sizer->Add(palSizer, 1, wxEXPAND | wxALL, DEFAULT_BORDER);

        // ::TODO list::
        auto* paletteList = new wxPanel(panel);
        palSizer->Add(paletteList, 1, wxEXPAND | wxALL, DEFAULT_BORDER);

        auto* palPanel = new PalettePanel(panel, wxID_ANY, controller.paletteController());
        palSizer->Add(palPanel, 0, wxEXPAND | wxALL, DEFAULT_BORDER);

        this->AddPage(panel, "FrameSet");
    }

    // Frame Panel
    {
        auto* framePanel = new wxPanel(this);

        auto* frameSizer = new wxBoxSizer(wxVERTICAL);
        framePanel->SetSizer(frameSizer);

        // ::TODO list::
        auto* frameList = new wxPanel(framePanel);
        frameSizer->Add(frameList, 1, wxEXPAND | wxALL, DEFAULT_BORDER);

        auto* frameNotepad = new wxNotebook(framePanel, wxID_ANY);
        frameSizer->Add(frameNotepad, 1, wxEXPAND | wxALL, DEFAULT_BORDER);

        frameNotepad->AddPage(
            new FramePanel(frameNotepad, wxID_ANY, controller.frameController()),
            "Frame");

        {
            auto* panel = new wxPanel(frameNotepad);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            // ::TODO list::
            auto* list = new wxPanel(this);
            sizer->Add(list, 1, wxEXPAND | wxALL, DEFAULT_BORDER);

            auto* editor = new FrameObjectPanel(panel, wxID_ANY, controller.frameObjectController());
            sizer->Add(editor, 0, wxEXPAND | wxALL, DEFAULT_BORDER);

            frameNotepad->AddPage(panel, "Objects");
        }

        {
            auto* panel = new wxPanel(frameNotepad);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            // ::TODO list::
            auto* list = new wxPanel(this);
            sizer->Add(list, 1, wxEXPAND | wxALL, DEFAULT_BORDER);

            auto* editor = new ActionPointPanel(panel, wxID_ANY, controller.actionPointController());
            sizer->Add(editor, 0, wxEXPAND | wxALL, DEFAULT_BORDER);

            frameNotepad->AddPage(panel, "Action Points");
        }

        {
            auto* panel = new wxPanel(frameNotepad);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            // ::TODO list::
            auto* list = new wxPanel(this);
            sizer->Add(list, 1, wxEXPAND | wxALL, DEFAULT_BORDER);

            auto* editor = new EntityHitboxPanel(panel, wxID_ANY, controller.entityHitboxController());
            sizer->Add(editor, 0, wxEXPAND | wxALL, DEFAULT_BORDER);

            frameNotepad->AddPage(panel, "Hitboxes");
        }

        this->AddPage(framePanel, "Frame");
    }
}

// FRAME
// =====

FramePanel::FramePanel(wxWindow* parent, int wxWindowID,
                       MS::FrameController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    auto* grid = new wxFlexGridSizer(2, 2, DEFAULT_HGAP, DEFAULT_VGAP);
    sizer->Add(grid, 1, wxEXPAND | wxALL, DEFAULT_BORDER);

    grid->AddGrowableCol(1, 1);

    _solid = new wxCheckBox(this, wxID_ANY, "Solid");
    grid->Add(_solid, 1);
    grid->Add(new wxPanel(this, wxID_ANY));

    _tileHitbox = new Ms8RectCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Tile Hitbox:"));
    grid->Add(_tileHitbox, 1, wxEXPAND);

    UpdateGui();
}

void FramePanel::UpdateGui()
{
    const MS::Frame* frame = _controller.selected();

    if (frame) {
        _tileHitbox->SetValue(frame->tileHitbox());

        _solid->SetValue(frame->solid());
        _tileHitbox->Enable(frame->solid());

        this->Enable();
    }
    else {
        static const ms8rect zeroRect(0, 0, 0, 0);

        _solid->SetValue(false);
        _tileHitbox->SetValue(zeroRect);
        this->Disable();
    }
}

// FRAME OBJECTS
// =============

FrameObjectPanel::FrameObjectPanel(wxWindow* parent, int wxWindowID,
                                   MS::FrameObjectController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    auto* grid = new wxFlexGridSizer(5, 2, DEFAULT_HGAP, DEFAULT_VGAP);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _location = new Ms8PointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Location:"));
    grid->Add(_location, 1, wxEXPAND);

    _tileId = new wxSpinCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Tile Id:"));
    grid->Add(_tileId, 1, wxEXPAND);

    wxArrayString sizeChoices;
    sizeChoices.Add("Small");
    sizeChoices.Add("Large");

    _size = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sizeChoices);
    grid->Add(new wxStaticText(this, wxID_ANY, "Size:"));
    grid->Add(_size, 1, wxEXPAND);

    _order = new wxSpinCtrl(this, wxID_ANY);
    _order->SetRange(0, 3);
    grid->Add(new wxStaticText(this, wxID_ANY, "Order:"));
    grid->Add(_order, 1, wxEXPAND);

    _hFlip = new wxCheckBox(this, wxID_ANY, "hFlip");
    _vFlip = new wxCheckBox(this, wxID_ANY, "vFlip");

    auto* box = new wxBoxSizer(wxHORIZONTAL);
    box->Add(_hFlip, 1, wxEXPAND, 0);
    box->Add(_vFlip, 1, wxEXPAND, 0);
    grid->Add(new wxPanel(this, wxID_ANY));
    grid->Add(box, 1, wxEXPAND);

    UpdateGui();
}

void FrameObjectPanel::UpdateGui()
{
    typedef UnTech::MetaSprite::FrameObject::ObjectSize OS;

    const MS::FrameObject* obj = _controller.selected();

    if (obj) {
        const MS::FrameSet& frameSet = obj->frame().frameSet();

        if (obj->size() == MS::FrameObject::ObjectSize::SMALL) {
            _tileId->SetRange(0, frameSet.smallTileset().size() - 1);
        }
        else {
            _tileId->SetRange(0, frameSet.largeTileset().size() - 1);
        }

        _location->SetValue(obj->location());
        _tileId->SetValue(obj->tileId());
        _size->SetSelection(obj->size() == OS::LARGE ? 1 : 0);
        _order->SetValue(obj->order());
        _hFlip->SetValue(obj->hFlip());
        _vFlip->SetValue(obj->hFlip());

        this->Enable();
    }
    else {
        static const ms8point zeroPoint(0, 0);

        _location->SetValue(zeroPoint);
        _tileId->SetValue(0);
        _size->SetSelection(wxNOT_FOUND);
        _order->SetValue(0);
        _hFlip->SetValue(false);
        _vFlip->SetValue(false);

        this->Disable();
    }
}

// ACTION POINTS
// =============

ActionPointPanel::ActionPointPanel(wxWindow* parent, int wxWindowID,
                                   MS::ActionPointController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    auto* grid = new wxFlexGridSizer(2, 2, DEFAULT_HGAP, DEFAULT_VGAP);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _location = new Ms8PointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Location:"));
    grid->Add(_location, 1, wxEXPAND);

    // ::TODO replace with something better::
    _parameter = new wxSpinCtrl(this, wxID_ANY);
    _parameter->SetRange(0, 255);
    grid->Add(new wxStaticText(this, wxID_ANY, "Parameter:"));
    grid->Add(_parameter, 1, wxEXPAND);

    UpdateGui();
}

void ActionPointPanel::UpdateGui()
{
    const MS::ActionPoint* ap = _controller.selected();

    if (ap) {
        _location->SetValue(ap->location());
        _parameter->SetValue(ap->parameter());

        this->Enable();
    }
    else {
        static const ms8point zeroPoint(0, 0);

        _location->SetValue(zeroPoint);
        _parameter->SetValue(0);

        this->Disable();
    }
}

// ENTITY HITBOXES
// ===============

EntityHitboxPanel::EntityHitboxPanel(wxWindow* parent, int wxWindowID,
                                     MS::EntityHitboxController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    auto* grid = new wxFlexGridSizer(2, 2, DEFAULT_HGAP, DEFAULT_VGAP);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _aabb = new Ms8RectCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Location:"));
    grid->Add(_aabb, 1, wxEXPAND);

    // ::TODO replace with something better::
    _parameter = new wxSpinCtrl(this, wxID_ANY);
    _parameter->SetRange(0, 255);
    grid->Add(new wxStaticText(this, wxID_ANY, "Parameter:"));
    grid->Add(_parameter, 1, wxEXPAND);

    UpdateGui();
}

void EntityHitboxPanel::UpdateGui()
{
    const MS::EntityHitbox* eh = _controller.selected();

    if (eh) {
        _aabb->SetValue(eh->aabb());
        _parameter->SetValue(eh->parameter());

        this->Enable();
    }
    else {
        static const ms8rect zeroRect(0, 0, 0, 0);

        _aabb->SetValue(zeroRect);
        _parameter->SetValue(0);

        this->Disable();
    }
}
