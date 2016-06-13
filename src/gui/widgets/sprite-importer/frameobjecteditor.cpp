#include "frameobjecteditor.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::SpriteImporter;

FrameObjectEditor::FrameObjectEditor(SI::SpriteImporterController& controller)
    : widget()
    , _controller(controller)
    , _locationSpinButtons()
    , _sizeCombo()
    , _locationLabel(_("Location:"), Gtk::ALIGN_START)
    , _locationCommaLabel(" ,  ")
    , _sizeLabel(_("Size:"), Gtk::ALIGN_START)
    , _updatingValues(false)
{
    _sizeCombo.append(_("small"));
    _sizeCombo.append(_("large"));
    _sizeCombo.set_active(0);

    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    widget.attach(_locationLabel, 0, 0, 1, 1);
    widget.attach(_locationSpinButtons.xSpin, 1, 0, 1, 1);
    widget.attach(_locationCommaLabel, 2, 0, 1, 1);
    widget.attach(_locationSpinButtons.ySpin, 3, 0, 1, 1);

    widget.attach(_sizeLabel, 0, 1, 1, 1);
    widget.attach(_sizeCombo, 1, 1, 3, 1);

    updateGuiValues();

    /*
     * SLOTS
     * =====
     */

    // Controller Signals
    _controller.frameObjectController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameObjectEditor::updateGuiValues));

    _controller.frameObjectController().signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FrameObjectEditor::updateGuiValues));

    _controller.frameController().signal_frameSizeChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameObjectEditor::updateRange)));

    _controller.frameSetController().signal_gridChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FrameObjectEditor::updateRange)));

    /** Set location signal */
    _locationSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.frameObjectController().selected_setLocation_merge(
                _locationSpinButtons.value());
        }
    });
    _locationSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller, &SI::SpriteImporterController::dontMergeNextAction)));

    /** Set Size Signal */
    _sizeCombo.signal_changed().connect([this](void) {
        typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

        if (!_updatingValues) {
            auto size = _sizeCombo.get_active_row_number() == 0 ? OS::SMALL : OS::LARGE;

            _controller.frameObjectController().selected_setLocationAndSize(
                _locationSpinButtons.value(),
                size);
        }
    });
}

void FrameObjectEditor::updateGuiValues()
{
    typedef SI::FrameObject::ObjectSize OS;

    const SI::FrameObject* frameObject = _controller.frameObjectController().selected();

    if (frameObject) {
        _updatingValues = true;

        _locationSpinButtons.set_range(frameObject->frame().locationSize(), frameObject->sizePx());
        _locationSpinButtons.set_value(frameObject->location());
        _sizeCombo.set_active(frameObject->size() == OS::LARGE ? 1 : 0);

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        static const upoint zeroPoint(0, 0);

        _locationSpinButtons.set_value(zeroPoint);
        _sizeCombo.set_active_text("");

        widget.set_sensitive(false);
    }
}

void FrameObjectEditor::updateRange()
{
    const SI::FrameObject* frameObject = _controller.frameObjectController().selected();

    if (frameObject) {
        _locationSpinButtons.set_range(frameObject->frame().locationSize(), frameObject->sizePx());
    }
}
