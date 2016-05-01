#include "frameobjecteditor.h"
#include "document.h"
#include "signals.h"
#include "models/common/string.h"
#include "gui/undo/actionhelper.h"
#include "gui/undo/mergeactionhelper.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::SpriteImporter;

SIMPLE_UNDO_MERGE_ACTION(frameObject_merge_setLocation,
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

FrameObjectEditor::FrameObjectEditor(Selection& selection)
    : widget()
    , _selection(selection)
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

    /* Update gui when selected frameObject changed */
    _selection.signal_frameObjectChanged.connect(sigc::mem_fun(
        *this, &FrameObjectEditor::updateGuiValues));

    /* Update gui if frameObject has changed */
    Signals::frameObjectChanged.connect([this](const SI::FrameObject* obj) {
        if (obj == _selection.frameObject()) {
            updateGuiValues();
        }
    });

    /* Update location range if necessary */
    Signals::frameSizeChanged.connect([this](const SI::Frame* frame) {
        if (_selection.frameObject() && frame == _selection.frame()) {
            _locationSpinButtons.set_range(_selection.frame()->locationSize());
        }
    });

    /* Update location range if necessary */
    Signals::frameSetGridChanged.connect([this](const SI::FrameSet* frameset) {
        if (_selection.frameObject() && frameset == _selection.frameSet()) {
            _locationSpinButtons.set_range(_selection.frame()->locationSize());
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

    /** Set Size Signal */
    _sizeCombo.signal_changed().connect([this](void) {
        typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

        if (!_updatingValues) {
            auto size = _sizeCombo.get_active_row_number() == 0 ? OS::SMALL : OS::LARGE;
            frameObject_setSize(_selection.frameObject(), size);
        }
    });
}

void FrameObjectEditor::updateGuiValues()
{
    typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

    const SI::FrameObject* frameObject = _selection.frameObject();

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
