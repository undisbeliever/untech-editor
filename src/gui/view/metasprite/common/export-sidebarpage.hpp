#pragma once
#include "exportordertreectrl.hpp"
#include "gui/view/common/filedialogs.h"
#include "gui/view/common/textandtogglebuttonctrl.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace Common {

template <class BaseControllerT>
class ExportSidebarPage : public wxPanel {
public:
    ExportSidebarPage(wxWindow* parent, int wxWindowID,
                      BaseControllerT& controller)
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

        _exportTree = new ExportOrderTreeCtrl<BaseControllerT>(this, wxID_ANY, controller);
        frameSizer->Add(_exportTree, wxSizerFlags(1).Expand().Border());

        auto* paraSizer = new wxStaticBoxSizer(wxVERTICAL, this, "FrameSet Parameters");
        sizer->Add(paraSizer, wxSizerFlags(1).Expand().Border());

        // ::TODO frame Properties::

        // Signals
        // -------
        _controller.frameSetController().signal_anyChanged().connect(sigc::mem_fun(
            *this, &ExportSidebarPage::UpdateGui));

        // Events
        // ------
        _exportOrder->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
            using FSEO = UnTech::MetaSprite::FrameSetExportOrder;

            const auto& frameSet = _controller.frameSetController().selected();

            if (_exportOrder->GetButtonValue()) {
                const static DocumentType documentType = { "Frame Set Export File", FSEO::FILE_EXTENSION };

                std::string oldFileName;
                if (frameSet.exportOrder != nullptr) {
                    oldFileName = frameSet.exportOrder->filename;
                }

                auto fn = openFileDialog(this, documentType, oldFileName);

                if (fn) {
                    auto& frameSetController = _controller.frameSetController();
                    frameSetController.selected_setExportOrderFilename(fn.value());
                }

                _exportOrder->SetButtonValue(false);
            }
        });
    }

    void UpdateGui()
    {
        this->Enable(_controller.frameSetController().hasSelected());

        const auto& frameSet = _controller.frameSetController().selected();
        const auto& exportOrder = frameSet.exportOrder;

        if (exportOrder != nullptr) {
            _exportOrder->ChangeTextValue(exportOrder->filename);
            _frameSetType->ChangeValue(exportOrder->name.str());
        }
        else {
            _exportOrder->ChangeTextValue(wxEmptyString);
            _frameSetType->ChangeValue(wxEmptyString);
        }
    }

private:
    BaseControllerT& _controller;

    TextAndToggleButtonCtrl* _exportOrder;
    wxTextCtrl* _frameSetType;

    ExportOrderTreeCtrl<BaseControllerT>* _exportTree;

    sigc::signal<void> _slot_frameNameChanged;
};
}
}
}
}
