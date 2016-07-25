#include "exportordertreectrl.h"
#include "models/metasprite-common/framesetexportorder.h"
#include <wx/artprov.h>
#include <wx/imaglist.h>

using namespace UnTech::View::MetaSpriteCommon;

ExportOrderTreeCtrl::ExportOrderTreeCtrl(wxWindow* parent, int wxWindowID,
                                         MSC::AbstractFrameSetController& controller)
    : wxTreeCtrl(parent, wxWindowID,
                 wxDefaultPosition, wxDefaultSize,
                 wxTR_DEFAULT_STYLE | wxTR_SINGLE | wxTR_HIDE_ROOT)
    , _controller(controller)
{
    // Export Tree Icons
    // -----------------
    auto* imageList = new wxImageList();

    imageList->Add(wxArtProvider::GetBitmap("wxART_CROSS_MARK", wxART_TOOLBAR));
    imageList->Add(wxArtProvider::GetBitmap("wxART_TICK_MARK", wxART_TOOLBAR));

    this->AssignImageList(imageList);

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &ExportOrderTreeCtrl::BuildTree));

    _controller.signal_exportOrderChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &ExportOrderTreeCtrl::BuildTree)));

    controller.animationController().signal_listChanged().connect(sigc::mem_fun(
        *this, &ExportOrderTreeCtrl::UpdateTree));

    controller.animationController().signal_itemRenamed().connect(sigc::hide(sigc::mem_fun(
        *this, &ExportOrderTreeCtrl::UpdateTree)));

    controller.animationController().signal_listDataChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &ExportOrderTreeCtrl::UpdateTree)));

    // Slots
    // -----
    _slot_frameNameChanged.connect(sigc::mem_fun(
        *this, &ExportOrderTreeCtrl::UpdateTree));
}

void ExportOrderTreeCtrl::BuildTree()
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

            this->DeleteChildren(tItem);

            for (const auto& it : exportNames) {
                auto row = this->AppendItem(tItem, it.first);

                for (const auto& alt : it.second.alternativeNames()) {
                    int i = (alt.hFlip() & 1) << 1 | (alt.vFlip() & 1);

                    this->AppendItem(row, ALT_STR + alt.name() + FLIP_MAP[i]);
                }
            }
        };

        this->Freeze();

        if (this->IsEmpty()) {
            auto root = this->AddRoot("Export Order");
            this->AppendItem(root, "Still Frames");
            this->AppendItem(root, "Animations");
        }

        const auto& exportOrder = frameSet->exportOrderDocument()->exportOrder();

        wxTreeItemIdValue cookie;
        auto root = this->GetRootItem();
        auto firstChild = this->GetFirstChild(root, cookie);
        auto secondChild = this->GetNextChild(root, cookie);

        loadExportNames(firstChild, exportOrder.stillFrames());
        loadExportNames(secondChild, exportOrder.animations());

        this->Thaw();

        UpdateTree();
    }
    else {
        this->DeleteAllItems();
    }
}

void ExportOrderTreeCtrl::UpdateTree()
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

    if (this->IsEmpty()) {
        BuildTree();
    }

    this->Freeze();

    const auto& exportOrder = frameSet->exportOrderDocument()->exportOrder();

    wxTreeItemIdValue rootCookie;
    auto root = this->GetRootItem();
    {
        wxTreeItemIdValue cookie;

        auto frameNode = this->GetFirstChild(root, rootCookie);
        auto node = this->GetFirstChild(frameNode, cookie);

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
            this->SetItemImage(node, itemId);

            node = this->GetNextChild(frameNode, cookie);
        }

        this->SetItemImage(frameNode, totalItemId);
    }
    {
        wxTreeItemIdValue cookie;

        auto aniNode = this->GetNextChild(root, rootCookie);
        auto node = this->GetFirstChild(aniNode, cookie);

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
            this->SetItemImage(node, itemId);

            node = this->GetNextChild(aniNode, cookie);
        }

        this->SetItemImage(aniNode, totalItemId);
    }

    this->Thaw();
}
