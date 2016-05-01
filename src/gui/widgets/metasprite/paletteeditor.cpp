#include "paletteeditor.h"
#include "palettecolordialog.h"
#include "gui/undo/actionhelper.h"
#include <iomanip>

using namespace UnTech::Widgets::MetaSprite;

PaletteEditor::PaletteEditor(Selection& selection)
    : widget()
    , _selection(selection)
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

    _selection.signal_paletteChanged.connect([this](void) {
        updateGuiValues();
        _editSelectOption.set_active(true);
        unselectAllColors();
    });

    _selection.signal_editTileColorChanged.connect(sigc::mem_fun(
        *this, &PaletteEditor::updateGuiValues));

    /** Palette Updated signal */
    Signals::paletteChanged.connect([this](const MS::Palette* palette) {
        if (_selection.palette() == palette) {
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
    const MS::Palette* palette = _selection.palette();

    if (palette) {
        _updatingValues = true;

        for (unsigned i = 0; i < N_COLORS; i++) {
            _colorButtons[i].set_color(palette->color(i).rgb());
        }

        widget.set_sensitive(true);

        _updatingValues = false;

        int active = _selection.editTileColor();
        if (active >= 0 && active < (int)N_COLORS) {
            _editSelectOption.set_active(false);

            if (_colorButtons[active].get_active() == false) {
                _colorButtons[active].set_active(true);
            }
        }
        else {
            unselectAllColors();
        }
    }
    else {
        for (auto& c : _colorButtons) {
            c.unset_color();
            c.set_active(false);
        }

        widget.set_sensitive(false);
    }
}

void PaletteEditor::on_color_toggled(int colorId)
{
    if (!_updatingValues) {
        _updatingValues = true;

        MS::Palette* palette = _selection.palette();

        if (palette && colorId >= 0 && colorId < (int)N_COLORS) {
            auto& selectedColor = _colorButtons[colorId];

            if (selectedColor.get_active() == true) {
                // unselect everything but current
                for (auto& c : _colorButtons) {
                    if (&c != &selectedColor) {
                        c.set_active(false);
                    }
                }

                if (_editSelectOption.get_active() == true) {
                    // Show dialog for editing colors
                    // Dialog is responsible for setting up undo action.
                    PaletteColorDialog dialog(*palette, colorId, selectedColor);
                    dialog.run();

                    unselectAllColors();
                }
                else {
                    // Set color to editing tiles
                    _selection.setEditTileColor(colorId);
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

    _selection.unsetEditTileColor();

    _updatingValues = oldState;
}
