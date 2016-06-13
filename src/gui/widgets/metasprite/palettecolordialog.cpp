#include "palettecolordialog.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>

using namespace UnTech;
using namespace UnTech::Widgets::MetaSprite;

PaletteColorDialog::PaletteColorDialog(MS::PaletteController& controller, unsigned colorId, Gtk::Widget& parent)
    : Gtk::Dialog(_("SNES Color"), false)
    , _controller(controller)
    , _grid()
    , _blueScale(Gtk::Adjustment::create(0.0, 0.0, MAX_COLOR_VALUE, 1.0, 4.0, 0.0),
                 Gtk::ORIENTATION_HORIZONTAL)
    , _greenScale(Gtk::Adjustment::create(0.0, 0.0, MAX_COLOR_VALUE, 1.0, 4.0, 0.0),
                  Gtk::ORIENTATION_HORIZONTAL)
    , _redScale(Gtk::Adjustment::create(0.0, 0.0, MAX_COLOR_VALUE, 1.0, 4.0, 0.0),
                Gtk::ORIENTATION_HORIZONTAL)
    , _colorArea()
    , _blueLabel(_("Blue:"), Gtk::ALIGN_START)
    , _greenLabel(_("Green:"), Gtk::ALIGN_START)
    , _redLabel(_("Red:"), Gtk::ALIGN_START)
    , _colorId(colorId)
    , _updatingValues(false)
{
    set_default_response(Gtk::RESPONSE_ACCEPT);

    add_button(_("_OK"), Gtk::RESPONSE_ACCEPT);
    add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);

    // Expand the widgets to fit
    _blueScale.set_hexpand(true);
    _blueScale.set_size_request(160, -1);

    _colorArea.set_hexpand(true);
    _colorArea.set_size_request(50, -1);

    _blueScale.set_digits(0);
    _greenScale.set_digits(0);
    _redScale.set_digits(0);

    _blueScale.set_value_pos(Gtk::POS_RIGHT);
    _greenScale.set_value_pos(Gtk::POS_RIGHT);
    _redScale.set_value_pos(Gtk::POS_RIGHT);

    _grid.set_column_spacing(DEFAULT_ROW_SPACING);

    _grid.attach(_blueLabel, 0, 0, 1, 1);
    _grid.attach(_blueScale, 1, 0, 4, 1);
    _grid.attach(_greenLabel, 0, 1, 1, 1);
    _grid.attach(_greenScale, 1, 1, 4, 1);
    _grid.attach(_redLabel, 0, 2, 1, 1);
    _grid.attach(_redScale, 1, 2, 4, 1);

    _grid.attach(_colorArea, 5, 0, 1, 3);

    get_content_area()->add(_grid);

    updateGuiValues();

    show_all_children();

    // Set transient parent from widget
    // Could not derefernce RefPtr<Gtk::Window> for base-class constructor
    // using C method instead
    GtkWidget* toplevel = gtk_widget_get_toplevel(parent.gobj());
    gtk_window_set_transient_for(GTK_WINDOW(this->gobj()), GTK_WINDOW(toplevel));

    /*
     * SLOTS
     * =====
     */

    /* Controller Signals */
    _con1 = _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &PaletteColorDialog::updateGuiValues));

    _con2 = _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &PaletteColorDialog::updateGuiValues));

    /* Slider Signals */
    _blueScale.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            _color.setBlue(_blueScale.get_value());
            _controller.selected_setColor_merge(_colorId, _color);
        }
    });
    _greenScale.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            _color.setGreen(_greenScale.get_value());
            _controller.selected_setColor_merge(_colorId, _color);
        }
    });
    _redScale.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            _color.setRed(_redScale.get_value());
            _controller.selected_setColor_merge(_colorId, _color);
        }
    });

    /** Reset color on cancel. */
    signal_response().connect([this](int response) {
        if (response == Gtk::RESPONSE_ACCEPT) {
            _controller.baseController().dontMergeNextAction();
        }
        else {
            // restore color.
            _controller.baseController().undoStack().undo();
        }
        close();
    });
}

PaletteColorDialog::~PaletteColorDialog()
{
    _con1.disconnect();
    _con2.disconnect();
}

void PaletteColorDialog::updateGuiValues()
{
    const MS::Palette* palette = _controller.selected();

    if (palette != nullptr) {
        _updatingValues = true;

        _color = _controller.selected()->color(_colorId);

        _blueScale.set_value(_color.blue());
        _greenScale.set_value(_color.green());
        _redScale.set_value(_color.red());

        _colorArea.set_color(_color.rgb());

        _updatingValues = false;
    }
}
