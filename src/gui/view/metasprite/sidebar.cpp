#include "sidebar.h"
#include "palettelist.h"
#include "palettepanel.h"
#include "sidebar-lists.hpp"
#include "gui/view/common/ms8aabb.h"
#include "gui/view/defaults.h"
#include "gui/view/metasprite-common/abstractframesetpanel.h"
#include "gui/view/metasprite-common/animation-sidebarpage.h"
#include "gui/view/metasprite-common/export-sidebarpage.h"
#include "models/common/string.h"
#include <wx/spinctrl.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

static const ms8point emptyPoint(0, 0);
static const ms8rect emptyRect(0, 0, 1, 1);

typedef MS::MetaSpriteController::SelectedTypeController::Type SelectedType;

enum class SidebarPages {
    FRAMESET_PAGE,
    FRAME_PAGE,
    ANIMATION_PAGE,
    EXPORT_PAGE
};
enum class FramePages {
    FRAME_PAGE,
    FRAME_OBJECT_PAGE,
    ACTION_POINT_PAGE,
    ENTITY_HITBOX_PAGE
};

class FramePanel : public wxPanel {
public:
    FramePanel(wxWindow* parent, int wxWindowID,
               MS::MetaSpriteController& controller);

private:
    void UpdateGui();

private:
    MS::FrameController& _controller;
    MS::MetaSpriteController& _msController;

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
        sizer->Add(afPanel, wxSizerFlags().Expand().Border());

        auto* palSizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Palette");
        sizer->Add(palSizer, wxSizerFlags(1).Expand().Border());

        palSizer->Add(new OrderedListToolBar<MS::Palette>(
                          panel, wxID_ANY,
                          controller.paletteController()),
                      wxSizerFlags(0).Right().Border());

        palSizer->Add(new PaletteListCtrl(
                          panel, wxID_ANY,
                          controller.paletteController()),
                      wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

        palSizer->Add(new PalettePanel(
                          panel, wxID_ANY,
                          controller.paletteController()),
                      wxSizerFlags(0).Expand().Border());

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
                        wxSizerFlags(0).Right().Border());

        frameSizer->Add(new NamedListCtrl<MS::Frame>(
                            framePanel, wxID_ANY,
                            controller.frameController()),
                        wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

        _frameNotebook = new wxNotebook(framePanel, wxID_ANY);
        frameSizer->Add(_frameNotebook, wxSizerFlags(1).Expand().Border());

        _frameNotebook->AddPage(
            new FramePanel(_frameNotebook, wxID_ANY, controller),
            "Frame");

        {
            auto* panel = new wxPanel(_frameNotebook);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new OrderedListToolBar<MS::FrameObject>(
                           panel, wxID_ANY,
                           controller.frameObjectController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new OrderedListCtrl<MS::FrameObject>(
                           panel, wxID_ANY,
                           controller.frameObjectController()),
                       wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

            sizer->Add(new FrameObjectPanel(
                           panel, wxID_ANY,
                           controller),
                       wxSizerFlags(0).Expand().Border());

            _frameNotebook->AddPage(panel, "Objects");
        }

        {
            auto* panel = new wxPanel(_frameNotebook);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new OrderedListToolBar<MS::ActionPoint>(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new OrderedListCtrl<MS::ActionPoint>(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

            sizer->Add(new ActionPointPanel(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       wxSizerFlags(0).Expand().Border());

            _frameNotebook->AddPage(panel, "Action Points");
        }

        {
            auto* panel = new wxPanel(_frameNotebook);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new OrderedListToolBar<MS::EntityHitbox>(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new OrderedListCtrl<MS::EntityHitbox>(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

            sizer->Add(new EntityHitboxPanel(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       wxSizerFlags(0).Expand().Border());

            _frameNotebook->AddPage(panel, "Hitboxes");
        }

        this->AddPage(framePanel, "Frame");

        this->AddPage(
            new AnimationSidebarPage(this, wxID_ANY,
                                     controller.abstractFrameSetController()),
            "Animations");

        auto* exportPage = new ExportSidebarPage(this, wxID_ANY,
                                                 controller.abstractFrameSetController());
        this->AddPage(exportPage, "Export");

        // Signals
        // -------
        controller.frameController().signal_listChanged().connect(
            exportPage->slot_frameNameChanged());

        controller.frameController().signal_listDataChanged().connect(sigc::hide(
            exportPage->slot_frameNameChanged()));

        controller.frameController().signal_itemRenamed().connect(sigc::hide(
            exportPage->slot_frameNameChanged()));

        controller.selectedTypeController().signal_selectedChanged().connect([this](void) {
            const auto type = _controller.selectedTypeController().type();

            if (type != SelectedType::NONE) {
                this->SetSelection(int(SidebarPages::FRAME_PAGE));

                switch (type) {
                case SelectedType::NONE:
                    break;

                case SelectedType::FRAME_OBJECT:
                    _frameNotebook->SetSelection(int(FramePages::FRAME_OBJECT_PAGE));
                    break;

                case SelectedType::ACTION_POINT:
                    _frameNotebook->SetSelection(int(FramePages::ACTION_POINT_PAGE));
                    break;

                case SelectedType::ENTITY_HITBOX:
                    _frameNotebook->SetSelection(int(FramePages::ENTITY_HITBOX_PAGE));
                    break;

                case SelectedType::TILE_HITBOX:
                    _frameNotebook->SetSelection(int(FramePages::FRAME_PAGE));
                    break;
                }
            }
        });
    }
}

// FRAME
// =====

FramePanel::FramePanel(wxWindow* parent, int wxWindowID,
                       MS::MetaSpriteController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller.frameController())
    , _msController(controller)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(2, 2, defBorder, defBorder * 2);
    sizer->Add(grid, wxSizerFlags(1).Expand().Border());

    grid->AddGrowableCol(1, 1);

    _solid = new wxCheckBox(this, wxID_ANY, "Solid");
    grid->Add(_solid, wxSizerFlags(1));
    grid->Add(new wxPanel(this, wxID_ANY));

    _tileHitbox = new Ms8RectCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Tile Hitbox:"));
    grid->Add(_tileHitbox, wxSizerFlags(1).Expand());

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
        _msController.selectedTypeController().selectTileHitbox();
    });

    _tileHitbox->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        _msController.selectedTypeController().selectTileHitbox();
        e.Skip();
    });
    _tileHitbox->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setTileHitbox_merge(_tileHitbox->GetValue());
    });
}

void FramePanel::UpdateGui()
{
    const MS::Frame* frame = _controller.selected();

    if (frame) {
        _solid->SetValue(frame->solid());
        _tileHitbox->Enable(frame->solid());
        _tileHitbox->SetValue(frame->tileHitbox());

        this->Enable();
    }
    else {
        _solid->SetValue(false);
        _tileHitbox->SetValue(emptyRect);

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
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(5, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _location = new Ms8PointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Location:"));
    grid->Add(_location, wxSizerFlags(1).Expand());

    _tileId = new wxSpinCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Tile Id:"));
    grid->Add(_tileId, wxSizerFlags(1).Expand());

    wxArrayString sizeChoices;
    sizeChoices.Add("Small");
    sizeChoices.Add("Large");

    _size = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sizeChoices);
    grid->Add(new wxStaticText(this, wxID_ANY, "Size:"));
    grid->Add(_size, wxSizerFlags(1).Expand());

    _order = new wxSpinCtrl(this, wxID_ANY);
    _order->SetRange(0, 3);
    grid->Add(new wxStaticText(this, wxID_ANY, "Order:"));
    grid->Add(_order, wxSizerFlags(1).Expand());

    _hFlip = new wxCheckBox(this, wxID_ANY, "hFlip");
    _vFlip = new wxCheckBox(this, wxID_ANY, "vFlip");

    auto* box = new wxBoxSizer(wxHORIZONTAL);
    box->Add(_hFlip, wxSizerFlags(1).Expand());
    box->Add(_vFlip, wxSizerFlags(1).Expand());
    grid->Add(new wxPanel(this, wxID_ANY));
    grid->Add(box, wxSizerFlags(1).Expand());

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
    _location->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _location->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setLocation_merge(_location->GetValue());
    });

    _tileId->Bind(wxEVT_SET_FOCUS, [this](wxFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
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
        _location->SetValue(emptyPoint);
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
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(2, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _location = new Ms8PointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Location:"));
    grid->Add(_location, wxSizerFlags(1).Expand());

    // ::TODO replace with something better::
    _parameter = new wxSpinCtrl(this, wxID_ANY);
    _parameter->SetRange(0, 255);
    grid->Add(new wxStaticText(this, wxID_ANY, "Parameter:"));
    grid->Add(_parameter, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGui));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGui));

    // Events
    // ------
    _location->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _location->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setLocation_merge(_location->GetValue());
    });

    _parameter->Bind(wxEVT_SET_FOCUS, [this](wxFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
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
        _location->SetValue(emptyPoint);
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
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(2, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _aabb = new Ms8RectCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "AABB:"));
    grid->Add(_aabb, wxSizerFlags(1).Expand());

    // ::TODO replace with something better::
    _parameter = new wxSpinCtrl(this, wxID_ANY);
    _parameter->SetRange(0, 255);
    grid->Add(new wxStaticText(this, wxID_ANY, "Parameter:"));
    grid->Add(_parameter, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGui));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGui));

    // Events
    // ------
    _aabb->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _aabb->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setAabb_merge(_aabb->GetValue());
    });

    _parameter->Bind(wxEVT_SET_FOCUS, [this](wxFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
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
        _aabb->SetValue(emptyRect);
        _parameter->SetValue(0);

        this->Disable();
    }
}
