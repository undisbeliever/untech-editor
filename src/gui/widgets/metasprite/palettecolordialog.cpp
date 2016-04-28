#include "palettecolordialog.h"
#include "signals.h"
#include "models/metasprite/palette.h"
#include "gui/widgets/defaults.h"
#include "gui/undo/undostack.h"
#include "gui/undo/undodocument.h"

#include <glibmm/i18n.h>

using namespace UnTech;
using namespace UnTech::Widgets::MetaSprite;

inline void palette_setColor(MS::Palette& item, unsigned colorId,
                             const Snes::SnesColor& oldColor,
                             const Snes::SnesColor& newColor)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(MS::Palette& item,
               const unsigned colorId,
               const Snes::SnesColor& oldColor,
               const Snes::SnesColor& newColor)
            : _item(item)
            , _colorId(colorId)
            , _oldColor(oldColor)
            , _newColor(newColor)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _item.color(_colorId) = _oldColor;
            Signals::paletteChanged.emit(&_item);
        }

        virtual void redo() override
        {
            _item.color(_colorId) = _newColor;
            Signals::paletteChanged.emit(&_item);
        }

        virtual const Glib::ustring& message() const override
        {
            const static Glib::ustring message = _("Change color");
            return message;
        }

    private:
        MS::Palette& _item;
        const unsigned _colorId;
        const Snes::SnesColor _oldColor;
        const Snes::SnesColor _newColor;
    };

    if (oldColor != newColor) {
        auto a = std::make_unique<Action>(item, colorId, oldColor, newColor);

        auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(item.document()));
        undoDoc->undoStack().add_undo(std::move(a));
    }
}

PaletteColorDialog::PaletteColorDialog(MS::Palette& palette, unsigned colorId, Gtk::Widget& parent)
    : Gtk::Dialog(_("SNES Color"), false)
    , _grid()
    , _blueScale(Gtk::Adjustment::create(0.0, 0.0, MAX_COLOR_VALUE, 1.0, 4.0, 0.0),
                 Gtk::ORIENTATION_HORIZONTAL)
    , _greenScale(Gtk::Adjustment::create(0.0, 0.0, MAX_COLOR_VALUE, 1.0, 4.0, 0.0),
                  Gtk::ORIENTATION_HORIZONTAL)
    , _redScale(Gtk::Adjustment::create(0.0, 0.0, MAX_COLOR_VALUE, 1.0, 4.0, 0.0),
                Gtk::ORIENTATION_HORIZONTAL)
    , _colorArea(palette.color(colorId).rgb())
    , _blueLabel(_("Blue:"), Gtk::ALIGN_START)
    , _greenLabel(_("Green:"), Gtk::ALIGN_START)
    , _redLabel(_("Red:"), Gtk::ALIGN_START)
    , _palette(palette)
    , _colorId(colorId)
    , _oldColor(palette.color(colorId))
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

    // Signals
    Signals::paletteChanged.connect([this](const MS::Palette* pal) {
        if (pal == &_palette) {
            updateGuiValues();
            _colorArea.queue_draw();
        }
    });

    _blueScale.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            _palette.color(_colorId).setBlue(_blueScale.get_value());
            Signals::paletteChanged.emit(&_palette);
        }
    });
    _greenScale.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            _palette.color(_colorId).setGreen(_greenScale.get_value());
            Signals::paletteChanged.emit(&_palette);
        }
    });
    _redScale.signal_value_changed().connect([this](void) {
        if (!_updatingValues) {
            _palette.color(_colorId).setRed(_redScale.get_value());
            Signals::paletteChanged.emit(&_palette);
        }
    });

    /** Reset color if necessary. */
    signal_response().connect([this](int response) {
        if (response == Gtk::RESPONSE_ACCEPT) {
            palette_setColor(_palette, _colorId,
                             _oldColor,
                             _palette.color(_colorId));
        }
        else {
            // restore color.
            _palette.color(_colorId) = _oldColor;
            Signals::paletteChanged.emit(&_palette);
        }
    });
}

void PaletteColorDialog::updateGuiValues()
{
    _updatingValues = true;

    auto& c = _palette.color(_colorId);

    _blueScale.set_value(c.blue());
    _greenScale.set_value(c.green());
    _redScale.set_value(c.red());

    _colorArea.set_color(c.rgb());

    _updatingValues = false;
}
