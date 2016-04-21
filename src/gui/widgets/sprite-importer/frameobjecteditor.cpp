#include "frameobjecteditor.h"
#include "gui/undo/actionhelper.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

SIMPLE_UNDO_ACTION(frameObject_setLocation,
                   SI::FrameObject, UnTech::upoint, location, setLocation,
                   Signals::frameObjectChanged,
                   "Move Frame Object")

// Cannot use simple undo action for FrameObject::setSize
// Changing the size can change the location.
inline void frameObject_setSize(SI::FrameObject* item,
                                const SI::FrameObject::ObjectSize& newSize)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(SI::FrameObject* item,
               const SI::FrameObject::ObjectSize& oldSize,
               const UnTech::upoint& oldLocation,
               const SI::FrameObject::ObjectSize& newSize)
            : _item(item)
            , _oldSize(oldSize)
            , _oldLocation(oldLocation)
            , _newSize(newSize)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _item->setSize(_oldSize);
            _item->setLocation(_oldLocation);
            Signals::frameObjectChanged.emit(_item);
        }

        virtual void redo() override
        {
            _item->setSize(_newSize);
            Signals::frameObjectChanged.emit(_item);
        }

        virtual const Glib::ustring& message() const override
        {
            const static Glib::ustring message = _("Resize Frame Object");
            return message;
        }

    private:
        SI::FrameObject* _item;

        SI::FrameObject::ObjectSize _oldSize;
        UnTech::upoint _oldLocation;

        SI::FrameObject::ObjectSize _newSize;
    };

    if (item) {
        SI::FrameObject::ObjectSize oldSize = item->size();

        if (oldSize != newSize) {
            UnTech::upoint oldLocation = item->location();

            item->setSize(newSize);
            Signals::frameObjectChanged.emit(item);

            auto a = std::make_unique<Action>(item, oldSize, oldLocation, newSize);

            auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(item->document()));
            undoDoc->undoStack().add_undo(std::move(a));
        }
    }
}

FrameObjectEditor::FrameObjectEditor()
    : widget()
    , _frameObject()
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

    /** Set location signal */
    _locationSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frameObject && !_updatingValues) {
            frameObject_setLocation(_frameObject, _locationSpinButtons.value());
        }
    });

    /** Set Size Signal */
    _sizeCombo.signal_changed().connect([this](void) {
        typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

        if (_frameObject && !_updatingValues) {
            auto size = _sizeCombo.get_active_row_number() == 0 ? OS::SMALL : OS::LARGE;
            frameObject_setSize(_frameObject, size);
        }
    });

    /* Update gui if object has changed */
    Signals::frameObjectChanged.connect([this](const SI::FrameObject* obj) {
        if (_frameObject == obj) {
            updateGuiValues();
        }
    });

    /* Update location range if necessary */
    Signals::frameSizeChanged.connect([this](const SI::Frame* frame) {
        if (frame && _frameObject) {
            const SI::Frame& f = _frameObject->frame();

            if (frame == &f) {
                _locationSpinButtons.set_range(frame->locationSize());
            }
        }
    });

    /* Update location range if necessary */
    Signals::frameSetGridChanged.connect([this](const SI::FrameSet* frameSet) {
        if (frameSet && _frameObject) {
            const SI::Frame& f = _frameObject->frame();

            if (frameSet == &f.frameSet()) {
                _locationSpinButtons.set_range(f.locationSize());
            }
        }
    });
}

void FrameObjectEditor::updateGuiValues()
{
    typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

    if (_frameObject) {
        _updatingValues = true;

        _locationSpinButtons.set_range(_frameObject->frame().locationSize(), _frameObject->sizePx());
        _locationSpinButtons.set_value(_frameObject->location());
        _sizeCombo.set_active(_frameObject->size() == OS::LARGE ? 1 : 0);

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
