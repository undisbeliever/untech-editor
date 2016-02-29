#include "framepropertieseditor.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

FramePropertiesEditor::FramePropertiesEditor()
    : widget()
    , _frame(nullptr)
    , _gridLocationSpinButtons()
    , _locationSpinButtons()
    , _originSpinButtons()
    , _tileHitboxSpinButtons()
    , _spriteOrderAdjustment(Gtk::Adjustment::create(2.0, 0.0, 3.0, 1.0, 1.0, 0.0))
    , _spriteOrderSpinButton(_spriteOrderAdjustment)
    , _useGridLocationCB(_("Use Grid Location"))
    , _useCustomOriginCB(_("Use Custom Origin"))
    , _solidCB(_("Solid Tile Hitbox"))
    , _emptySpace()
    , _gridLocationLabel(_("Grid Location:"), Gtk::ALIGN_START)
    , _gridLocationCommaLabel(" ,  ")
    , _locationLabel(_("Location:"), Gtk::ALIGN_START)
    , _locationCommaLabel(" , ")
    , _locationCrossLabel(" x ")
    , _originLabel(_("Origin:"), Gtk::ALIGN_START)
    , _originCommaLabel(" , ")
    , _tileHitboxLabel(_("Tile Hitbox:"), Gtk::ALIGN_START)
    , _tileHitboxCommaLabel(" , ")
    , _tileHitboxCrossLabel(" x ")
    , _spriteOrderLabel(_("Sprite Order:"))
{
    _emptySpace.set_size_request(GRID_CHILD_MARGIN_WIDTH, 2);

    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    widget.attach(_spriteOrderLabel, 0, 0, 2, 1);
    widget.attach(_spriteOrderSpinButton, 2, 0, 3, 1);

    widget.attach(_useGridLocationCB, 0, 1, 5, 1);

    widget.attach(_emptySpace, 0, 2, 1, 1);
    widget.attach(_gridLocationLabel, 1, 2, 1, 1);
    widget.attach(_gridLocationSpinButtons.xSpin, 2, 2, 1, 1);
    widget.attach(_gridLocationCommaLabel, 3, 2, 1, 1);
    widget.attach(_gridLocationSpinButtons.ySpin, 4, 2, 1, 1);

    widget.attach(_locationLabel, 1, 3, 1, 1);
    widget.attach(_locationSpinButtons.xSpin, 2, 3, 1, 1);
    widget.attach(_locationCommaLabel, 3, 3, 1, 1);
    widget.attach(_locationSpinButtons.ySpin, 4, 3, 1, 1);
    widget.attach(_locationSpinButtons.widthSpin, 2, 4, 1, 1);
    widget.attach(_locationCrossLabel, 3, 4, 1, 1);
    widget.attach(_locationSpinButtons.heightSpin, 4, 4, 1, 1);

    widget.attach(_useCustomOriginCB, 0, 5, 5, 1);

    widget.attach(_originLabel, 1, 6, 1, 1);
    widget.attach(_originSpinButtons.xSpin, 2, 6, 1, 1);
    widget.attach(_originCommaLabel, 3, 6, 1, 1);
    widget.attach(_originSpinButtons.ySpin, 4, 6, 1, 1);

    widget.attach(_solidCB, 0, 7, 5, 1);

    widget.attach(_tileHitboxLabel, 1, 8, 1, 1);
    widget.attach(_tileHitboxSpinButtons.xSpin, 2, 8, 1, 1);
    widget.attach(_tileHitboxCommaLabel, 3, 8, 1, 1);
    widget.attach(_tileHitboxSpinButtons.ySpin, 4, 8, 1, 1);
    widget.attach(_tileHitboxSpinButtons.widthSpin, 2, 9, 1, 1);
    widget.attach(_tileHitboxCrossLabel, 3, 9, 1, 1);
    widget.attach(_tileHitboxSpinButtons.heightSpin, 4, 9, 1, 1);

    updateGuiValues();

    /**
     * SLOTS
     */

    /** Grid location signal */
    _gridLocationSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frame) {
            _frame->setGridLocation(_gridLocationSpinButtons.value());
            Signals::frameChanged.emit(_frame);
        }
    });
    /** Location signal */
    _locationSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frame) {
            auto oldLocationSize = _frame->location().size();

            _frame->setLocation(_locationSpinButtons.value());
            Signals::frameChanged.emit(_frame);

            if (_frame->location().size() != oldLocationSize) {
                Signals::frameSizeChanged.emit(_frame);
            }
        }
    });
    /** Origin signal */
    _originSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frame) {
            _frame->setOrigin(_originSpinButtons.value());
            Signals::frameChanged.emit(_frame);
        }
    });
    /** Tile hitbox signal */
    _tileHitboxSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frame) {
            _frame->setTileHitbox(_tileHitboxSpinButtons.value());
            Signals::frameChanged.emit(_frame);
        }
    });
    /** Sprite Order Signal */
    _spriteOrderSpinButton.signal_value_changed().connect([this](void) {
        int i = _spriteOrderSpinButton.get_value_as_int();
        assert(i >= 0);
        assert(i <= 3);

        if (_frame) {
            _frame->setSpriteOrder((unsigned)i);
            Signals::frameChanged.emit(_frame);
        }
    });
    /** Use Grid Location Checkbox signal */
    _useGridLocationCB.signal_clicked().connect([this](void) {
        if (_frame) {
            _frame->setUseGridLocation(_useGridLocationCB.get_active());
            Signals::frameChanged.emit(_frame);
            Signals::frameSizeChanged.emit(_frame);
        }
    });
    /** Use Custom Origin Checkbox signal */
    _useCustomOriginCB.signal_clicked().connect([this](void) {
        if (_frame) {
            _frame->setUseGridOrigin(!(_useCustomOriginCB.get_active()));
            Signals::frameChanged.emit(_frame);
        }
    });
    /** Solid Checkbox signal */
    _solidCB.signal_clicked().connect([this](void) {
        if (_frame) {
            _frame->setSolid(_solidCB.get_active());
            Signals::frameChanged.emit(_frame);
        }
    });

    /** Frame Updated signal */
    Signals::frameChanged.connect([this](const std::shared_ptr<SI::Frame> frame) {
        if (_frame == frame) {
            updateGuiValues();
        }
    });

    /** FrameSet Grid Updated signal */
    Signals::frameSetGridChanged.connect([this](const std::shared_ptr<SI::FrameSet> fs) {
        if (_frame) {
            if (_frame->frameSet().lock() == fs) {
                updateGuiValues();
                Signals::frameSizeChanged.emit(_frame);
            }
        }
    });
}

void FramePropertiesEditor::updateGuiValues()
{
    if (_frame) {
        _gridLocationSpinButtons.set_value(_frame->gridLocation());
        _locationSpinButtons.set_value(_frame->location());
        _originSpinButtons.set_value(_frame->origin());
        _spriteOrderSpinButton.set_value(_frame->spriteOrder());
        _tileHitboxSpinButtons.set_value(_frame->tileHitbox());

        // ::TODO _gridLocationSpinButtons range based on image size::
        // ::TODO _locationSpinButtons range based on image size::
        _locationSpinButtons.set_minSize(_frame->minimumViableSize());
        _originSpinButtons.set_range(_frame->locationSize());
        _tileHitboxSpinButtons.set_range(_frame->locationSize());

        bool useGridLocation = _frame->useGridLocation();
        _useGridLocationCB.set_active(useGridLocation);

        _gridLocationSpinButtons.set_sensitive(useGridLocation);
        _gridLocationLabel.set_sensitive(useGridLocation);
        _gridLocationCommaLabel.set_sensitive(useGridLocation);

        _locationSpinButtons.set_sensitive(!useGridLocation);
        _locationLabel.set_sensitive(!useGridLocation);
        _locationCommaLabel.set_sensitive(!useGridLocation);
        _locationCrossLabel.set_sensitive(!useGridLocation);

        bool useCustomOrigin = !_frame->useGridOrigin();
        _useCustomOriginCB.set_active(useCustomOrigin);

        _originSpinButtons.set_sensitive(useCustomOrigin);
        _originLabel.set_sensitive(useCustomOrigin);
        _originCommaLabel.set_sensitive(useCustomOrigin);

        bool solid = _frame->solid();
        _solidCB.set_active(solid);

        _tileHitboxSpinButtons.set_sensitive(solid);
        _tileHitboxLabel.set_sensitive(solid);
        _tileHitboxCommaLabel.set_sensitive(solid);
        _tileHitboxCrossLabel.set_sensitive(solid);

        widget.set_sensitive(true);
    }
    else {
        static const upoint zeroPoint = { 0, 0 };
        static const urect zeroRect = { 0, 0, 0, 0 };

        _gridLocationSpinButtons.set_value(zeroPoint);
        _locationSpinButtons.set_value(zeroRect);
        _originSpinButtons.set_value(zeroPoint);
        _spriteOrderSpinButton.set_value(0);
        _tileHitboxSpinButtons.set_value(zeroRect);

        _useGridLocationCB.set_active(false);
        _useCustomOriginCB.set_active(false);
        _solidCB.set_active(false);

        widget.set_sensitive(false);
    }
}
