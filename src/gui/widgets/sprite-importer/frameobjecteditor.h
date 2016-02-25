#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEOBJECTEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMEOBJECTEDITOR_H_

#include "signals.h"
#include "models/sprite-importer/frameobject.h"
#include "models/sprite-importer/frame.h"
#include "gui/widgets/common/aabb.h"
#include "gui/widgets/defaults.h"

#include <cassert>
#include <memory>

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

class FrameObjectEditor {
public:
    FrameObjectEditor()
        : widget()
        , _frameObject()
        , _locationSpinButtons()
        , _sizeCombo()
        , _locationLabel(_("Location:"), Gtk::ALIGN_START)
        , _locationCommaLabel(" ,  ")
        , _sizeLabel(_("Size:"), Gtk::ALIGN_START)
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
            if (_frameObject) {
                _frameObject->setLocation(_locationSpinButtons.value());
                signal_frameObjectChanged.emit(_frameObject);
            }
        });

        /** Set Size Signal */
        _sizeCombo.signal_changed().connect([this](void) {
            typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

            if (_frameObject) {
                _frameObject->setSize(_sizeCombo.get_active_row_number() == 0 ? OS::SMALL : OS::LARGE);
                signal_frameObjectChanged.emit(_frameObject);
            }
        });

        /* Update gui if object has changed */
        signal_frameObjectChanged.connect([this](const std::shared_ptr<SI::FrameObject> obj) {
            if (_frameObject == obj) {
                updateGuiValues();
            }
        });

        /* Update location range if necessary */
        signal_frameSizeChanged.connect([this](const std::shared_ptr<SI::Frame> frame) {
            if (_frameObject) {
                const auto f = _frameObject->frame().lock();
                if (frame == f) {
                    _locationSpinButtons.set_range(frame->locationSize());
                }
            }
        });

        /* Update location range if necessary */
        signal_frameSetGridChanged.connect([this](const std::shared_ptr<SI::FrameSet> fs) {
            if (_frameObject) {
                const auto frame = _frameObject->frame().lock();
                if (frame->frameSet().lock() == fs) {
                    _locationSpinButtons.set_range(frame->locationSize());
                }
            }
        });
    }

    void setFrameObject(std::shared_ptr<UnTech::SpriteImporter::FrameObject> frameObject)
    {
        _frameObject = frameObject;
        updateGuiValues();
    }

protected:
    void updateGuiValues()
    {
        typedef UnTech::SpriteImporter::FrameObject::ObjectSize OS;

        if (_frameObject) {
            auto frame = _frameObject->frame().lock();

            if (frame) {
                _locationSpinButtons.set_range(frame->locationSize(), _frameObject->sizePx());
            }
            _locationSpinButtons.set_value(_frameObject->location());
            _sizeCombo.set_active(_frameObject->size() == OS::LARGE ? 1 : 0);

            widget.set_sensitive(true);
        }
        else {
            static const upoint zeroPoint = { 0, 0 };

            _locationSpinButtons.set_value(zeroPoint);
            _sizeCombo.set_active_text("");

            widget.set_sensitive(false);
        }
    }

public:
    Gtk::Grid widget;

private:
    std::shared_ptr<UnTech::SpriteImporter::FrameObject> _frameObject;

    UpointSpinButtons _locationSpinButtons;
    Gtk::ComboBoxText _sizeCombo;

    Gtk::Label _locationLabel, _locationCommaLabel, _sizeLabel;
};
}
}
}

#endif
