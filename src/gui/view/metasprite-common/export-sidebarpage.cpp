#include "export-sidebarpage.h"
#include "gui/view/common/filedialogs.h"
#include "models/metasprite-common/framesetexportorder.h"
#include <wx/artprov.h>
#include <wx/imaglist.h>

using namespace UnTech::View::MetaSpriteCommon;

// EXPORT SIDEBAR
// ==============

ExportSidebarPage::ExportSidebarPage(wxWindow* parent, int wxWindowID,
                                     MSC::AbstractFrameSetController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(4, 2, defBorder, defBorder * 2);
    sizer->Add(grid, wxSizerFlags().Expand().Border());

    grid->AddGrowableCol(1, 1);

    grid->Add(new wxStaticText(this, wxID_ANY, wxT("Export Order:")));
    _exportOrder = new TextAndToggleButtonCtrl(this, wxID_ANY);
    grid->Add(_exportOrder, wxSizerFlags().Expand());

    _frameSetType = new wxTextCtrl(this, wxID_ANY);
    _frameSetType->Disable();
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("FrameSet Type:")));
    grid->Add(_frameSetType, wxSizerFlags().Expand());

    auto* frameSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Export Order");
    sizer->Add(frameSizer, wxSizerFlags(1).Expand().Border());

    _exportTree = new wxTreeCtrl(this, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTR_DEFAULT_STYLE | wxTR_SINGLE | wxTR_HIDE_ROOT);
    frameSizer->Add(_exportTree, wxSizerFlags(1).Expand().Border());

    auto* paraSizer = new wxStaticBoxSizer(wxVERTICAL, this, "FrameSet Parameters");
    sizer->Add(paraSizer, wxSizerFlags(1).Expand().Border());

    // ::TODO frame Properties::

    // Export Tree Icons
    // -----------------
    {
        auto* imageList = new wxImageList();

        imageList->Add(wxArtProvider::GetBitmap("wxART_CROSS_MARK", wxART_TOOLBAR));
        imageList->Add(wxArtProvider::GetBitmap("wxART_TICK_MARK", wxART_TOOLBAR));

        _exportTree->AssignImageList(imageList);
    }

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &ExportSidebarPage::UpdateGui));

    _controller.signal_dataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &ExportSidebarPage::UpdateGui)));

    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &ExportSidebarPage::BuildGuiTree));

    _controller.signal_exportOrderChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &ExportSidebarPage::BuildGuiTree)));

    controller.animationController().signal_listChanged().connect(sigc::mem_fun(
        *this, &ExportSidebarPage::UpdateGuiTree));

    controller.animationController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &ExportSidebarPage::UpdateGuiTree)));

    controller.animationController().signal_itemRenamed().connect(sigc::hide(sigc::mem_fun(
        *this, &ExportSidebarPage::UpdateGuiTree)));

    // Slots
    // -----
    _slot_frameNameChanged.connect(sigc::mem_fun(
        *this, &ExportSidebarPage::UpdateGuiTree));

    // Events
    // ------
    _exportOrder->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
        namespace FSEO = MSC::FrameSetExportOrder;

        const MSC::AbstractFrameSet* frameSet = _controller.selected();

        if (frameSet && _exportOrder->GetButtonValue()) {
            auto fn = openFileDialog(this,
                                     FSEO::ExportOrderDocument::DOCUMENT_TYPE,
                                     frameSet->exportOrderDocument().get());
            if (fn) {
                _controller.selected_setExportOrderFilename(fn.value());
            }

            _exportOrder->SetButtonValue(false);
        }
    });
}

void ExportSidebarPage::UpdateGui()
{
    const MSC::AbstractFrameSet* frameSet = _controller.selected();

    if (frameSet) {
        this->Enable();

        const auto& fseoDocument = frameSet->exportOrderDocument();
        if (fseoDocument) {
            _exportOrder->ChangeTextValue(fseoDocument->filename());
            _frameSetType->ChangeValue(fseoDocument->exportOrder().name());
        }
        else {
            _exportOrder->ChangeTextValue(wxEmptyString);
            _frameSetType->ChangeValue(wxEmptyString);
        }
    }
    else {
        this->Disable();

        _exportOrder->ChangeTextValue(wxEmptyString);
        _frameSetType->ChangeValue(wxEmptyString);
    }
}

void ExportSidebarPage::BuildGuiTree()
{
    const static wxString ALT_STR = "Alt: ";
    const static wxString FLIP_MAP[4] = { " (no Flip)",
                                          " (vFlip)",
                                          " (hFlip)",
                                          " (hvFlip)" };

    namespace FSEO = UnTech::MetaSpriteCommon::FrameSetExportOrder;

    // REMEMBER: that ExportOrder is read only
    const MSC::AbstractFrameSet* frameSet = _controller.selected();

    if (frameSet && frameSet->exportOrderDocument()) {
        auto loadExportNames = [this](wxTreeItemId tItem,
                                      const FSEO::ExportName::list_t& exportNames) {

            _exportTree->DeleteChildren(tItem);

            for (const auto& it : exportNames) {
                auto row = _exportTree->AppendItem(tItem, it.first);

                for (const auto& alt : it.second.alternativeNames()) {
                    int i = (alt.hFlip() & 1) << 1 | (alt.vFlip() & 1);

                    _exportTree->AppendItem(row, ALT_STR + alt.name() + FLIP_MAP[i]);
                }
            }
        };

        _exportTree->Freeze();

        if (_exportTree->IsEmpty()) {
            auto root = _exportTree->AddRoot("Export Order");
            _exportTree->AppendItem(root, "Still Frames");
            _exportTree->AppendItem(root, "Animations");
        }

        const auto& exportOrder = frameSet->exportOrderDocument()->exportOrder();

        wxTreeItemIdValue cookie;
        auto root = _exportTree->GetRootItem();
        auto firstChild = _exportTree->GetFirstChild(root, cookie);
        auto secondChild = _exportTree->GetNextChild(root, cookie);

        loadExportNames(firstChild, exportOrder.stillFrames());
        loadExportNames(secondChild, exportOrder.animations());

        _exportTree->Thaw();

        UpdateGuiTree();
    }
    else {
        _exportTree->DeleteAllItems();
    }
}

void ExportSidebarPage::UpdateGuiTree()
{
    const int CROSS_ICON = 0;
    const int TICK_ICON = 1;

    // REMEMBER: that ExportOrder is read only
    const MSC::AbstractFrameSet* frameSet = _controller.selected();

    if (frameSet == nullptr) {
        return;
    }
    if (frameSet->exportOrderDocument() == nullptr) {
        return;
    }

    if (_exportTree->IsEmpty()) {
        BuildGuiTree();
    }

    _exportTree->Freeze();

    const auto& exportOrder = frameSet->exportOrderDocument()->exportOrder();

    wxTreeItemIdValue rootCookie;
    auto root = _exportTree->GetRootItem();
    {
        wxTreeItemIdValue cookie;

        auto frameNode = _exportTree->GetFirstChild(root, rootCookie);
        auto node = _exportTree->GetFirstChild(frameNode, cookie);

        int totalItemId = TICK_ICON;

        for (const auto& it : exportOrder.stillFrames()) {
            int itemId = CROSS_ICON;
            if (frameSet->containsFrameName(it.first)) {
                itemId = TICK_ICON;
            }
            else {
                for (const auto& alt : it.second.alternativeNames()) {
                    if (frameSet->containsFrameName(alt.name())) {
                        itemId = TICK_ICON;
                        break;
                    }
                }
            }
            if (itemId == CROSS_ICON) {
                totalItemId = CROSS_ICON;
            }
            _exportTree->SetItemImage(node, itemId);

            node = _exportTree->GetNextChild(frameNode, cookie);
        }

        _exportTree->SetItemImage(frameNode, totalItemId);
    }
    {
        wxTreeItemIdValue cookie;

        auto aniNode = _exportTree->GetNextChild(root, rootCookie);
        auto node = _exportTree->GetFirstChild(aniNode, cookie);

        const auto& animations = frameSet->animations();

        int totalItemId = TICK_ICON;

        for (const auto& it : exportOrder.animations()) {
            int itemId = CROSS_ICON;
            if (animations.nameExists(it.first)) {
                itemId = TICK_ICON;
            }
            else {
                for (const auto& alt : it.second.alternativeNames()) {
                    if (animations.nameExists(alt.name())) {
                        itemId = TICK_ICON;
                        break;
                    }
                }
            }
            if (itemId == CROSS_ICON) {
                totalItemId = CROSS_ICON;
            }
            _exportTree->SetItemImage(node, itemId);

            node = _exportTree->GetNextChild(aniNode, cookie);
        }

        _exportTree->SetItemImage(aniNode, totalItemId);
    }

    _exportTree->Thaw();
}
