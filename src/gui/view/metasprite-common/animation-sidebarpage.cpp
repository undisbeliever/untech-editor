#include "animation-sidebarpage.h"
#include "animation-sidebarlists.hpp"
#include "gui/view/common/enumclasschoice.h"
#include "gui/view/common/namedlistnamectrl.h"

namespace UnTech {
namespace View {
namespace MetaSpriteCommon {

class AnimationInstructionPanel : public wxPanel {
public:
    AnimationInstructionPanel(wxWindow* parent, int wxWindowID,
                              MSC::AnimationInstructionController& controller);

private:
    void UpdateGui();

    void OnFrameChanged();
    void OnParameterChanged();

private:
    MSC::AnimationInstructionController& _controller;

    EnumClassChoice<MSC::AnimationBytecode>* _operation;
    wxStaticText* _frameLabel;
    NamedListNameCtrl* _frameName;
    wxChoice* _frameFlip;
    wxStaticText* _parameterLabel;
    wxTextCtrl* _parameter;
    wxStaticText* _parameterMeaning;
};
}
}
}

using namespace UnTech::View::MetaSpriteCommon;

// ANIMATION SIDEBAR
// =================

AnimationSidebarPage::AnimationSidebarPage(wxWindow* parent, int wxWindowID,
                                           MSC::AbstractFrameSetController& controller)
    : wxPanel(parent, wxWindowID)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    sizer->Add(new NamedListToolBar<MSC::Animation>(
                   this, wxID_ANY,
                   controller.animationController()),
               wxSizerFlags(0).Right().Border());

    sizer->Add(new NamedListCtrl<MSC::Animation>(
                   this, wxID_ANY,
                   controller.animationController()),
               wxSizerFlags(2).Expand().Border(wxLEFT | wxRIGHT));

    // Animation Bytecode
    auto* instSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Animation Bytecode");
    sizer->Add(instSizer, wxSizerFlags(3).Expand().Border());

    instSizer->Add(new OrderedListToolBar<MSC::AnimationInstruction>(
                       this, wxID_ANY,
                       controller.animationInstructionController()),
                   wxSizerFlags().Right().Border());

    instSizer->Add(new OrderedListCtrl<MSC::AnimationInstruction>(
                       this, wxID_ANY,
                       controller.animationInstructionController()),
                   wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

    instSizer->Add(new AnimationInstructionPanel(
                       this, wxID_ANY,
                       controller.animationInstructionController()),
                   wxSizerFlags().Expand().Border());
}

// ANIMATION INSTRUCTION
// =====================

AnimationInstructionPanel::AnimationInstructionPanel(wxWindow* parent, int wxWindowID,
                                                     MSC::AnimationInstructionController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(3, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _operation = new EnumClassChoice<MSC::AnimationBytecode>(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Operation:"));
    grid->Add(_operation, wxSizerFlags(1).Expand());

    wxArrayString flipChoices;
    flipChoices.Add(wxEmptyString);
    flipChoices.Add("hFlip");
    flipChoices.Add("vFlip");
    flipChoices.Add("hvFlip");

    _frameLabel = new wxStaticText(this, wxID_ANY, "Frame:");
    _frameName = new NamedListNameCtrl(this, wxID_ANY,
                                       wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                       wxTE_PROCESS_ENTER);
    _frameFlip = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, flipChoices);

    grid->Add(_frameLabel);
    {
        auto* box = new wxBoxSizer(wxHORIZONTAL);
        box->Add(_frameName, wxSizerFlags(1).Expand());
        box->Add(_frameFlip, wxSizerFlags(1).Expand().Border(wxLEFT));
        grid->Add(box, wxSizerFlags().Expand());
    }

    _parameterLabel = new wxStaticText(this, wxID_ANY, "Parameter:");
    _parameter = new wxTextCtrl(this, wxID_ANY,
                                wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                wxTE_PROCESS_ENTER);
    _parameterMeaning = new wxStaticText(this, wxID_ANY, wxEmptyString);

    grid->Add(_parameterLabel);
    {
        auto* box = new wxBoxSizer(wxHORIZONTAL);
        box->Add(_parameter, wxSizerFlags(1).Expand());
        box->Add(_parameterMeaning, wxSizerFlags(1).Expand().Border(wxLEFT));
        grid->Add(box, wxSizerFlags().Expand());
    }

    // Signals
    // -------
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &AnimationInstructionPanel::UpdateGui));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &AnimationInstructionPanel::UpdateGui));

    // Events
    // ------
    _operation->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        _controller.selected_setOperation(_operation->GetValue());
    });

    _frameName->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
        OnFrameChanged();
    });
    _frameName->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
        OnFrameChanged();
        e.Skip();
    });

    _frameFlip->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        OnFrameChanged();
    });

    _parameter->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
        OnParameterChanged();
    });
    _parameter->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
        OnParameterChanged();
        e.Skip();
    });

    // Convert spaces to underscores in Goto labels
    _parameter->Bind(wxEVT_TEXT, [this](wxCommandEvent& e) {
        const MSC::AnimationInstruction* inst = _controller.selected();

        if (inst && inst->operation().usesGotoLabel()) {
            bool changed = false;
            wxString text = _parameter->GetValue();

            for (auto c : text) {
                if (!UnTech::isNameCharValid(c)) {
                    c = '_';
                    changed = true;
                }
            }

            if (changed) {
                _parameter->ChangeValue(text);
            }

            e.Skip();
        }
    });
}

void AnimationInstructionPanel::UpdateGui()
{
    typedef MSC::AnimationBytecode::Enum BC;

    const MSC::AnimationInstruction* inst = _controller.selected();

    if (inst) {
        const MSC::AnimationBytecode& op = inst->operation();

        this->Enable();

        _operation->SetValue(op);

        if (op.usesFrame()) {
            const auto& fref = inst->frame();

            _frameLabel->Enable();

            _frameName->Enable();
            _frameName->ChangeValue(fref.frameName);

            _frameFlip->Enable();
            _frameFlip->SetSelection((fref.vFlip << 1) | fref.hFlip);
        }
        else {
            _frameLabel->Disable();

            _frameName->Disable();
            _frameName->ChangeValue(wxEmptyString);

            _frameFlip->Disable();
            _frameFlip->SetSelection(wxNOT_FOUND);
        }

        if (op.usesParameter()) {
            _parameterLabel->Enable();

            wxString v;
            v << inst->parameter();
            _parameter->Enable();
            _parameter->ChangeValue(v);

            _parameterMeaning->Enable();

            switch (op.value()) {
            case BC::SET_FRAME_AND_WAIT_FRAMES:
                _parameterMeaning->SetLabel("frames");
                break;

            case BC::SET_FRAME_AND_WAIT_TIME:
                _parameterMeaning->SetLabel(
                    wxString::Format("%d ms", inst->parameter() * 1000 / 75));
                break;

            case BC::SET_FRAME_AND_WAIT_XVECL:
            case BC::SET_FRAME_AND_WAIT_YVECL:
                _parameterMeaning->SetLabel(
                    wxString::Format("%0.3f px", (double)inst->parameter() / 32));
                break;

            default:
                break;
            }
        }
        else if (op.usesGotoLabel()) {
            _parameterLabel->Enable();

            _parameter->Enable();
            _parameter->ChangeValue(inst->gotoLabel());

            _parameterMeaning->Disable();
            _parameterMeaning->SetLabel(wxEmptyString);
        }
        else {
            _parameterLabel->Disable();

            _parameter->Disable();
            _parameter->ChangeValue(wxEmptyString);

            _parameterMeaning->Disable();
            _parameterMeaning->SetLabel(wxEmptyString);
        }
    }
    else {
        this->Disable();
    }
}

void AnimationInstructionPanel::OnFrameChanged()
{
    const MSC::AnimationInstruction* inst = _controller.selected();

    if (inst) {
        const MSC::AnimationBytecode& op = inst->operation();

        if (op.usesFrame()) {
            MSC::FrameReference ref;

            ref.frameName = _frameName->GetValue();

            int sel = _frameFlip->GetSelection();
            if (sel < 0 || sel > 3) {
                sel = 0;
            }
            ref.hFlip = sel & 0x01;
            ref.vFlip = sel & 0x02;

            _controller.selected_setFrame(ref);
        }
    }
}

void AnimationInstructionPanel::OnParameterChanged()
{
    const MSC::AnimationInstruction* inst = _controller.selected();

    if (inst) {
        const MSC::AnimationBytecode& op = inst->operation();

        if (op.usesParameter()) {
            long p;
            bool s = _parameter->GetValue().ToLong(&p);

            if (s && p > INT_MIN && p < INT_MAX) {
                _controller.selected_setParameter(p);
            }
            else {
                UpdateGui();
            }
        }
        else if (op.usesGotoLabel()) {
            _controller.selected_setGotoLabel(_parameter->GetValue().ToStdString());
        }
        else {
            UpdateGui();
        }
    }
}
