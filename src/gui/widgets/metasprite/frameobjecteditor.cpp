#include "frameobjecteditor.h"
#include "signals.h"
#include "gui/undo/actionhelper.h"
#include "gui/undo/mergeactionhelper.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;

// ::SHOULDO add a Boolean type::
// ::: would need undo and redo messages in Undo::Action ::

SIMPLE_UNDO_MERGE_ACTION(frameObject_merge_setLocation,
                         MS::FrameObject, UnTech::ms8point, location, setLocation,
                         Signals::frameObjectChanged,
                         "Move Frame Object")

SIMPLE_UNDO_MERGE_ACTION(frameObject_merge_setTileId,
                         MS::FrameObject, unsigned, tileId, setTileId,
                         Signals::frameObjectChanged,
                         "Change Frame Object Tile")

SIMPLE_UNDO_ACTION(frameObject_setSize,
                   MS::FrameObject, MS::FrameObject::ObjectSize, size, setSize,
                   Signals::frameObjectChanged,
                   "Change Frame Object Size")

SIMPLE_UNDO_ACTION(frameObject_setOrder,
                   MS::FrameObject, unsigned, order, setOrder,
                   Signals::frameObjectChanged,
                   "Change Frame Object Order")

SIMPLE_UNDO_ACTION(frameObject_setHFlip,
                   MS::FrameObject, bool, hFlip, setHFlip,
                   Signals::frameObjectChanged,
                   "Change Frame Object hFlip")

SIMPLE_UNDO_ACTION(frameObject_setVFlip,
                   MS::FrameObject, bool, vFlip, setVFlip,
                   Signals::frameObjectChanged,
                   "Change Frame Object vFlip")

FrameObjectEditor::FrameObjectEditor(Selection& selection)
    : widget()
    , _selection(selection)
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

    /* Update gui when selected object changed */
    _selection.signal_frameObjectChanged.connect(sigc::mem_fun(
        *this, &FrameObjectEditor::updateGuiValues));

    /* Update gui if object has changed */
    Signals::frameObjectChanged.connect([this](const MS::FrameObject* obj) {
        if (obj == _selection.frameObject()) {
            updateGuiValues();
        }
    });

    /* Update tileId range if The number of tiles in the tileset changed */
    Signals::frameSetTilesetCountChanged.connect([this](const MS::FrameSet* frameSet) {
        if (frameSet == _selection.frameSet()) {
            updateTileIdRange();
        }
    });

    /** Set location signal */
    _locationSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frameObject_merge_setLocation(_selection.frameObject(),
                                          _locationSpinButtons.value());
        }
    });
    _locationSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

    /** Set Tile ID signal */
    _tileIdSpinButton.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            frameObject_merge_setTileId(_selection.frameObject(),
                                        _tileIdSpinButton.get_value());
        }
    });
    _tileIdSpinButton.signal_focus_out_event().connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

    /** Set Size Signal */
    _sizeCombo.signal_changed().connect([this](void) {
        typedef UnTech::MetaSprite::FrameObject::ObjectSize OS;

        if (!_updatingValues) {
            auto size = _sizeCombo.get_active_row_number() == 0 ? OS::SMALL : OS::LARGE;
            frameObject_setSize(_selection.frameObject(), size);
        }
    });

    /** Set Order signal */
    _orderSpinButton.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            frameObject_setOrder(_selection.frameObject(),
                                 _orderSpinButton.get_value());
        }
    });

    /** Set hFlip signal */
    _hFlipCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            frameObject_setHFlip(_selection.frameObject(),
                                 _hFlipCB.get_active());
        }
    });

    /** Set vFlip signal */
    _vFlipCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            frameObject_setVFlip(_selection.frameObject(),
                                 _vFlipCB.get_active());
        }
    });
}

void FrameObjectEditor::updateTileIdRange()
{
    MS::FrameObject* frameObject = _selection.frameObject();

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

    const MS::FrameObject* frameObject = _selection.frameObject();

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
