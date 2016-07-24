#include "export-sidebarpage.h"
#include "gui/view/common/filedialogs.h"
#include "models/metasprite-common/framesetexportorder.h"

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

    // ::TODO frame list changed signal - UpdateGuiTree::

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
    }
    else {
        _exportTree->DeleteAllItems();
    }
}
