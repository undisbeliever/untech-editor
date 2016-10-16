#pragma once

#include "gui/view/common/aboutdialog.h"
#include "gui/view/common/filedialogs.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

enum MenuIds {
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
    ID_LAYER_FRAME_OBJECTS,
    ID_LAYER_ACTION_POINTS,
    ID_LAYER_ENTITY_HITBOXES,
    ID_LAYER_ORIGIN,
    ID_LAYER_TILE_HITBOX,
    ID_LAYER_SHOW_ALL,

    LAST_ID
};

template <class FrameT>
FrameHelper<FrameT>::FrameHelper(FrameT* frame, controller_type& controller)
    : _frame(frame)
    , _controller(controller)
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

    menuBar->Append(file, "&File");
    menuBar->Append(edit, "&Edit");
    menuBar->Append(view, "&View");

    _frame->SetMenuBar(menuBar);

    // EVENTS
    // ------

    // FILE
    menuBar->Bind(wxEVT_COMMAND_MENU_SELECTED,
                  &FrameHelper::OnMenuNew, this,
                  wxID_NEW);

    menuBar->Bind(wxEVT_COMMAND_MENU_SELECTED,
                  &FrameHelper::OnMenuOpen, this,
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
            AboutDialog dialog(_frame, FrameT::WINDOW_NAME);
            dialog.ShowModal();
        },
        wxID_ABOUT);

    menuBar->Bind(
        wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
            _frame->Close();
        },
        wxID_EXIT);

    // EDIT

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

    // Signals
    // =======

    _controller.selectedController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameHelper::UpdateGuiMenu));

    _controller.selectedController().signal_listChanged().connect(sigc::mem_fun(
        *this, &FrameHelper::UpdateGuiMenu));

    _controller.undoStack().signal_stackChanged().connect(sigc::mem_fun(
        *this, &FrameHelper::UpdateGuiUndo));

    _controller.undoStack().signal_dirtyBitChanged().connect(sigc::mem_fun(
        *this, &FrameHelper::UpdateGuiTitle));

    _controller.frameSetController().signal_dataChanged().connect(sigc::mem_fun(
        *this, &FrameHelper::UpdateGuiTitle));

    _controller.frameSetController().signal_selectedChanged().connect(
        [this](void) {
            UpdateGuiMenu();
            UpdateGuiZoom();
            UpdateGuiUndo();
            UpdateGuiTitle();
        });

    // Events
    // ======
    _frame->Bind(wxEVT_CLOSE_WINDOW, &FrameHelper::OnClose, this);

    // Cleanup
    // =======
    UpdateGuiUndo();
    UpdateGuiMenu();
    UpdateGuiZoom();
    UpdateGuiLayers();
}

template <class FrameT>
void FrameHelper<FrameT>::OnClose(wxCloseEvent& event)
{
    const bool isDirty = _controller.undoStack().isDirty();

    if (event.CanVeto() && isDirty) {
        int r = wxMessageBox("Do you want to save?\n\n"
                             "If you close without saving, your changes will be discarded.",
                             "Do you want to save?",
                             wxICON_QUESTION | wxCANCEL | wxYES | wxNO,
                             _frame);

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

template <class FrameT>
void FrameHelper<FrameT>::UpdateGuiMenu()
{
    wxMenuBar* menuBar = _frame->GetMenuBar();

    if (_controller.hasDocument()) {
        menuBar->EnableTop(1, true);
        menuBar->EnableTop(2, true);
        menuBar->Enable(wxID_SAVEAS, true);
    }
    else {
        menuBar->EnableTop(1, false);
        menuBar->EnableTop(2, false);
        menuBar->Enable(wxID_SAVE, false);
        menuBar->Enable(wxID_SAVEAS, false);
    }

    auto& selected = _controller.selectedController();

    menuBar->Enable(ID_CREATE, selected.canCreateSelected());
    menuBar->Enable(ID_CLONE, selected.canCloneSelected());
    menuBar->Enable(ID_REMOVE, selected.canRemoveSelected());
    menuBar->Enable(ID_MOVE_UP, selected.canMoveSelectedUp());
    menuBar->Enable(ID_MOVE_DOWN, selected.canMoveSelectedDown());
}

template <class FrameT>
void FrameHelper<FrameT>::UpdateGuiZoom()
{
    wxMenuBar* menuBar = _frame->GetMenuBar();
    auto& setting = _controller.settingsController().zoom();

    int zoom = setting.zoom();
    if (zoom >= 0 && zoom <= 9) {
        menuBar->Check(ID_ZOOM_1 - 1 + zoom, true);
    }
    menuBar->Check(ID_ASPECT_SQUARE + static_cast<int>(setting.aspectRatio()), true);
}

template <class FrameT>
void FrameHelper<FrameT>::UpdateGuiLayers()
{
    wxMenuBar* menuBar = _frame->GetMenuBar();
    auto& layers = _controller.settingsController().layers();

    menuBar->Check(ID_LAYER_FRAME_OBJECTS, layers.frameObjects());
    menuBar->Check(ID_LAYER_ACTION_POINTS, layers.actionPoints());
    menuBar->Check(ID_LAYER_ENTITY_HITBOXES, layers.entityHitboxes());
    menuBar->Check(ID_LAYER_ORIGIN, layers.origin());
    menuBar->Check(ID_LAYER_TILE_HITBOX, layers.tileHitbox());
}

template <class FrameT>
void FrameHelper<FrameT>::UpdateGuiUndo()
{
    wxMenuBar* menuBar = _frame->GetMenuBar();

    const auto& undoStack = _controller.undoStack();
    const bool hasFile = _controller.hasDocument();

    if (hasFile && undoStack.canUndo()) {
        menuBar->Enable(wxID_UNDO, true);
        menuBar->SetLabel(wxID_UNDO, "&Undo " + undoStack.undoMessage() + "\tCTRL+Z");
    }
    else {
        menuBar->Enable(wxID_UNDO, false);
        menuBar->SetLabel(wxID_UNDO, "&Undo\tCTRL+Z");
    }

    if (hasFile && undoStack.canRedo()) {
        menuBar->Enable(wxID_REDO, true);
        menuBar->SetLabel(wxID_REDO, "&Redo " + undoStack.redoMessage() + "\tCTRL+SHIFT+Z");
    }
    else {
        menuBar->Enable(wxID_REDO, false);
        menuBar->SetLabel(wxID_REDO, "&Redo\tCTRL+SHIFT+Z");
    }

    menuBar->Enable(wxID_SAVE, hasFile && undoStack.isDirty());
}

template <class FrameT>
void FrameHelper<FrameT>::UpdateGuiTitle()
{
    const auto& undoStack = _controller.undoStack();

    if (_controller.hasDocument()) {
        const auto& frameSet = _controller.frameSetController().selected();

        if (undoStack.isDirty()) {
            _frame->SetTitle("*" + frameSet.name + ": " + FrameT::WINDOW_NAME);
        }
        else {
            _frame->SetTitle(frameSet.name + ": " + FrameT::WINDOW_NAME);
        }
    }
    else {
        _frame->SetTitle(FrameT::WINDOW_NAME);
    }
}

template <class FrameT>
void FrameHelper<FrameT>::OnMenuNew(wxCommandEvent&)
{
    if (_controller.hasDocument()) {
        FrameT* frame = new FrameT();
        frame->Controller().newDocument();
        frame->Show(true);
    }
    else {
        _controller.newDocument();
    }
}

template <class FrameT>
void FrameHelper<FrameT>::OnMenuOpen(wxCommandEvent&)
{
    auto fn = openFileDialog(_frame, FrameT::FRAMESET_DOCUMENT_TYPE);

    if (fn) {
        auto& fsController = _controller.frameSetController();

        if (fsController.hasSelected()) {
            FrameT::CreateOpen(fn.value());
        }
        else {
            _controller.loadDocument(fn.value());
        }
    }
}

template <class FrameT>
bool FrameHelper<FrameT>::SaveDocument()
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
template <class FrameT>
bool FrameHelper<FrameT>::SaveDocumentAs()
{
    auto& fsController = _controller.frameSetController();

    if (fsController.hasSelected()) {
        auto fn = saveFileDialog(_frame, FrameT::FRAMESET_DOCUMENT_TYPE,
                                 _controller.filename());

        if (fn) {
            return _controller.saveDocument(fn.value());
        }
    }

    return false;
}
}
}
}
