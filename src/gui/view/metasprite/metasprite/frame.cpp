/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frame.h"
#include "addtilesdialog.h"
#include "graphicspanel.h"
#include "sidebar.h"
#include "gui/view/defaults.h"
#include "gui/view/metasprite/animation/preview-msrenderer.h"
#include "gui/view/metasprite/framehelper.hpp"

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

const wxString Frame::WINDOW_NAME = "UnTech MetaSprite Editor";

const DocumentType Frame::FRAMESET_DOCUMENT_TYPE = {
    "MetaSprite FrameSet",
    // ::BUGFIX can't use MS::FrameSet::FILE_EXTENSION with g++ -O2::
    "utms"
};

enum CustomIds {
    ID_ADD_TILES = MenuIds::LAST_ID,
    ID_SPLIT_VIEW,
    ID_CENTER_VIEW,

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
    , _dontMergeFocusHack(_controller)
    , _frameHelper(this, _controller)
{
    SetMinSize(MIN_FRAME_SIZE);

    // Widgets
    // =======
    {
        auto* sizer = new wxBoxSizer(wxHORIZONTAL);

        auto* notebook = new wxNotebook(this, wxID_ANY,
                                        wxDefaultPosition, wxDefaultSize,
                                        wxNB_BOTTOM);

        sizer->Add(notebook, wxSizerFlags(1).Expand().Border());

        _graphics = new GraphicsPanel(notebook, wxID_ANY, _controller);
        notebook->AddPage(_graphics, "Frame Editor");

        _animationPreview = new Animation::PreviewPanel(
            notebook, wxID_ANY,
            _controller.animationControllerInterface(),
            _controller.settingsController(),
            std::make_unique<Animation::PreviewMsRenderer>(_controller));
        notebook->AddPage(_animationPreview, "Animation Preview");

        _sidebar = new Sidebar(this, wxID_ANY, _controller);
        sizer->Add(_sidebar, wxSizerFlags().Expand().Border(wxTOP | wxBOTTOM | wxRIGHT));

        this->SetSizer(sizer);

        Centre();
    }

    // Events
    // ======
    _dontMergeFocusHack.BindEventRecursive(this);

    // Extra Menus
    // ===========
    {
        auto* menuBar = GetMenuBar();
        auto* edit = menuBar->GetMenu(1);

        edit->AppendSeparator();
        edit->Append(ID_ADD_TILES, "Add Tiles");

        auto* view = menuBar->GetMenu(2);
        view->AppendCheckItem(ID_SPLIT_VIEW, "&Split View\tCTRL+T");
        view->Append(ID_CENTER_VIEW, "&Centre View\tCTRL+E");

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
