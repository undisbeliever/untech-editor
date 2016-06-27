#pragma once

#include "animationlists.h"
#include "gui/controllers/metasprite-common.h"
#include "gui/widgets/common/enumclasscombobox.h"
#include "gui/widgets/common/namedlistnameentry.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSpriteCommon {

namespace MSC = UnTech::MetaSpriteCommon;

class AnimationEditor {
public:
    AnimationEditor(MSC::AbstractFrameSetController&);

protected:
    void updateGuiValues();

    void instruction_setFrame();
    void instruction_setParameter();

public:
    Gtk::Box widget;

private:
    MSC::AbstractFrameSetController& _controller;

    AnimationListEditor _animationListEditor;
    AnimationInstructionListEditor _instructionListEditor;

    Gtk::Frame _instructionsFrame;
    Gtk::Box _instructionsBox;
    Gtk::Grid _instructionsGrid;

    EnumClassComboBox<MSC::AnimationBytecode> _operationCombo;
    NamedListNameEntry _frameNameEntry;
    Gtk::ComboBoxText _frameFlipCombo;
    Gtk::Entry _parameterEntry;
    Gtk::Label _parameterMeaning;

    Gtk::Label _operationLabel;
    Gtk::Label _frameLabel;
    Gtk::Label _parameterLabel;

    bool _updatingValues;
};
}
}
}
