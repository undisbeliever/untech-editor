#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ACTIONPOINTEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ACTIONPOINTEDITOR_H_

#include "signals.h"
#include "models/sprite-importer/actionpoint.h"
#include "models/sprite-importer/frame.h"
#include "models/common/string.h"
#include "gui/widgets/common/aabb.h"
#include "gui/widgets/defaults.h"

#include <cassert>
#include <memory>

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

class ActionPointEditor {
public:
    ActionPointEditor()
        : widget()
        , _actionPoint()
        , _locationSpinButtons()
        , _parameterEntry()
        , _locationLabel(_("Location:"), Gtk::ALIGN_START)
        , _locationCommaLabel(" ,  ")
        , _parameterLabel(_("Parameter:"), Gtk::ALIGN_START)
    {
        widget.set_row_spacing(DEFAULT_ROW_SPACING);

        widget.attach(_locationLabel, 0, 0, 1, 1);
        widget.attach(_locationSpinButtons.xSpin, 1, 0, 1, 1);
        widget.attach(_locationCommaLabel, 2, 0, 1, 1);
        widget.attach(_locationSpinButtons.ySpin, 3, 0, 1, 1);

        widget.attach(_parameterLabel, 0, 1, 1, 1);
        widget.attach(_parameterEntry, 1, 1, 3, 1);

        widget.set_sensitive(false);

        /*
         * SLOTS
         * =====
         */

        /** Set location signal */
        _locationSpinButtons.signal_valueChanged.connect([this](void) {
            if (_actionPoint) {
                _actionPoint->setLocation(_locationSpinButtons.value());
                signal_actionPointChanged.emit(_actionPoint);
                signal_actionPointLocationChanged.emit(_actionPoint);
            }
        });

        /* Set Parameter has finished editing */
        // signal_editing_done does not work
        // using activate and focus out instead.
        _parameterEntry.signal_activate().connect(sigc::mem_fun(
            *this, &ActionPointEditor::onParameterFinishedEditing));
        _parameterEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
            this->onParameterFinishedEditing();
            return false;
        });

        /* Update gui if object has changed */
        signal_actionPointChanged.connect([this](const std::shared_ptr<SI::ActionPoint> obj) {
            if (_actionPoint == obj) {
                updateGuiValues();
            }
        });

        /* Update location range if necessary */
        signal_frameLocationChanged.connect([this](const std::shared_ptr<SI::Frame> frame) {
            if (_actionPoint) {
                const auto f = _actionPoint->frame().lock();
                if (frame == f) {
                    _locationSpinButtons.set_range(frame->locationSize());
                }
            }
        });

        signal_frameSetGridChanged.connect([this](const std::shared_ptr<SI::FrameSet> fs) {
            if (_actionPoint) {
                const auto frame = _actionPoint->frame().lock();
                if (frame->frameSet().lock() == fs) {
                    _locationSpinButtons.set_range(frame->locationSize());
                }
            }
        });
    }

    void setActionPoint(std::shared_ptr<UnTech::SpriteImporter::ActionPoint> actionPoint)
    {
        _actionPoint = actionPoint;
        updateGuiValues();
    }

protected:
    void updateGuiValues()
    {
        if (_actionPoint) {
            auto frame = _actionPoint->frame().lock();

            if (frame) {
                _locationSpinButtons.set_range(frame->locationSize());
            }
            _locationSpinButtons.set_value(_actionPoint->location());
            _parameterEntry.set_text(Glib::ustring::compose("%1", _actionPoint->parameter()));

            widget.set_sensitive(true);
        }
        else {
            widget.set_sensitive(false);
        }
    }

    void onParameterFinishedEditing()
    {
        if (_actionPoint) {
            auto value = UnTech::String::toUint8(_parameterEntry.get_text());
            if (value.second) {
                _actionPoint->setParameter(value.first);
            }
            signal_actionPointChanged.emit(_actionPoint);
        }
    }

public:
    Gtk::Grid widget;

private:
    std::shared_ptr<UnTech::SpriteImporter::ActionPoint> _actionPoint;

    UpointSpinButtons _locationSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _locationLabel, _locationCommaLabel, _parameterLabel;
};
}
}
}

#endif
