#include "frameobjecteditor.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;
typedef UnTech::Controller::BaseController BaseController;

FrameObjectEditor::FrameObjectEditor(MS::MetaSpriteController& msController)
    : widget()
    , _controller(msController.frameObjectController())
    , _fsController(msController.frameSetController())
    , _locationSpinButtons()
    , _tileIdSpinButton(Gtk::Adjustment::create(0.0, 0.0, 0.0, 1.0, 1.0, 0.0))
    , _sizeCombo()
    , _orderSpinButton(Gtk::Adjustment::create(2.0, 0.0, 3.0, 1.0, 1.0, 0.0))
    , _hFlipCB(_("hFlip"))
    , _vFlipCB(_("vFlip"))
    , _locationLabel(_("Location:"), Gtk::ALIGN_START)
    , _locationCommaLabel(" ,  ")
    , _tileIdLabel(_("Tile:"), Gtk::ALIGN_START)
    , _sizeLabel(_("Size:"), Gtk::ALIGN_START)
    , _orderLabel(_("Order:"), Gtk::ALIGN_START)
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

    widget.attach(_tileIdLabel, 0, 2, 1, 1);
    widget.attach(_tileIdSpinButton, 1, 2, 3, 1);

    widget.attach(_sizeLabel, 0, 3, 1, 1);
    widget.attach(_sizeCombo, 1, 3, 3, 1);

    widget.attach(_orderLabel, 0, 4, 1, 1);
    widget.attach(_orderSpinButton, 1, 4, 3, 1);

    widget.attach(_hFlipCB, 1, 5, 1, 1);
    widget.attach(_vFlipCB, 3, 5, 3, 1);

    updateGuiValues();

    /*
     * SLOTS
     * =====
     */

    /* Controller signals */
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FrameObjectEditor::updateGuiValues));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FrameObjectEditor::updateGuiValues));

    _fsController.signal_tileCountChanged().connect(
        [this](const MS::FrameSet* frameSet) {
            if (frameSet == _fsController.selected()) {
                updateTileIdRange();
            }
        });

    /** Set location signal */
    _locationSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.selected_setLocation_merge(_locationSpinButtons.value());
        }
    });
    _locationSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller.baseController(), &BaseController::dontMergeNextAction)));

    /** Set Tile ID signal */
    _tileIdSpinButton.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            _controller.selected_setTileId_merge(_tileIdSpinButton.get_value());
        }
    });
    _tileIdSpinButton.signal_focus_out_event().connect(sigc::hide(sigc::mem_fun(
        _controller.baseController(), &BaseController::dontMergeNextAction)));

    /** Set Size Signal */
    _sizeCombo.signal_changed().connect([this](void) {
        typedef UnTech::MetaSprite::FrameObject::ObjectSize OS;

        if (!_updatingValues) {
            auto size = _sizeCombo.get_active_row_number() == 0 ? OS::SMALL : OS::LARGE;
            _controller.selected_setSize(size);
        }
    });

    /** Set Order signal */
    _orderSpinButton.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            _controller.selected_setOrder(_orderSpinButton.get_value());
        }
    });

    /** Set hFlip signal */
    _hFlipCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            _controller.selected_setHFlip(_hFlipCB.get_active());
        }
    });

    /** Set vFlip signal */
    _vFlipCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            _controller.selected_setVFlip(_vFlipCB.get_active());
        }
    });
}

void FrameObjectEditor::updateTileIdRange()
{
    const MS::FrameObject* frameObject = _controller.selected();

    if (frameObject) {
        MS::FrameSet& frameSet = frameObject->frame().frameSet();

        int nTiles;
        if (frameObject->size() == MS::FrameObject::ObjectSize::SMALL) {
            nTiles = frameSet.smallTileset().size();
        }
        else {
            nTiles = frameSet.largeTileset().size();
        }
        _tileIdSpinButton.set_range(0, nTiles - 1);
    }
}

void FrameObjectEditor::updateGuiValues()
{
    typedef UnTech::MetaSprite::FrameObject::ObjectSize OS;

    const MS::FrameObject* frameObject = _controller.selected();

    if (frameObject) {
        _updatingValues = true;

        updateTileIdRange();

        _locationSpinButtons.set_value(frameObject->location());
        _tileIdSpinButton.set_value(frameObject->tileId());
        _sizeCombo.set_active(frameObject->size() == OS::LARGE ? 1 : 0);
        _orderSpinButton.set_value(frameObject->order());
        _hFlipCB.set_active(frameObject->hFlip());
        _vFlipCB.set_active(frameObject->vFlip());

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        static const ms8point zeroPoint(0, 0);

        _locationSpinButtons.set_value(zeroPoint);
        _tileIdSpinButton.set_value(0);
        _orderSpinButton.set_value(0);
        _hFlipCB.set_active(0);
        _hFlipCB.set_active(0);

        _sizeCombo.set_active_text("");

        widget.set_sensitive(false);
    }
}
