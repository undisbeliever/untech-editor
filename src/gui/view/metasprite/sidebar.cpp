#include "sidebar.h"
#include "palettepanel.h"
#include "sidebar-lists.hpp"
#include "gui/view/common/ms8aabb.h"
#include "gui/view/defaults.h"
#include "gui/view/metasprite-common/abstractframesetpanel.h"
#include "models/common/string.h"
#include <wx/spinctrl.h>

// ::TODO call _controller.dontMergeNextAction on wxSpinCtrl focus out::

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
                     MS::MetaSpriteController& controller);

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
    , _controller(controller)
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

        frameSizer->Add(new NamedListToolBar<MS::Frame>(
                            framePanel, wxID_ANY,
                            controller.frameController()),
                        0, wxALIGN_RIGHT | wxALL, DEFAULT_BORDER);

        frameSizer->Add(new NamedListCtrl<MS::Frame>(
                            framePanel, wxID_ANY,
                            controller.frameController()),
                        1, wxEXPAND | wxLEFT | wxRIGHT, DEFAULT_BORDER);

        auto* frameNotepad = new wxNotebook(framePanel, wxID_ANY);
        frameSizer->Add(frameNotepad, 1, wxEXPAND | wxALL, DEFAULT_BORDER);

        frameNotepad->AddPage(
            new FramePanel(frameNotepad, wxID_ANY, controller.frameController()),
            "Frame");

        {
            auto* panel = new wxPanel(frameNotepad);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new OrderedListToolBar<MS::FrameObject>(
                           panel, wxID_ANY,
                           controller.frameObjectController()),
                       0, wxALIGN_RIGHT | wxALL, DEFAULT_BORDER);

            sizer->Add(new OrderedListCtrl<MS::FrameObject>(
                           panel, wxID_ANY,
                           controller.frameObjectController()),
                       1, wxEXPAND | wxLEFT | wxRIGHT, DEFAULT_BORDER);

            sizer->Add(new FrameObjectPanel(
                           panel, wxID_ANY,
                           controller),
                       0, wxEXPAND | wxALL, DEFAULT_BORDER);

            frameNotepad->AddPage(panel, "Objects");
        }

        {
            auto* panel = new wxPanel(frameNotepad);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new OrderedListToolBar<MS::ActionPoint>(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       0, wxALIGN_RIGHT | wxALL, DEFAULT_BORDER);

            sizer->Add(new OrderedListCtrl<MS::ActionPoint>(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       1, wxEXPAND | wxLEFT | wxRIGHT, DEFAULT_BORDER);

            sizer->Add(new ActionPointPanel(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       0, wxEXPAND | wxALL, DEFAULT_BORDER);

            frameNotepad->AddPage(panel, "Action Points");
        }

        {
            auto* panel = new wxPanel(frameNotepad);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new OrderedListToolBar<MS::EntityHitbox>(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       0, wxALIGN_RIGHT | wxALL, DEFAULT_BORDER);

            sizer->Add(new OrderedListCtrl<MS::EntityHitbox>(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       1, wxEXPAND | wxLEFT | wxRIGHT, DEFAULT_BORDER);

            sizer->Add(new EntityHitboxPanel(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       0, wxEXPAND | wxALL, DEFAULT_BORDER);

            frameNotepad->AddPage(panel, "Hitboxes");
        }

        this->AddPage(framePanel, "Frame");
    }

    // EVENTS
    // ------

    // BUGFIX: Force a GUI update on tab change
    //
    // Tab ordering is broken by a wxWindow->Disable() call
    // in UpdateGui, which is called by the sidebar/list/toolbar
    // constructors.
    //
    // Other attempts to fix this have failed.
    // Currently tab order is still broken even if I:
    //      * Call `UpdateGui` in constructor
    //      * Call `UpdateGui` or `emitAllDataChanged` after show
    //      * Call `emitAllDataChanged` in wxEVT_SHOW
    //      * Call `emitAllDataChanged` in wxEVT_IDLE
    //      * Change tab order in `UpdateGui`
    //
    // Ironically, after the wxFrame is shown and fully rendered,
    // calls to Disable/Enable do not break tab ordering.
    //
    // This hack will get the controller to update the GUI on every
    // tab change. Users will not notice because the broken widgets are
    // on the second wxNotebook page.
    this->Bind(wxEVT_NOTEBOOK_PAGE_CHANGING, [this](wxBookCtrlEvent&) {
        _controller.emitAllDataChanged();
    });
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

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FramePanel::UpdateGui));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FramePanel::UpdateGui));

    // Events
    // ------
    _solid->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _controller.selected_setSolid(_solid->GetValue());
    });

    _tileHitbox->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setTileHitbox_merge(_tileHitbox->GetValue());
    });
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
                                   MS::MetaSpriteController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller.frameObjectController())
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

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGui));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGui));

    controller.frameSetController().signal_tileCountChanged().connect(
        [this](const MS::FrameSet* frameSet) {
            const MS::FrameObject* obj = _controller.selected();

            if (obj && frameSet == &obj->frame().frameSet()) {
                assert(frameSet != nullptr);

                if (obj->size() == MS::FrameObject::ObjectSize::SMALL) {
                    _tileId->SetRange(0, frameSet->smallTileset().size() - 1);
                }
                else {
                    _tileId->SetRange(0, frameSet->largeTileset().size() - 1);
                }
            }
        });

    // Events
    // ------
    _location->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setLocation_merge(_location->GetValue());
    });

    _tileId->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setTileId_merge(_tileId->GetValue());
    });

    _size->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        typedef UnTech::MetaSprite::FrameObject::ObjectSize OS;
        _controller.selected_setSize(_size->GetSelection() == 1 ? OS::LARGE : OS::SMALL);
    });

    _order->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setOrder(_order->GetValue());
    });

    _hFlip->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _controller.selected_setHFlip(_hFlip->GetValue());
    });

    _vFlip->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _controller.selected_setVFlip(_vFlip->GetValue());
    });
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
        _vFlip->SetValue(obj->vFlip());

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

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGui));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGui));

    // Events
    // ------
    _location->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setLocation_merge(_location->GetValue());
    });

    _parameter->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setParameter_merge(_parameter->GetValue());
    });
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
    grid->Add(new wxStaticText(this, wxID_ANY, "AABB:"));
    grid->Add(_aabb, 1, wxEXPAND);

    // ::TODO replace with something better::
    _parameter = new wxSpinCtrl(this, wxID_ANY);
    _parameter->SetRange(0, 255);
    grid->Add(new wxStaticText(this, wxID_ANY, "Parameter:"));
    grid->Add(_parameter, 1, wxEXPAND);

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGui));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGui));

    // Events
    // ------
    _aabb->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setAabb_merge(_aabb->GetValue());
    });

    _parameter->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setParameter_merge(_parameter->GetValue());
    });
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
