#include "sidebar.h"
#include "palettelist.h"
#include "palettepanel.h"
#include "sidebar-lists.hpp"
#include "gui/view/common/enumclasschoice.h"
#include "gui/view/common/ms8aabb.h"
#include "gui/view/defaults.h"
#include "gui/view/metasprite/animation/animation-sidebarpage.h"
#include "gui/view/metasprite/common/export-sidebarpage.hpp"
#include "gui/view/metasprite/common/framesetcommonpanel.hpp"
#include "models/common/string.h"
#include <wx/spinctrl.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

using SelectedType = UnTech::MetaSprite::SelectedType;

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
    void UpdateGuiRange();

private:
    MS::FrameObjectController& _controller;
    MS::MetaSpriteController& _msController;

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
    EnumClassChoice<UnTech::MetaSprite::EntityHitboxType>* _hitboxType;
};
}
}
}
}

using namespace UnTech::View::MetaSprite::MetaSprite;
using namespace UnTech::View::MetaSprite::Common;

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

        auto* afPanel = new FrameSetCommonPanel<MS::FrameSetController>(
            panel, wxID_ANY, controller.frameSetController());
        sizer->Add(afPanel, wxSizerFlags().Expand().Border());

        auto* palSizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Palette");
        sizer->Add(palSizer, wxSizerFlags(1).Expand().Border());

        palSizer->Add(new VectorListToolBar<MS::PaletteController>(
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

        frameSizer->Add(new IdMapListToolBar<MS::FrameController>(
                            framePanel, wxID_ANY,
                            controller.frameController()),
                        wxSizerFlags(0).Right().Border());

        frameSizer->Add(new IdMapListCtrl<MS::FrameController>(
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

            sizer->Add(new VectorListToolBar<MS::FrameObjectController>(
                           panel, wxID_ANY,
                           controller.frameObjectController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new VectorListCtrl<MS::FrameObjectController>(
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

            sizer->Add(new VectorListToolBar<MS::ActionPointController>(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new VectorListCtrl<MS::ActionPointController>(
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

            sizer->Add(new VectorListToolBar<MS::EntityHitboxController>(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new VectorListCtrl<MS::EntityHitboxController>(
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
            new Animation::AnimationSidebarPage(
                this, wxID_ANY,
                controller.animationControllerInterface()),
            "Animations");

        auto* exportPage = new ExportSidebarPage<MS::MetaSpriteController>(
            this, wxID_ANY, controller);
        this->AddPage(exportPage, "Export");

        // Signals
        // -------

        controller.selectedController().signal_selectedChanged().connect([this](void) {
            const auto type = _controller.selectedController().type();

            if (type != SelectedType::NONE) {
                this->SetSelection(int(SidebarPages::FRAME_PAGE));

                switch (type) {
                case SelectedType::NONE:
                case SelectedType::FRAME:
                    break;

                case SelectedType::TILE_HITBOX:
                    _frameNotebook->SetSelection(int(FramePages::FRAME_PAGE));
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
                }
            }
        });

        _controller.frameSetController().signal_selectedChanged().connect([this](void) {
            this->Enable(_controller.frameSetController().hasSelected());
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
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &FramePanel::UpdateGui));

    // Events
    // ------
    _solid->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _controller.selected_setSolid(_solid->GetValue());
        _msController.selectedController().selectTileHitbox();
    });

    _tileHitbox->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setTileHitbox(_tileHitbox->GetValue());
        _msController.selectedController().selectTileHitbox();
    });
}

void FramePanel::UpdateGui()
{
    this->Enable(_controller.hasSelected());

    const MS::Frame& frame = _controller.selected();

    _solid->SetValue(frame.solid);
    _tileHitbox->Enable(frame.solid);
    _tileHitbox->SetValue(frame.tileHitbox);
}

// FRAME OBJECTS
// =============

FrameObjectPanel::FrameObjectPanel(wxWindow* parent, int wxWindowID,
                                   MS::MetaSpriteController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller.frameObjectController())
    , _msController(controller)
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
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGui));

    _msController.frameSetController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGui));

    // Events
    // ------
    _location->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setLocation(_location->GetValue());
    });

    _tileId->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setTileId(_tileId->GetValue());
    });

    _size->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        typedef UnTech::MetaSprite::ObjectSize OS;
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
    typedef UnTech::MetaSprite::ObjectSize OS;

    this->Enable(_controller.hasSelected());

    UpdateGuiRange();

    const MS::FrameObject& obj = _controller.selected();

    _location->SetValue(obj.location);
    _tileId->SetValue(obj.tileId);
    _size->SetSelection(obj.size == OS::LARGE ? 1 : 0);
    _order->SetValue(obj.order);
    _hFlip->SetValue(obj.hFlip);
    _vFlip->SetValue(obj.vFlip);
}

void FrameObjectPanel::UpdateGuiRange()
{
    typedef UnTech::MetaSprite::ObjectSize OS;

    const MS::FrameObject& obj = _controller.selected();
    const MS::FrameSet& frameSet = _msController.frameSetController().selected();

    if (obj.size == OS::SMALL) {
        _tileId->SetRange(0, frameSet.smallTileset.size() - 1);
    }
    else {
        _tileId->SetRange(0, frameSet.largeTileset.size() - 1);
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
    _parameter->SetRange(1, 255);
    grid->Add(new wxStaticText(this, wxID_ANY, "Parameter:"));
    grid->Add(_parameter, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGui));

    // Events
    // ------
    _location->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setLocation(_location->GetValue());
    });

    _parameter->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setParameter(_parameter->GetValue());
    });
}

void ActionPointPanel::UpdateGui()
{
    this->Enable(_controller.hasSelected());

    const MS::ActionPoint& ap = _controller.selected();

    _location->SetValue(ap.location);
    _parameter->SetValue(ap.parameter);
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

    _hitboxType = new EnumClassChoice<UnTech::MetaSprite::EntityHitboxType>(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Type:"));
    grid->Add(_hitboxType, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGui));

    // Events
    // ------
    _aabb->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setAabb(_aabb->GetValue());
    });

    _hitboxType->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        _controller.selected_setHitboxType(_hitboxType->GetValue());
    });
}

void EntityHitboxPanel::UpdateGui()
{
    this->Enable(_controller.hasSelected());

    const MS::EntityHitbox& eh = _controller.selected();

    _aabb->SetValue(eh.aabb);
    _hitboxType->SetValue(eh.hitboxType);
}
