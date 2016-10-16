#include "frame.h"
#include "framesetgraphicsctrl.h"
#include "sidebar.h"
#include "gui/view/common/aboutdialog.h"
#include "gui/view/common/filedialogs.h"
#include "gui/view/defaults.h"
#include "gui/view/metasprite/framehelper.hpp"

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace SpriteImporter {

const wxString Frame::WINDOW_NAME = "UnTech Sprite Importer";

const DocumentType Frame::FRAMESET_DOCUMENT_TYPE = {
    "Sprite Importer FrameSet",
    // ::BUGFIX can't use SI::FrameSet::FILE_EXTENSION with g++ -O2::
    "utsi"
};

enum CustomIds {
    ID_RELOAD_IMAGE = MenuIds::LAST_ID,

    ID_INIT_TIMER,
};
}
}
}
}

using namespace UnTech::View::MetaSprite::SpriteImporter;

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
        auto* graphics = new FrameSetGraphicsCtrl(this, wxID_ANY, _controller);
        auto* sidebar = new Sidebar(this, wxID_ANY, _controller);

        auto* sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->Add(graphics, wxSizerFlags(1).Expand().Border());
        sizer->Add(sidebar, wxSizerFlags().Expand().Border(wxTOP | wxBOTTOM | wxRIGHT));

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
        edit->Append(ID_RELOAD_IMAGE, "Reload Image\tCTRL+R");

        // EVENTS
        // ------
        menuBar->Bind(
            wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&) {
                _controller.frameSetController().selected_reloadImage();
            },
            ID_RELOAD_IMAGE);
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
     * Previously the "update" signal call was in the Sidebar's
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
