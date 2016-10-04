#pragma once

#include "models/metasprite/frameset-exportorder.h"
#include <sigc++/signal.h>
#include <wx/artprov.h>
#include <wx/imaglist.h>
#include <wx/treectrl.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace Common {

template <class BaseControllerT>
class ExportOrderTreeCtrl : public wxTreeCtrl {
    enum TreeIcon {
        CROSS_ICON,
        TICK_ICON,
        WARNING_ICON,
    };

public:
    ExportOrderTreeCtrl(wxWindow* parent, int wxWindowID,
                        BaseControllerT& controller)
        : wxTreeCtrl(parent, wxWindowID,
                     wxDefaultPosition, wxDefaultSize,
                     wxTR_DEFAULT_STYLE | wxTR_SINGLE | wxTR_HIDE_ROOT)
        , _controller(controller)
    {
// Export Tree Icons
// -----------------

#ifdef __WXMSW__
        const wxSize asize = wxArtProvider::GetNativeSizeHint(wxART_LIST);
        auto* imageList = new wxImageList(asize.GetWidth(), asize.GetHeight(), true, 3);
        imageList->Add(wxArtProvider::GetBitmap("wxART_CROSS_MARK", wxART_LIST, asize));
        imageList->Add(wxArtProvider::GetBitmap("wxART_TICK_MARK", wxART_LIST, asize));
        imageList->Add(wxArtProvider::GetBitmap("wxART_WARNING", wxART_LIST, asize));
#else
        auto* imageList = new wxImageList();
        imageList->Add(wxArtProvider::GetBitmap("wxART_CROSS_MARK", wxART_TOOLBAR));
        imageList->Add(wxArtProvider::GetBitmap("wxART_TICK_MARK", wxART_TOOLBAR));
        imageList->Add(wxArtProvider::GetBitmap("wxART_WARNING", wxART_TOOLBAR));
#endif

        this->AssignImageList(imageList);

        // Signals
        // -------
        auto& aniController = _controller.animationControllerInterface();

        _controller.frameSetController().signal_anyChanged().connect(sigc::mem_fun(
            *this, &ExportOrderTreeCtrl::BuildTree));

        _controller.frameController().signal_mapChanged().connect(sigc::mem_fun(
            *this, &ExportOrderTreeCtrl::UpdateTreeFrames));

        aniController.animationController().signal_anyChanged().connect(sigc::mem_fun(
            *this, &ExportOrderTreeCtrl::UpdateTreeAnimations));

        aniController.instructionController().signal_anyChanged().connect(sigc::mem_fun(
            *this, &ExportOrderTreeCtrl::UpdateTreeAnimations));
    }

    void BuildTree()
    {
        const static wxString ALT_STR = "Alt: ";
        const static wxString FLIP_MAP[4] = { " (no Flip)",
                                              " (vFlip)",
                                              " (hFlip)",
                                              " (hvFlip)" };

        using ExportName = UnTech::MetaSprite::FrameSetExportOrder::ExportName;

        const auto& frameSet = _controller.frameSetController().selected();

        if (frameSet.exportOrder != nullptr) {
            auto loadExportNames = [this](wxTreeItemId tItem,
                                          const ExportName::list_t& exportNames) {

                this->DeleteChildren(tItem);

                for (const auto& en : exportNames) {
                    auto row = this->AppendItem(tItem, en.name.str());

                    for (const auto& alt : en.alternatives) {
                        int i = (alt.hFlip & 1) << 1 | (alt.vFlip & 1);

                        this->AppendItem(row, ALT_STR + alt.name + FLIP_MAP[i]);
                    }
                }
            };

            this->Freeze();

            if (this->IsEmpty()) {
                auto root = this->AddRoot("Export Order");
                this->AppendItem(root, "Still Frames");
                this->AppendItem(root, "Animations");
            }

            wxTreeItemIdValue cookie;
            auto root = this->GetRootItem();
            auto firstChild = this->GetFirstChild(root, cookie);
            auto secondChild = this->GetNextChild(root, cookie);

            loadExportNames(firstChild, frameSet.exportOrder->stillFrames);
            loadExportNames(secondChild, frameSet.exportOrder->animations);

            this->Thaw();

            UpdateTreeFrames();
            UpdateTreeAnimations();
        }
        else {
            this->DeleteAllItems();
        }
    }

    void UpdateTreeFrames()
    {
        using ExportName = UnTech::MetaSprite::FrameSetExportOrder::ExportName;

        const auto& frameSet = _controller.frameSetController().selected();
        const auto& exportOrder = frameSet.exportOrder;

        if (exportOrder == nullptr) {
            return;
        }

        if (this->IsEmpty()) {
            BuildTree();
        }

        this->Freeze();

        wxTreeItemIdValue rootCookie;
        auto root = this->GetRootItem();

        auto frameNode = this->GetFirstChild(root, rootCookie);

        TreeIcon totalItemId = TICK_ICON;

        wxTreeItemIdValue cookie;
        auto node = this->GetFirstChild(frameNode, cookie);

        for (const ExportName& en : exportOrder->stillFrames) {
            TreeIcon itemId = CROSS_ICON;
            if (frameSet.frames.contains(en.name)) {
                itemId = TICK_ICON;
            }
            else {
                for (const auto& alt : en.alternatives) {
                    if (frameSet.frames.contains(alt.name)) {
                        itemId = TICK_ICON;
                        break;
                    }
                }
            }
            this->SetItemImage(node, itemId);

            if (itemId == CROSS_ICON) {
                totalItemId = CROSS_ICON;
            }

            node = this->GetNextChild(frameNode, cookie);
        }

        this->SetItemImage(frameNode, totalItemId);

        this->Thaw();
    }

    void UpdateTreeAnimations()
    {
        using ExportName = UnTech::MetaSprite::FrameSetExportOrder::ExportName;

        const auto& frameSet = _controller.frameSetController().selected();
        const auto& exportOrder = frameSet.exportOrder;

        if (exportOrder == nullptr) {
            return;
        }

        if (this->IsEmpty()) {
            BuildTree();
        }

        this->Freeze();

        wxTreeItemIdValue rootCookie;
        auto root = this->GetRootItem();

        this->GetFirstChild(root, rootCookie);
        auto aniNode = this->GetNextChild(root, rootCookie);

        TreeIcon totalItemId = TICK_ICON;

        wxTreeItemIdValue cookie;
        auto node = this->GetFirstChild(aniNode, cookie);

        const auto& animations = frameSet.animations;

        for (const ExportName& en : exportOrder->animations) {
            TreeIcon itemId = CROSS_ICON;
            if (animations.contains(en.name)) {
                // ::TODO Animation.isValid::
                // itemId = animations.at(en.name).isValid() ? TICK_ICON : WARNING_ICON;
                itemId = WARNING_ICON;
            }
            else {
                for (const auto& alt : en.alternatives) {
                    if (animations.contains(alt.name)) {
                        // ::TODO Animation.isValid::
                        // itemId = animations.at(alt.name).isValid() ? TICK_ICON : WARNING_ICON;
                        itemId = WARNING_ICON;
                        break;
                    }
                }
            }
            this->SetItemImage(node, itemId);

            if (itemId == CROSS_ICON) {
                totalItemId = CROSS_ICON;
            }
            else if (itemId == WARNING_ICON && totalItemId != CROSS_ICON) {
                totalItemId = WARNING_ICON;
            }

            node = this->GetNextChild(aniNode, cookie);
        }

        this->SetItemImage(aniNode, totalItemId);

        this->Thaw();
    }

private:
    BaseControllerT& _controller;
};
}
}
}
}
