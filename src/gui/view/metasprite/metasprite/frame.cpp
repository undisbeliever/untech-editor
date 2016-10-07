#include "frame.h"
#include "addtilesdialog.h"
#include "graphicspanel.h"
#include "sidebar.h"
#include "gui/view/common/aboutdialog.h"
#include "gui/view/common/controllerinterface.h"
#include "gui/view/common/filedialogs.h"
#include "gui/view/defaults.h"

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

const wxString Frame::WINDOW_NAME = "UnTech MetaSprite Editor";

const DocumentType FRAMESET_DOCUMENT_TYPE = {
    "MetaSprite FrameSet",
    // ::BUGFIX can't use MS::FrameSet::FILE_EXTENSION with g++ -O2::
    "utms"
};

enum MENU_IDS {
    ID_ADD_TILES = 1000,
    ID_CREATE,
    ID_CLONE,
    ID_REMOVE,
    ID_MOVE_UP,
    ID_MOVE_DOWN,
    ID_ZOOM_1,
    ID_ZOOM_2,
    ID_ZOOM_3,
    ID_ZOOM_4,
    ID_ZOOM_5,
    ID_ZOOM_6,
    ID_ZOOM_7,
    ID_ZOOM_8,
    ID_ZOOM_9,
    ID_ASPECT_SQUARE,
    ID_ASPECT_NTSC,
    ID_ASPECT_PAL,
    ID_SPLIT_VIEW,
    ID_CENTER_VIEW,
    ID_LAYER_FRAME_OBJECTS,
    ID_LAYER_ACTION_POINTS,
    ID_LAYER_ENTITY_HITBOXES,
    ID_LAYER_ORIGIN,
    ID_LAYER_TILE_HITBOX,
    ID_LAYER_SHOW_ALL,

    ID_INIT_TIMER,
};
}
}
}
}

using namespace UnTech::View::MetaSprite::MetaSprite;

Frame::Frame()
    : wxFrame(NULL, wxID_ANY, WINDOW_NAME)
    , _controllerInterface(this)
    , _controller(_controllerInterface)
{
    SetMinSize(MIN_FRAME_SIZE);

    // Widgets
    // =======
    {
        _graphics = new GraphicsPanel(this, wxID_ANY, _controller);
        _sidebar = new Sidebar(this, wxID_ANY, _controller);

        auto* sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->Add(_graphics, wxSizerFlags(1).Expand().Border());
        sizer->Add(_sidebar, wxSizerFlags(0).Expand().Border(wxTOP | wxBOTTOM | wxRIGHT));
        this->SetSizer(sizer);

        Centre();
    }

    // Menus
    // =====
    {
        auto* menuBar = new wxMenuBar();

        auto* file = new wxMenu();
        file->Append(wxID_NEW, "&New\tCTRL+N");
        file->Append(wxID_OPEN, "&Open\tCTRL+O");
        file->Append(wxID_SAVE, "&Save\tCTRL+S");
        file->Append(wxID_SAVEAS, "Save &As\tCTRL+SHIFT+S");
        file->AppendSeparator();
        file->Append(wxID_ABOUT);
        file->AppendSeparator();
        file->Append(wxID_EXIT, "&Exit\tALT+F4");

        auto* edit = new wxMenu();
        edit->Append(wxID_UNDO);
        edit->Append(wxID_REDO);
        edit->AppendSeparator();
        edit->Append(ID_ADD_TILES, "Add Tiles");
        edit->AppendSeparator();
        edit->Append(ID_CREATE, "Create");
        edit->Append(ID_CLONE, "Clone Selected\tCTRL+D");
        edit->Append(ID_REMOVE, "Remove Selected");
        edit->Append(ID_MOVE_UP, "Move Selected Up");
        edit->Append(ID_MOVE_DOWN, "Move Selected Down");

        auto* zoom = new wxMenu();
        zoom->Append(wxID_ZOOM_IN, "Zoom &In\tCTRL++");
        zoom->Append(wxID_ZOOM_OUT, "Zoom &Out\tCTRL+-");
        zoom->AppendRadioItem(ID_ZOOM_1, "1\tALT+1");
        zoom->AppendRadioItem(ID_ZOOM_2, "2\tALT+2");
        zoom->AppendRadioItem(ID_ZOOM_3, "3\tALT+3");
        zoom->AppendRadioItem(ID_ZOOM_4, "4\tALT+4");
        zoom->AppendRadioItem(ID_ZOOM_5, "5\tALT+5");
        zoom->AppendRadioItem(ID_ZOOM_6, "6\tALT+6");
        zoom->AppendRadioItem(ID_ZOOM_7, "7\tALT+7");
        zoom->AppendRadioItem(ID_ZOOM_8, "8\tALT+8");
        zoom->AppendRadioItem(ID_ZOOM_9, "9\tALT+9");

        auto* aspectRatio = new wxMenu();
        aspectRatio->AppendRadioItem(ID_ASPECT_SQUARE, "&Square");
        aspectRatio->AppendRadioItem(ID_ASPECT_NTSC, "&NTSC");
        aspectRatio->AppendRadioItem(ID_ASPECT_PAL, "&PAL");

        auto* layers = new wxMenu();
        layers->AppendCheckItem(ID_LAYER_FRAME_OBJECTS, "Frame &Objects\tALT+g");
        layers->AppendCheckItem(ID_LAYER_ACTION_POINTS, "&Action Points\tALT+h");
        layers->AppendCheckItem(ID_LAYER_ENTITY_HITBOXES, "&Entity Hitboxes\tALT+j");
        layers->AppendCheckItem(ID_LAYER_ORIGIN, "O&rigin\tALT+k");
        layers->AppendCheckItem(ID_LAYER_TILE_HITBOX, "&Tile Hitbox\tALT+l");
        layers->Append(ID_LAYER_SHOW_ALL, "Show &All");

        auto* view = new wxMenu();
        view->AppendSubMenu(zoom, "&Zoom");
        view->AppendSubMenu(aspectRatio, "&Aspect Ratio");
        view->AppendSubMenu(layers, "&Layers");
        view->AppendCheckItem(ID_SPLIT_VIEW, "&Split View\tCTRL+T");
        view->Append(ID_CENTER_VIEW, "&Centre View\tCTRL+E");

        menuBar->Append(file, "&File");
        menuBar->Append(edit, "&Edit");
        menuBar->Append(view, "&View");

        this->SetMenuBar(menuBar);

        // EVENTS
        // ------

        // FILE
        menuBar->Bind(wxEVT_COMMAND_MENU_SELECTED,
                      &Frame::OnMenuNew, this,
                      wxID_NEW);

        menuBar->Bind(wxEVT_COMMAND_MENU_SELECTED,
                      &Frame::OnMenuOpen, this,
                      wxID_OPEN);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                SaveDocument();
            },
            wxID_SAVE);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                SaveDocumentAs();
            },
            wxID_SAVEAS);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                AboutDialog dialog(this, WINDOW_NAME);
                dialog.ShowModal();
            },
            wxID_ABOUT);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                this->Close();
            },
            wxID_EXIT);

        // EDIT

        // ::TODO UndoStack::
        /*

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                _controller.undoStack().undo();
            },
            wxID_UNDO);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                _controller.undoStack().redo();
            },
            wxID_REDO);
        */

        edit->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                auto& fsController = _controller.frameSetController();

                if (fsController.hasSelected()) {
                    const MS::FrameSet& frameSet = fsController.selected();

                    AddTilesDialog dialog(this,
                                          frameSet.smallTileset.size(),
                                          frameSet.largeTileset.size());

                    if (dialog.ShowModal() == wxID_OK) {
                        fsController.selected_addTiles(dialog.GetSmallToAdd(),
                                                       dialog.GetLargeToAdd());
                    }
                }
            },
            ID_ADD_TILES);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                _controller.selectedController().createNewOfSelectedType();
            },
            ID_CREATE);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                _controller.selectedController().cloneSelected();
            },
            ID_CLONE);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                _controller.selectedController().removeSelected();
            },
            ID_REMOVE);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                _controller.selectedController().moveSelectedUp();
            },
            ID_MOVE_UP);

        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                _controller.selectedController().moveSelectedDown();
            },
            ID_MOVE_DOWN);

        // VIEW

        zoom->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                auto& zoom = _controller.settingsController().zoom();
                zoom.setZoom(zoom.zoom() + 1);
            },
            wxID_ZOOM_IN);

        zoom->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                auto& zoom = _controller.settingsController().zoom();
                zoom.setZoom(zoom.zoom() - 1);
            },
            wxID_ZOOM_OUT);

        zoom->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent& e) {
                if (e.IsChecked()) {
                    int z = e.GetId() - ID_ZOOM_1 + 1;
                    _controller.settingsController().zoom().setZoom(z);
                }
            },
            ID_ZOOM_1, ID_ZOOM_9);

        aspectRatio->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent& e) {
                if (e.IsChecked()) {
                    int a = e.GetId() - ID_ASPECT_SQUARE;
                    _controller.settingsController().zoom().setAspectRatio(
                        static_cast<UnTech::MetaSprite::ViewSettings::Zoom::AspectRatio>(a));
                }
            },
            ID_ASPECT_SQUARE, ID_ASPECT_PAL);

        layers->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent& e) {
                auto& layers = _controller.settingsController().layers();

                switch (e.GetId()) {
                case ID_LAYER_ORIGIN:
                    layers.setOrigin(e.IsChecked());
                    break;
                case ID_LAYER_TILE_HITBOX:
                    layers.setTileHitbox(e.IsChecked());
                    break;
                case ID_LAYER_FRAME_OBJECTS:
                    layers.setFrameObjects(e.IsChecked());
                    break;
                case ID_LAYER_ACTION_POINTS:
                    layers.setActionPoints(e.IsChecked());
                    break;
                case ID_LAYER_ENTITY_HITBOXES:
                    layers.setEntityHitboxes(e.IsChecked());
                    break;
                case ID_LAYER_SHOW_ALL:
                    layers.showAll();
                    break;
                }
            });

        view->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent& e) {
                _graphics->SetSplit(e.IsChecked());
            },
            ID_SPLIT_VIEW);

        view->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                _graphics->CenterMetaSpriteFrames();
            },
            ID_CENTER_VIEW);
    }

    UpdateGuiUndo();
    UpdateGuiMenu();
    UpdateGuiZoom();
    UpdateGuiLayers();

    // Signals
    // =======

    _controller.selectedController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &Frame::UpdateGuiMenu));

    _controller.selectedController().signal_listChanged().connect(sigc::mem_fun(
        *this, &Frame::UpdateGuiMenu));

    // ::TODO UndoStack::
    /*
    _controller.undoStack().signal_stackChanged().connect(sigc::mem_fun(
        *this, &Frame::UpdateGuiUndo));

    _controller.undoStack().signal_dirtyBitChanged().connect(sigc::mem_fun(
        *this, &Frame::UpdateGuiTitle));
     */

    _controller.frameSetController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &Frame::UpdateGuiTitle));

    _controller.frameSetController().signal_selectedChanged().connect(
        [this](void) {
            UpdateGuiMenu();
            UpdateGuiZoom();
            UpdateGuiUndo();
            UpdateGuiTitle();
        });

    // Events
    // ======
    this->Bind(wxEVT_CLOSE_WINDOW, &Frame::OnClose, this);

    /*
     * BUGFIX:
     *
     * Tab ordering is broken by a wxWindow->Disable() call
     * in UpdateGui, which is called by the sidebar/list/toolbar
     * constructors.
     *
     * Other attempts to fix this have failed.
     * Currently tab order is still broken even if I:
     *      * Call `UpdateGui` in constructor
     *      * Call `UpdateGui` or `emitAllDataChanged` after show in main/controller
     *      * Call `emitAllDataChanged` in wxEVT_SHOW
     *      * Call `emitAllDataChanged` in wxEVT_IDLE
     *      * Change tab order in `UpdateGui`
     *
     * Ironically, after the wxFrame is shown and fully rendered,
     * calls to Disable/Enable do not break tab ordering.
     *
     * This hack will get the controller to update the GUI 100 ms
     * after it is displayed on the screen. Users should not notice
     * because the broken widgets are on the second and third wxNotebook
     * page.
     *
     * Previously the `emitAllDataChanged` call was in the Sidebar's
     * wxEVT_NOTEBOOK_PAGE_CHANGING event, but it was intermittently
     * segfaulting because wxNoteBook::Destroy was emitting the event if
     * the user was not on the first notebook page.
     */
    _initBugfixTimer.SetOwner(this, ID_INIT_TIMER);

    this->Bind(
        wxEVT_TIMER, [this](wxTimerEvent&) {
            // signal will propagate through the controller and all child controllers will be called.
            _controller.frameSetController().signal_selectedChanged().emit();
        },
        ID_INIT_TIMER);

    this->Bind(
        wxEVT_SHOW, [this](wxShowEvent&) {
            this->CallAfter([this](void) {
                _initBugfixTimer.StartOnce(100);
            });
        });
}

void Frame::CreateOpen(const std::string& filename)
{
    auto* frame = new Frame();
    bool s = frame->Controller().loadDocument(filename);

    if (s) {
        frame->Show(true);
    }
    else {
        frame->Destroy();
    }
}

void Frame::OnClose(wxCloseEvent& event)
{
    // ::TODO UndoStack::
    const bool isDirty = _controller.hasDocument();

    if (event.CanVeto() && isDirty) {
        int r = wxMessageBox("Do you want to save?\n\n"
                             "If you close without saving, your changes will be discarded.",
                             "Do you want to save?",
                             wxICON_QUESTION | wxCANCEL | wxYES | wxNO,
                             this);

        if (r == wxYES) {
            bool s = SaveDocument();
            if (s == false) {
                // keep frame open
                event.Veto();
                return;
            }
        }
        else if (r == wxCANCEL) {
            // keep frame open
            event.Veto();
            return;
        }
    }

    event.Skip(); // close frame
}

void Frame::UpdateGuiMenu()
{
    wxMenuBar* menuBar = GetMenuBar();

    if (_controller.hasDocument()) {
        menuBar->EnableTop(1, true);
        menuBar->EnableTop(2, true);
        menuBar->Enable(wxID_SAVEAS, true);
        menuBar->Enable(ID_ADD_TILES, true);
    }
    else {
        menuBar->EnableTop(1, false);
        menuBar->EnableTop(2, false);
        menuBar->Enable(wxID_SAVE, false);
        menuBar->Enable(wxID_SAVEAS, false);
        menuBar->Enable(ID_ADD_TILES, false);
    }

    auto& selected = _controller.selectedController();

    menuBar->Enable(ID_CREATE, selected.canCreateSelected());
    menuBar->Enable(ID_CLONE, selected.canCloneSelected());
    menuBar->Enable(ID_REMOVE, selected.canRemoveSelected());
    menuBar->Enable(ID_MOVE_UP, selected.canMoveSelectedUp());
    menuBar->Enable(ID_MOVE_DOWN, selected.canMoveSelectedDown());
}

void Frame::UpdateGuiZoom()
{
    wxMenuBar* menuBar = GetMenuBar();
    auto& setting = _controller.settingsController().zoom();

    int zoom = setting.zoom();
    if (zoom >= 0 && zoom <= 9) {
        menuBar->Check(ID_ZOOM_1 - 1 + zoom, true);
    }
    menuBar->Check(ID_ASPECT_SQUARE + static_cast<int>(setting.aspectRatio()), true);
}

void Frame::UpdateGuiLayers()
{
    wxMenuBar* menuBar = GetMenuBar();
    auto& layers = _controller.settingsController().layers();

    menuBar->Check(ID_LAYER_FRAME_OBJECTS, layers.frameObjects());
    menuBar->Check(ID_LAYER_ACTION_POINTS, layers.actionPoints());
    menuBar->Check(ID_LAYER_ENTITY_HITBOXES, layers.entityHitboxes());
    menuBar->Check(ID_LAYER_ORIGIN, layers.origin());
    menuBar->Check(ID_LAYER_TILE_HITBOX, layers.tileHitbox());
}

void Frame::UpdateGuiUndo()
{
    wxMenuBar* menuBar = GetMenuBar();

    // ::TODO UndoStack::
    const bool canUndo = false;
    const bool canRedo = false;
    const bool isDirty = _controller.hasDocument();
    const bool hasFile = _controller.hasDocument();

    if (hasFile && canUndo) {
        menuBar->Enable(wxID_UNDO, true);
        // menuBar->SetLabel(wxID_UNDO, "&Undo " + undoStack.undoMessage() + "\tCTRL+Z");
    }
    else {
        menuBar->Enable(wxID_UNDO, false);
        menuBar->SetLabel(wxID_UNDO, "&Undo\tCTRL+Z");
    }

    if (hasFile && canRedo) {
        menuBar->Enable(wxID_REDO, true);
        // menuBar->SetLabel(wxID_REDO, "&Redo " + undoStack.redoMessage() + "\tCTRL+SHIFT+Z");
    }
    else {
        menuBar->Enable(wxID_REDO, false);
        menuBar->SetLabel(wxID_REDO, "&Redo\tCTRL+SHIFT+Z");
    }

    menuBar->Enable(wxID_SAVE, hasFile && isDirty);
}

void Frame::UpdateGuiTitle()
{
    // ::TODO UndoStack::
    const bool isDirty = _controller.hasDocument();

    if (_controller.hasDocument()) {
        const MS::FrameSet& frameSet = _controller.frameSetController().selected();

        if (isDirty) {
            SetTitle("*" + frameSet.name + ": " + WINDOW_NAME);
        }
        else {
            SetTitle(frameSet.name + ": " + WINDOW_NAME);
        }
    }
    else {
        SetTitle(WINDOW_NAME);
    }
}

void Frame::OnMenuNew(wxCommandEvent&)
{
    if (_controller.hasDocument()) {
        Frame* frame = new Frame();
        frame->_controller.newDocument();
        frame->Show(true);
    }
    else {
        _controller.newDocument();
    }
}

void Frame::OnMenuOpen(wxCommandEvent&)
{
    auto fn = openFileDialog(this, FRAMESET_DOCUMENT_TYPE);

    if (fn) {
        auto& fsController = _controller.frameSetController();

        if (fsController.hasSelected()) {
            CreateOpen(fn.value());
        }
        else {
            _controller.loadDocument(fn.value());
        }
    }
}

bool Frame::SaveDocument()
{
    if (_controller.hasDocument()) {
        bool s = _controller.saveDocument();
        if (s) {
            return true;
        }
        else {
            return SaveDocumentAs();
        }
    }

    return false;
}

bool Frame::SaveDocumentAs()
{
    auto& fsController = _controller.frameSetController();

    if (fsController.hasSelected()) {
        auto fn = saveFileDialog(this, FRAMESET_DOCUMENT_TYPE,
                                 _controller.filename());

        if (fn) {
            return _controller.saveDocument(fn.value());
        }
    }

    return false;
}
