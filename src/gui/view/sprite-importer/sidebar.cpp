#include "sidebar.h"
#include "sidebar-lists.hpp"
#include "gui/view/common/aabb.h"
#include "gui/view/common/filedialogs.h"
#include "gui/view/common/textandtogglebuttonctrl.h"
#include "gui/view/defaults.h"
#include "gui/view/metasprite-common/abstractframesetpanel.h"
#include "gui/view/metasprite-common/animation-sidebarpage.h"
#include "gui/view/metasprite-common/export-sidebarpage.h"
#include "models/common/string.h"
#include <wx/spinctrl.h>

namespace UnTech {
namespace View {
namespace SpriteImporter {

static const upoint zeroPoint(0, 0);
static const usize zeroSize(0, 0);
static const urect zeroRect(0, 0, 0, 0);

const UnTech::DocumentType PNG_DOCUMENT_TYPE = {
    "PNG Image",
    "png"
};

class FrameSetImagePanel : public wxPanel {
public:
    FrameSetImagePanel(wxWindow* parent, int wxWindowID,
                       SI::FrameSetController& controller);

private:
    void UpdateGui();

private:
    SI::FrameSetController& _controller;

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

    USizeCtrl* _frameSize;
    UPointCtrl* _offset;
    USizeCtrl* _padding;
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

    wxSpinCtrl* _spriteOrder;

    wxCheckBox* _useGridLocation;
    UPointCtrl* _gridLocation;
    URectCtrl* _location;

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
    wxSpinCtrl* _parameter;
};
}
}
}

using namespace UnTech::View::SpriteImporter;
using namespace UnTech::View::MetaSpriteCommon;

// SIDEBAR
// =======

Sidebar::Sidebar(wxWindow* parent, int wxWindowID,
                 SI::SpriteImporterController& controller)
    : wxNotebook(parent, wxWindowID)
{
    this->SetSizeHints(SIDEBAR_WIDTH, -1);

    // FrameSet Panel
    {
        auto* panel = new wxPanel(this);

        auto* sizer = new wxBoxSizer(wxVERTICAL);
        panel->SetSizer(sizer);

        auto* afPanel = new AbstractFrameSetPanel(panel, wxID_ANY, controller.abstractFrameSetController());
        sizer->Add(afPanel, wxSizerFlags().Expand().Border());

        auto* fsImageSizer = new wxStaticBoxSizer(wxVERTICAL, panel, "Image");
        sizer->Add(fsImageSizer, wxSizerFlags().Expand().Border());

        fsImageSizer->Add(
            new FrameSetImagePanel(panel, wxID_ANY, controller.frameSetController()),
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

        frameSizer->Add(new NamedListToolBar<SI::Frame>(
                            framePanel, wxID_ANY,
                            controller.frameController()),
                        wxSizerFlags(0).Right().Border());

        frameSizer->Add(new NamedListCtrl<SI::Frame>(
                            framePanel, wxID_ANY,
                            controller.frameController()),
                        wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

        auto* frameNotepad = new wxNotebook(framePanel, wxID_ANY);
        frameSizer->Add(frameNotepad, wxSizerFlags(1).Expand().Border());

        frameNotepad->AddPage(
            new FramePanel(frameNotepad, wxID_ANY, controller),
            "Frame");

        {
            auto* panel = new wxPanel(frameNotepad);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new OrderedListToolBar<SI::FrameObject>(
                           panel, wxID_ANY,
                           controller.frameObjectController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new OrderedListCtrl<SI::FrameObject>(
                           panel, wxID_ANY,
                           controller.frameObjectController()),
                       wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

            sizer->Add(new FrameObjectPanel(
                           panel, wxID_ANY,
                           controller),
                       wxSizerFlags(0).Expand().Border());

            frameNotepad->AddPage(panel, "Objects");
        }

        {
            auto* panel = new wxPanel(frameNotepad);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new OrderedListToolBar<SI::ActionPoint>(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new OrderedListCtrl<SI::ActionPoint>(
                           panel, wxID_ANY,
                           controller.actionPointController()),
                       wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

            sizer->Add(new ActionPointPanel(
                           panel, wxID_ANY,
                           controller),
                       wxSizerFlags(0).Expand().Border());

            frameNotepad->AddPage(panel, "Action Points");
        }

        {
            auto* panel = new wxPanel(frameNotepad);

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            panel->SetSizer(sizer);

            sizer->Add(new OrderedListToolBar<SI::EntityHitbox>(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       wxSizerFlags(0).Right().Border());

            sizer->Add(new OrderedListCtrl<SI::EntityHitbox>(
                           panel, wxID_ANY,
                           controller.entityHitboxController()),
                       wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

            sizer->Add(new EntityHitboxPanel(
                           panel, wxID_ANY,
                           controller),
                       wxSizerFlags(0).Expand().Border());

            frameNotepad->AddPage(panel, "Hitboxes");
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
    }
}

// FRAMESET IMAGE
// ==============

FrameSetImagePanel::FrameSetImagePanel(wxWindow* parent, int wxWindowID,
                                       SI::FrameSetController& controller)
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
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameSetImagePanel::UpdateGui));

    _controller.signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetImagePanel::UpdateGui)));

    /** Update transparent button when changed */
    _controller.signal_selectTransparentModeChanged().connect([this](void) {
        _transparentColor->SetButtonValue(_controller.selectTransparentMode());
    });

    // Events
    // ------
    _filename->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
        const SI::FrameSet* frameSet = _controller.selected();

        if (frameSet && _filename->GetButtonValue()) {
            auto fn = openFileDialog(this,
                                     PNG_DOCUMENT_TYPE,
                                     frameSet->imageFilename());
            if (fn) {
                _controller.selected_setImageFilename(fn.value());
            }
            _filename->SetButtonValue(false);
        }
    });

    _transparentColor->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
        const SI::FrameSet* frameSet = _controller.selected();

        if (frameSet) {
            _controller.setSelectTransparentMode(
                _transparentColor->GetButtonValue());
        }
    });
}

void FrameSetImagePanel::UpdateGui()
{
    const SI::FrameSet* frameSet = _controller.selected();
    if (frameSet) {
        _filename->ChangeTextValue(frameSet->imageFilename());

        if (frameSet->transparentColorValid()) {
            auto t = frameSet->transparentColor().rgb();

            auto c = wxString::Format("%06x", t);
            _transparentColor->ChangeTextValue(c);
            _transparentColor->GetButton()->SetBackgroundColour(t);
        }
        else {
            _transparentColor->ChangeTextValue(wxEmptyString);
            _transparentColor->GetButton()->SetBackgroundColour(
                wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
        }
        _transparentColor->Enable(!frameSet->image().empty());

        this->Enable();
    }
    else {
        _filename->ChangeTextValue(wxEmptyString);

        _transparentColor->ChangeTextValue(wxEmptyString);
        _transparentColor->GetButton()->SetBackgroundColour(
            wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));

        this->Disable();
    }
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
    grid->Add(new wxStaticText(this, wxID_ANY, "Size:"));
    grid->Add(_frameSize, wxSizerFlags(1).Expand());

    _offset = new UPointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Offset:"));
    grid->Add(_offset, wxSizerFlags(1).Expand());

    _padding = new USizeCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Padding:"));
    grid->Add(_padding, wxSizerFlags(1).Expand());

    _origin = new UPointCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Origin:"));
    grid->Add(_origin, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameSetGridPanel::UpdateGui));

    controller.signal_gridChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameSetGridPanel::UpdateGui)));

    // Events
    // ------
    _frameSize->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _frameSize->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setGridFrameSize_merge(_frameSize->GetValue());
    });

    _offset->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _offset->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setGridOffset_merge(_offset->GetValue());
    });

    _padding->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _padding->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setGridPadding_merge(_padding->GetValue());
    });

    _origin->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _origin->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setGridOrigin_merge(_origin->GetValue());
    });
}

void FrameSetGridPanel::UpdateGui()
{
    const SI::FrameSet* frameSet = _controller.selected();
    if (frameSet) {
        const SI::FrameSetGrid& grid = frameSet->grid();

        _frameSize->SetValue(grid.frameSize());
        _offset->SetValue(grid.offset());
        _padding->SetValue(grid.padding());
        _origin->SetValue(grid.origin());

        this->Enable();
    }
    else {
        _frameSize->SetValue(zeroSize);
        _offset->SetValue(zeroPoint);
        _padding->SetValue(zeroSize);
        _origin->SetValue(zeroPoint);

        this->Disable();
    }
}

// FRAME
// =====

FramePanel::FramePanel(wxWindow* parent, int wxWindowID,
                       SI::SpriteImporterController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller.frameController())
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
    grid->Add(new wxStaticText(this, wxID_ANY, "Grid Location:"));
    grid->Add(_gridLocation, wxSizerFlags(1).Expand());

    _location = new URectCtrl(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Location:"));
    grid->Add(_location, wxSizerFlags(1).Expand());

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
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FramePanel::UpdateGui));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FramePanel::UpdateGui));

    controller.frameSetController().signal_gridChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FramePanel::UpdateGui)));

    // Events
    // ------
    _spriteOrder->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setSpriteOrder(_spriteOrder->GetValue());
    });

    _useGridLocation->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _controller.selected_setUseGridLocation(_useGridLocation->GetValue());
    });

    _gridLocation->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _gridLocation->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setGridLocation_merge(_gridLocation->GetValue());
    });

    _location->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _location->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setLocation_merge(_location->GetValue());
    });

    _useCustomOrigin->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _controller.selected_setUseGridOrigin(!_useCustomOrigin->GetValue());
    });

    _origin->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _origin->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setOrigin_merge(_origin->GetValue());
    });

    _solid->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _controller.selected_setSolid(_solid->GetValue());
    });

    _tileHitbox->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _tileHitbox->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setTileHitbox_merge(_tileHitbox->GetValue());
    });
}

void FramePanel::UpdateGui()
{
    const SI::Frame* frame = _controller.selected();
    if (frame) {
        _location->SetMinSize(frame->minimumViableSize());
        _origin->SetRange(frame->locationSize());
        _tileHitbox->SetRange(frame->locationSize());

        _spriteOrder->SetValue(frame->spriteOrder());

        _useGridLocation->SetValue(frame->useGridLocation());
        _gridLocation->Enable(frame->useGridLocation());
        _gridLocation->SetValue(frame->gridLocation());

        _location->Enable(!frame->useGridLocation());
        _location->SetValue(frame->location());

        _useCustomOrigin->SetValue(!frame->useGridOrigin());
        _origin->Enable(!frame->useGridOrigin());
        _origin->SetValue(frame->origin());

        _solid->SetValue(frame->solid());
        _tileHitbox->Enable(frame->solid());
        _tileHitbox->SetValue(frame->tileHitbox());

        this->Enable();
    }
    else {
        _location->SetMinSize(zeroSize);
        _origin->SetRange(zeroSize);
        _tileHitbox->SetRange(zeroSize);

        _spriteOrder->SetValue(0);

        _useGridLocation->SetValue(false);
        _gridLocation->SetValue(zeroPoint);
        _location->SetValue(zeroRect);

        _useCustomOrigin->SetValue(false);
        _origin->SetValue(zeroPoint);

        _solid->SetValue(false);
        _tileHitbox->SetValue(zeroRect);

        this->Disable();
    }
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
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGui));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGui));

    controller.frameController().signal_frameSizeChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGuiRange)));

    controller.frameSetController().signal_gridChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameObjectPanel::UpdateGuiRange)));

    // Events
    // ------
    _location->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& e) {
        _controller.baseController().dontMergeNextAction();
        e.Skip();
    });
    _location->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setLocation_merge(_location->GetValue());
    });

    _size->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;
        _controller.selected_setLocationAndSize(
            _location->GetValue(),
            _size->GetSelection() == 1 ? OS::LARGE : OS::SMALL);
    });
}

void FrameObjectPanel::UpdateGui()
{
    UpdateGuiRange();

    const SI::FrameObject* obj = _controller.selected();
    if (obj) {
        typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

        _location->SetValue(obj->location());
        _size->SetSelection(obj->size() == OS::LARGE ? 1 : 0);

        this->Enable();
    }
    else {
        _location->SetValue(zeroPoint);
        _size->SetSelection(wxNOT_FOUND);

        this->Disable();
    }
}

void FrameObjectPanel::UpdateGuiRange()
{
    const SI::FrameObject* obj = _controller.selected();
    if (obj) {
        _location->SetRange(obj->frame().locationSize(), obj->sizePx());
    }
    else {
        _location->SetRange(zeroSize);
    }
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

    controller.frameController().signal_frameSizeChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGuiRange)));

    controller.frameSetController().signal_gridChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &ActionPointPanel::UpdateGuiRange)));

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
    UpdateGuiRange();

    const SI::ActionPoint* ap = _controller.selected();
    if (ap) {
        _location->SetValue(ap->location());
        _parameter->SetValue(ap->parameter());

        this->Enable();
    }
    else {
        _location->SetValue(zeroPoint);
        _parameter->SetValue(0);

        this->Disable();
    }
}

void ActionPointPanel::UpdateGuiRange()
{
    const SI::ActionPoint* ap = _controller.selected();
    if (ap) {
        _location->SetRange(ap->frame().locationSize());
    }
    else {
        _location->SetRange(zeroSize);
    }
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

    controller.frameController().signal_frameSizeChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGuiRange)));

    controller.frameSetController().signal_gridChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &EntityHitboxPanel::UpdateGuiRange)));

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
    UpdateGuiRange();

    const SI::EntityHitbox* eh = _controller.selected();
    if (eh) {
        _aabb->SetValue(eh->aabb());
        _parameter->SetValue(eh->parameter());

        this->Enable();
    }
    else {
        _aabb->SetValue(zeroRect);
        _parameter->SetValue(0);

        this->Disable();
    }
}

void EntityHitboxPanel::UpdateGuiRange()
{
    const SI::EntityHitbox* eh = _controller.selected();
    if (eh) {
        _aabb->SetRange(eh->frame().locationSize());
    }
    else {
        _aabb->SetRange(zeroSize);
    }
}