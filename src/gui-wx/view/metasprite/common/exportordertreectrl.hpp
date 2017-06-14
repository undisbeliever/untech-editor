/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

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

        // Events
        // ------
        this->Bind(wxEVT_TREE_ITEM_RIGHT_CLICK,
                   &ExportOrderTreeCtrl::OnListItemRightClick, this);

        // Signals
        // -------
        auto& aniController = _controller.animationControllerInterface();

        _controller.frameSetController().signal_anyChanged().connect(sigc::mem_fun(
            *this, &ExportOrderTreeCtrl::BuildTree));

        _controller.frameController().signal_mapChanged().connect(sigc::mem_fun(
            *this, &ExportOrderTreeCtrl::UpdateTreeFrames));

        aniController.animationController().signal_anyChanged().connect(sigc::mem_fun(
            *this, &ExportOrderTreeCtrl::UpdateTreeAnimations));

        aniController.animationFrameController().signal_anyChanged().connect(sigc::mem_fun(
            *this, &ExportOrderTreeCtrl::UpdateTreeAnimations));
    }

    void BuildTree()
    {
        const static wxString ALT_STR = "Alt: ";

        using ExportName = UnTech::MetaSprite::FrameSetExportOrder::ExportName;

        const auto& frameSet = _controller.frameSetController().selected();

        if (frameSet.exportOrder != nullptr) {
            auto loadExportNames = [this](wxTreeItemId tItem,
                                          const ExportName::list_t& exportNames) {

                this->DeleteChildren(tItem);

                for (const auto& en : exportNames) {
                    auto row = this->AppendItem(tItem, en.name.str());

                    for (const auto& alt : en.alternatives) {
                        this->AppendItem(row, ALT_STR + alt.str());
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
                bool valid = animations.at(en.name).isValid(frameSet);
                itemId = valid ? TICK_ICON : WARNING_ICON;
            }
            else {
                for (const auto& alt : en.alternatives) {
                    if (animations.contains(alt.name)) {
                        bool valid = animations.at(alt.name).isValid(frameSet);
                        itemId = valid ? TICK_ICON : WARNING_ICON;
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

    int GetChildIndex(wxTreeItemId& parentNode, const wxTreeItemId& selected)
    {
        int pos = 0;

        wxTreeItemIdValue cookie;
        auto node = this->GetFirstChild(parentNode, cookie);

        while (node.IsOk()) {
            if (node == selected) {
                return pos;
            }

            pos++;
            node = this->GetNextChild(parentNode, cookie);
        };

        return -1;
    }

    void OnListItemRightClick(wxTreeEvent& event)
    {
        wxTreeItemId selected = event.GetItem();

        if (this->GetItemImage(selected) != CROSS_ICON) {
            // only show menu if exportOrder item doesn't exist
            return;
        }

        wxTreeItemIdValue rootCookie;
        auto root = this->GetRootItem();

        auto frameNode = this->GetFirstChild(root, rootCookie);
        int frameIndex = this->GetChildIndex(frameNode, selected);

        if (frameIndex >= 0) {
            GeneratePopupMenu(PopupMenuData::FRAME, frameIndex);
        }
        else {
            auto aniNode = this->GetNextChild(root, rootCookie);
            auto aniIndex = this->GetChildIndex(aniNode, selected);

            if (aniIndex >= 0) {
                GeneratePopupMenu(PopupMenuData::ANIMATION, aniIndex);
            }
        }
    }

private:
    class PopupMenuData : public wxObject {
        using ExportName = UnTech::MetaSprite::FrameSetExportOrder::ExportName;

    public:
        enum Type {
            FRAME,
            ANIMATION,
        };

    private:
        const ExportOrderTreeCtrl& parent;
        const Type type;
        const int index;

    public:
        PopupMenuData(ExportOrderTreeCtrl& parent, Type type, int index)
            : wxObject()
            , parent(parent)
            , type(type)
            , index(index)
        {
        }

        const ExportName& GetExportName() const
        {
            const auto& exportOrder = parent._controller.frameSetController().selected().exportOrder;

            assert(exportOrder != nullptr);

            if (type == PopupMenuData::FRAME) {
                return exportOrder->stillFrames.at(index);
            }
            else {
                return exportOrder->animations.at(index);
            }
        }

        Type GetType() const { return type; };
    };

    void GeneratePopupMenu(const typename PopupMenuData::Type type, int index)
    {
        auto menuData = std::make_unique<PopupMenuData>(*this, type, index);
        const auto& en = menuData->GetExportName();

        int id = wxID_HIGHEST;

        wxMenu menu;

        menu.Append(id, "Create " + en.name);

        for (const auto& alt : en.alternatives) {
            id++;
            menu.Append(id, "Create " + alt.name);
        }

        menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &ExportOrderTreeCtrl::OnPopupClick, this,
                  wxID_ANY, wxID_ANY, menuData.release());

        PopupMenu(&menu);
    }

    void OnPopupClick(wxCommandEvent& event)
    {
        int id = event.GetId() - wxID_HIGHEST;
        auto* menuData = dynamic_cast<PopupMenuData*>(event.GetEventUserData());

        if (menuData && id >= 0) {
            const auto& en = menuData->GetExportName();

            const idstring& toCreate = id == 0 ? en.name
                                               : en.alternatives.at(id - 1).name;

            if (menuData->GetType() == PopupMenuData::FRAME) {
                _controller.frameController().create(toCreate);
            }
            else {
                _controller.animationControllerInterface().animationController().create(toCreate);
            }
        }
    }

private:
    BaseControllerT& _controller;
};
}
}
}
}
