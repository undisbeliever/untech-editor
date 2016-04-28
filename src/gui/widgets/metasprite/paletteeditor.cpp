#include "paletteeditor.h"
#include "palettecolordialog.h"
#include "gui/undo/actionhelper.h"
#include <iomanip>

using namespace UnTech::Widgets::MetaSprite;

// ::TODO index editor::

PaletteEditor::PaletteEditor()
    : widget()
    , _palette(nullptr)
    , _editSelectOption(_("Edit"))
    , _colorButtons()
    , _paletteLabel(_("Palette:"), Gtk::ALIGN_START)
    , _updatingValues(false)
{
    widget.set_border_width(DEFAULT_BORDER);
    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    widget.attach(_paletteLabel, 0, 0, 6, 1);
    widget.attach(_editSelectOption, 6, 0, 2, 1);

    for (unsigned i = 0; i < N_COLORS; i++) {
        unsigned x = i % 8;
        unsigned y = i / 8 + 1;

        widget.attach(_colorButtons[i], x, y, 1, 1);
    }

    updateGuiValues();

    /**
     * SLOTS
     */

    /** Palette Updated signal */
    Signals::paletteChanged.connect([this](const MS::Palette* palette) {
        if (_palette == palette) {
            updateGuiValues();
        }
    });

    _editSelectOption.signal_toggled().connect([this](void) {
        unselectAllColors();
    });

    /** Button color selected signal */
    for (unsigned i = 0; i < N_COLORS; i++) {
        _colorButtons[i].signal_toggled().connect(sigc::bind<int>(
            sigc::mem_fun(*this, &PaletteEditor::on_color_toggled), i));
    }
}

void PaletteEditor::updateGuiValues()
{
    if (_palette) {
        _updatingValues = true;

        for (unsigned i = 0; i < N_COLORS; i++) {
            _colorButtons[i].set_color(_palette->color(i).rgb());
        }

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        for (auto& c : _colorButtons) {
            c.unset_color();
        }

        widget.set_sensitive(false);
    }
}

void PaletteEditor::on_color_toggled(int colorId)
{
    if (!_updatingValues) {
        _updatingValues = true;

        if (_palette && colorId >= 0 && colorId < (int)N_COLORS) {
            auto& selectedColor = _colorButtons[colorId];

            if (selectedColor.get_active() == true) {
                // unselect everything but current
                for (auto& c : _colorButtons) {
                    if (&c != &selectedColor) {
                        c.set_active(false);
                    }
                }

                // Bring up the menu if necessary
                if (_editSelectOption.get_active() == true) {
                    PaletteColorDialog dialog(*_palette, colorId, selectedColor);
                    dialog.run();

                    unselectAllColors();
                }
                else {
                    // ::TODO select color for editing tileset::
                }
            }
            else {
                unselectAllColors();
            }
        }
        else {
            unselectAllColors();
        }

        _updatingValues = false;
    }
}

void PaletteEditor::unselectAllColors()
{
    bool oldState = _updatingValues;
    _updatingValues = false;

    // unselect everything
    for (auto& c : _colorButtons) {
        c.set_active(false);
    }

    // ::TODO unselect color for editing tileset::

    _updatingValues = oldState;
}
