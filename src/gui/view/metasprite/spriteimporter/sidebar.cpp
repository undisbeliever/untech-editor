#include "sidebar.h"
#include "sidebar-lists.hpp"
#include "gui/view/common/aabb.h"
#include "gui/view/common/enumclasschoice.h"
#include "gui/view/common/filedialogs.h"
#include "gui/view/common/textandtogglebuttonctrl.h"
#include "gui/view/defaults.h"
#include "gui/view/metasprite/animation/animation-sidebarpage.h"
#include "gui/view/metasprite/common/export-sidebarpage.hpp"
#include "gui/view/metasprite/common/framesetcommonpanel.hpp"
#include "models/common/string.h"
#include <wx/spinctrl.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace SpriteImporter {

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

const DocumentType PNG_DOCUMENT_TYPE = {
    "PNG Image",
    "png"
};

class FrameSetImagePanel : public wxPanel {
public:
    FrameSetImagePanel(wxWindow* parent, int wxWindowID,
                       SI::SpriteImporterController& controller);

private:
    void UpdateGui();

private:
    SI::SpriteImporterController& _controller;

    TextAndToggleButtonCtrl* _filename;
    TextAndToggleButtonCtrl* _transparentColor;
};

class FrameSetGridPanel : public wxPanel {
public:
    FrameSetGridPanel(wxWindow* parent, int wxWindowID,
                      SI::FrameSetController& controller);

private:
    void UpdateGui();

private:
    SI::FrameSetController& _controller;
    SI::FrameSetGrid _grid;

    USizeCtrl* _frameSize;
    UPointCtrl* _offset;
    UPointCtrl* _padding;
    UPointCtrl* _origin;
};

class FramePanel : public wxPanel {
public:
    FramePanel(wxWindow* parent, int wxWindowID,
               SI::SpriteImporterController& controller);

private:
    void UpdateGui();

private:
    SI::FrameController& _controller;
    SI::SpriteImporterController& _siController;

    SI::FrameLocation _frameLocation;

    wxSpinCtrl* _spriteOrder;

    wxCheckBox* _useGridLocation;
    UPointCtrl* _gridLocation;
    URectCtrl* _aabb;

    wxCheckBox* _useCustomOrigin;
    UPointCtrl* _origin;

    wxCheckBox* _solid;
    URectCtrl* _tileHitbox;
};

class FrameObjectPanel : public wxPanel {
public:
    FrameObjectPanel(wxWindow* parent, int wxWindowID,
                     SI::SpriteImporterController& controller);

private:
    void UpdateGui();
    void UpdateGuiRange();

private:
    SI::FrameObjectController& _controller;

    UPointCtrl* _location;
    wxChoice* _size;
};

class ActionPointPanel : public wxPanel {
public:
    ActionPointPanel(wxWindow* parent, int wxWindowID,
                     SI::SpriteImporterController& controller);

private:
    void UpdateGui();
    void UpdateGuiRange();

private:
    SI::ActionPointController& _controller;

    UPointCtrl* _location;
    wxSpinCtrl* _parameter;
};

class EntityHitboxPanel : public wxPanel {
public:
    EntityHitboxPanel(wxWindow* parent, int wxWindowID,
                      SI::SpriteImporterController& controller);

private:
    void UpdateGui();
    void UpdateGuiRange();

private:
    SI::EntityHitboxController& _controller;

    URectCtrl* _aabb;
    EnumClassChoice<UnTech::MetaSprite::EntityHitboxType>* _hitboxType;
};
}
}
}
}

using namespace UnTech::View::MetaSprite::SpriteImporter;
using namespace UnTech::View::MetaSprite::Common;

// SIDEBAR
// =======

Sidebar::Sidebar(wxWindow* parent, int wxWindowID,
                 SI::SpriteImporterController& controller)
    : wxNotebook(parent, wxWindowID)
    , _controller(controller)
{
    this->SetSizeHints(SIDEBAR_WIDTH, -1);

    // FrameSet Panel
    {
        auto* panel = new wxPanel(this);

        auto* sizer = new wxBoxSizer(wxVERTICAL);
        panel->SetSizer(sizer);

        auto* afPanel = new FrameSetCommonPanel<SI::FrameSetController>(
            panel, wxID_ANY, controller.frameSetController());
        sizer->Add(afPanel, wxSizerFlags().Expand().Border());

        auto* fsImageSizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Image");
        sizer->Add(fsImageSizer, wxSizerFlags().Expand().Border());

        fsImageSizer->Add(
            new FrameSetImagePanel(panel, wxID_ANY, controller),
            wxSizerFlags().Expand().Border());

        auto* fsGridSizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Grid");
        sizer->Add(fsGridSizer, wxSizerFlags().Expand().Border());

        fsGridSizer->Add(
            new FrameSetGridPanel(panel, wxID_ANY, controller.frameSetController()),
            wxSizerFlags().Expand().Border());

        this->AddPage(panel, "FrameSet");
    }

    // Frame Panel
    {
        auto* framePanel = new wxPanel(this);

        auto* frameSizer = new wxBoxSizer(wxVERTICAL);
        framePanel->SetSizer(frameSizer);

        frameSizer->Add(new IdMapListToolBar<SI::FrameController>(
                            framePanel, wxID_ANY,
                            controller.frameController()),
                        wxSizerFlags(0).Right().Border());

        frameSizer->Add(new IdMapListCtrl<SI::FrameController>(
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

            sizer->Add(new VectorListToolBar<SI::FrameObjectController>(
                           panel, wxID_ANY,
                           controller.frameObjectController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new VectorListCtrl<SI::FrameObjectController>(
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

            sizer->Add(new VectorListToolBar<SI::ActionPointController>(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new VectorListCtrl<SI::ActionPointController>(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

            sizer->Add(new ActionPointPanel(
                           panel, wxID_ANY,
                           controller),
                       wxSizerFlags(0).Expand().Border());

            _frameNotebook->AddPage(panel, "Action Points");
        }

        {
            auto* panel = new wxPanel(_frameNotebook);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new VectorListToolBar<SI::EntityHitboxController>(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new VectorListCtrl<SI::EntityHitboxController>(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

            sizer->Add(new EntityHitboxPanel(
                           panel, wxID_ANY,
                           controller),
                       wxSizerFlags(0).Expand().Border());

            _frameNotebook->AddPage(panel, "Hitboxes");
        }

        this->AddPage(framePanel, "Frame");

        this->AddPage(
            new Animation::AnimationSidebarPage(
                this, wxID_ANY,
                controller.animationControllerInterface()),
            "Animations");

        auto* exportPage = new ExportSidebarPage<SI::SpriteImporterController>(
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

// FRAMESET IMAGE
// ==============

FrameSetImagePanel::FrameSetImagePanel(wxWindow* parent, int wxWindowID,
                                       SI::SpriteImporterController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(2, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _filename = new TextAndToggleButtonCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Filename:"));
    grid->Add(_filename, wxSizerFlags().Expand());

    _transparentColor = new TextAndToggleButtonCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Transparent:"));
    grid->Add(_transparentColor, wxSizerFlags().Expand());

    // Signals
    // -------
    _controller.frameSetController().signal_anyChanged().connect(sigc::mem_fun(
        *this, &FrameSetImagePanel::UpdateGui));

    /** Update transparent button when changed */
    auto& settingsController = _controller.settingsController();

    settingsController.selectColorWithMouse().signal_valueChanged().connect([this](void) {
        auto& settingsController = _controller.settingsController();
        _transparentColor->SetButtonValue(settingsController.selectColorWithMouse().value());
    });

    // Events
    // ------
    _filename->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
        auto& frameSetController = _controller.frameSetController();

        const SI::FrameSet& frameSet = frameSetController.selected();

        if (_filename->GetButtonValue()) {
            auto fn = openFileDialog(this,
                                     PNG_DOCUMENT_TYPE,
                                     frameSet.imageFilename);
            if (fn) {
                frameSetController.selected_setImageFilename(fn.value());
            }
            _filename->SetButtonValue(false);
        }
    });

    _transparentColor->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
        _controller.settingsController().selectColorWithMouse().setValue(
            _transparentColor->GetButtonValue());
    });
}

void FrameSetImagePanel::UpdateGui()
{
    auto& frameSetController = _controller.frameSetController();

    this->Enable(frameSetController.hasSelected());

    const SI::FrameSet& frameSet = frameSetController.selected();

    _filename->ChangeTextValue(frameSet.imageFilename);

    if (frameSet.transparentColorValid()) {
        auto t = frameSet.transparentColor.rgb();

        auto c = wxString::Format("%06x", t);
        _transparentColor->ChangeTextValue(c);
        _transparentColor->GetButton()->SetBackgroundColour(t);
    }
    else {
        _transparentColor->ChangeTextValue(wxEmptyString);
        _transparentColor->GetButton()->SetBackgroundColour(
            wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    }

    _transparentColor->Enable(frameSet.isImageValid());
}

// FRAMESET GRID
// =============

FrameSetGridPanel::FrameSetGridPanel(wxWindow* parent, int wxWindowID,
                                     SI::FrameSetController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(4, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _frameSize = new USizeCtrl(this, wxID_ANY);
    _frameSize->SetMaxValue(usize(255, 255));
    grid->Add(new wxStaticText(this, wxID_ANY, "Size:"));
    grid->Add(_frameSize, wxSizerFlags(1).Expand());

    _offset = new UPointCtrl(this, wxID_ANY);
    _offset->SetRange(usize(255, 255));
    grid->Add(new wxStaticText(this, wxID_ANY, "Offset:"));
    grid->Add(_offset, wxSizerFlags(1).Expand());

    _padding = new UPointCtrl(this, wxID_ANY);
    _padding->SetRange(usize(255, 255));
    grid->Add(new wxStaticText(this, wxID_ANY, "Padding:"));
    grid->Add(_padding, wxSizerFlags(1).Expand());

    _origin = new UPointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Origin:"));
    grid->Add(_origin, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &FrameSetGridPanel::UpdateGui));

    // Events
    // ------
    _frameSize->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _grid.frameSize = _frameSize->GetValue();
        _controller.selected_setGrid(_grid);
    });

    _offset->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _grid.offset = _offset->GetValue();
        _controller.selected_setGrid(_grid);
    });

    _padding->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _grid.padding = _padding->GetValue();
        _controller.selected_setGrid(_grid);
    });

    _origin->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _grid.origin = _origin->GetValue();
        _controller.selected_setGrid(_grid);
    });
}

void FrameSetGridPanel::UpdateGui()
{
    this->Enable(_controller.hasSelected());

    _grid = _controller.selected().grid;

    const SI::FrameSet& frameSet = _controller.selected();
    _frameSize->SetMinValue(frameSet.minimumFrameGridSize());
    _origin->SetRange(_grid.frameSize);

    _frameSize->SetValue(_grid.frameSize);
    _offset->SetValue(_grid.offset);
    _padding->SetValue(_grid.padding);
    _origin->SetValue(_grid.origin);
}

// FRAME
// =====

FramePanel::FramePanel(wxWindow* parent, int wxWindowID,
                       SI::SpriteImporterController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller.frameController())
    , _siController(controller)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(8, 2, defBorder, defBorder * 2);
    sizer->Add(grid, wxSizerFlags(1).Expand().Border());

    grid->AddGrowableCol(1, 1);

    _spriteOrder = new wxSpinCtrl(this, wxID_ANY);
    _spriteOrder->SetRange(0, 3);
    grid->Add(new wxStaticText(this, wxID_ANY, "Sprite Order:"));
    grid->Add(_spriteOrder, wxSizerFlags(1).Expand());

    _useGridLocation = new wxCheckBox(this, wxID_ANY, "Use Grid");
    grid->Add(_useGridLocation, wxSizerFlags(1));
    grid->Add(new wxWindow(this, wxID_ANY));

    _gridLocation = new UPointCtrl(this, wxID_ANY);
    _gridLocation->SetRange(usize(255, 255));
    grid->Add(new wxStaticText(this, wxID_ANY, "Grid Location:"));
    grid->Add(_gridLocation, wxSizerFlags(1).Expand());

    _aabb = new URectCtrl(this, wxID_ANY);
    _aabb->SetMaxRectSize(usize(255, 255));
    grid->Add(new wxStaticText(this, wxID_ANY, "Location:"));
    grid->Add(_aabb, wxSizerFlags(1).Expand());

    _useCustomOrigin = new wxCheckBox(this, wxID_ANY, "Custom Origin");
    grid->Add(_useCustomOrigin, wxSizerFlags(1));
    grid->Add(new wxWindow(this, wxID_ANY));

    _origin = new UPointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Origin:"));
    grid->Add(_origin, wxSizerFlags(1).Expand());

    _solid = new wxCheckBox(this, wxID_ANY, "Solid");
    grid->Add(_solid, wxSizerFlags(1));
    grid->Add(new wxWindow(this, wxID_ANY));

    _tileHitbox = new URectCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Tile Hitbox:"));
    grid->Add(_tileHitbox, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &FramePanel::UpdateGui));

    _siController.frameSetController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &FramePanel::UpdateGui));

    // Events
    // ------
    _spriteOrder->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setSpriteOrder(_spriteOrder->GetValue());
    });

    _useGridLocation->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _frameLocation.useGridLocation = _useGridLocation->GetValue();

        _controller.selected_setLocation(_frameLocation);
    });

    _gridLocation->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _frameLocation.gridLocation = _gridLocation->GetValue();

        _controller.selected_setLocation(_frameLocation);
    });

    _aabb->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _frameLocation.aabb = _aabb->GetValue();

        _controller.selected_setLocation(_frameLocation);
    });

    _useCustomOrigin->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _frameLocation.useGridOrigin = !_useCustomOrigin->GetValue();

        _controller.selected_setLocation(_frameLocation);
    });

    _origin->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _frameLocation.origin = _origin->GetValue();

        _controller.selected_setLocation(_frameLocation);
    });

    _solid->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _controller.selected_setSolid(_solid->GetValue());
        _siController.selectedController().selectTileHitbox();
    });

    _tileHitbox->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setTileHitbox(_tileHitbox->GetValue());
        _siController.selectedController().selectTileHitbox();
    });
}

void FramePanel::UpdateGui()
{
    this->Enable(_controller.hasSelected());

    const SI::Frame& frame = _controller.selected();
    _frameLocation = frame.location;

    _aabb->SetMinRectSize(frame.minimumViableSize());
    _origin->SetRange(_frameLocation.aabb.size());
    _tileHitbox->SetRange(_frameLocation.aabb.size());

    _spriteOrder->SetValue(frame.spriteOrder);

    _useGridLocation->SetValue(_frameLocation.useGridLocation);
    _gridLocation->Enable(_frameLocation.useGridLocation);
    _gridLocation->SetValue(_frameLocation.gridLocation);

    _aabb->Enable(!_frameLocation.useGridLocation);
    _aabb->SetValue(_frameLocation.aabb);

    _useCustomOrigin->SetValue(!_frameLocation.useGridOrigin);
    _origin->Enable(!_frameLocation.useGridOrigin);
    _origin->SetValue(_frameLocation.origin);

    _solid->SetValue(frame.solid);
    _tileHitbox->Enable(frame.solid);
    _tileHitbox->SetValue(frame.tileHitbox);
}

// FRAME OBJECTS
// =============

FrameObjectPanel::FrameObjectPanel(wxWindow* parent, int wxWindowID,
                                   SI::SpriteImporterController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller.frameObjectController())
{
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(5, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _location = new UPointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Location:"));
    grid->Add(_location, wxSizerFlags(1).Expand());

    wxArrayString sizeChoices;
    sizeChoices.Add("Small");
    sizeChoices.Add("Large");

    _size = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sizeChoices);
    grid->Add(new wxStaticText(this, wxID_ANY, "Size:"));
    grid->Add(_size, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGui));

    controller.frameController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGuiRange));

    controller.frameSetController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGuiRange));

    // Events
    // ------
    _location->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setLocation(_location->GetValue());
    });

    _size->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        typedef UnTech::MetaSprite::ObjectSize OS;

        _controller.selected_setSize(
            _size->GetSelection() == 1 ? OS::LARGE : OS::SMALL);
    });
}

void FrameObjectPanel::UpdateGui()
{
    typedef UnTech::MetaSprite::ObjectSize OS;

    this->Enable(_controller.hasSelected());

    UpdateGuiRange();

    const SI::FrameObject& obj = _controller.selected();

    _location->SetValue(obj.location);
    _size->SetSelection(obj.size == OS::LARGE ? 1 : 0);
}

void FrameObjectPanel::UpdateGuiRange()
{
    const SI::FrameObject& obj = _controller.selected();
    const SI::Frame& frame = _controller.parent().selected();

    _location->SetRange(frame.location.aabb.size(), obj.sizePx());
}

// ACTION POINTS
// =============

ActionPointPanel::ActionPointPanel(wxWindow* parent, int wxWindowID,
                                   SI::SpriteImporterController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller.actionPointController())
{
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(2, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _location = new UPointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Location:"));
    grid->Add(_location, wxSizerFlags(1).Expand());

    _parameter = new wxSpinCtrl(this, wxID_ANY);
    _parameter->SetRange(1, 255);
    grid->Add(new wxStaticText(this, wxID_ANY, "Parameter:"));
    grid->Add(_parameter, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGui));

    controller.frameController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGuiRange));

    controller.frameSetController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGuiRange));

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

    const SI::ActionPoint& ap = _controller.selected();

    UpdateGuiRange();

    _location->SetValue(ap.location);
    _parameter->SetValue(ap.parameter);
}

void ActionPointPanel::UpdateGuiRange()
{
    const SI::Frame& frame = _controller.parent().selected();

    _location->SetRange(frame.location.aabb.size());
}

// ENTITY HITBOXES
// ===============

EntityHitboxPanel::EntityHitboxPanel(wxWindow* parent, int wxWindowID,
                                     SI::SpriteImporterController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller.entityHitboxController())
{
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(2, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _aabb = new URectCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "AABB:"));
    grid->Add(_aabb, wxSizerFlags(1).Expand());

    _hitboxType = new EnumClassChoice<UnTech::MetaSprite::EntityHitboxType>(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Type:"));
    grid->Add(_hitboxType, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGui));

    controller.frameController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGuiRange));

    controller.frameSetController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGuiRange));

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

    const SI::EntityHitbox& eh = _controller.selected();

    UpdateGuiRange();

    _aabb->SetValue(eh.aabb);
    _hitboxType->SetValue(eh.hitboxType);
}

void EntityHitboxPanel::UpdateGuiRange()
{
    const SI::Frame& frame = _controller.parent().selected();

    _aabb->SetRange(frame.location.aabb.size());
}
