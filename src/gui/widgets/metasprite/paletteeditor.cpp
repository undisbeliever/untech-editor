#include "paletteeditor.h"
#include "palettecolordialog.h"
#include <iomanip>

using namespace UnTech::Widgets::MetaSprite;

PaletteEditor::PaletteEditor(MS::PaletteController& controller)
    : widget()
    , _controller(controller)
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

    /* Controller signals */
    _controller.signal_selectedChanged().connect([this](void) {
        updateGuiValues();
        _editSelectOption.set_active(_controller.selected() != nullptr);
        unselectAllColors();
    });

    _controller.signal_selectedColorChanged().connect(sigc::mem_fun(
        *this, &PaletteEditor::updateGuiValues));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &PaletteEditor::updateGuiValues));

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
    const MS::Palette* palette = _controller.selected();

    if (palette) {
        _updatingValues = true;

        for (unsigned i = 0; i < N_COLORS; i++) {
            _colorButtons[i].set_color(palette->color(i).rgb());
        }

        widget.set_sensitive(true);

        _updatingValues = false;

        int active = _controller.selectedColorId();

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

        const MS::Palette* palette = _controller.selected();

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
                    PaletteColorDialog dialog(_controller, colorId, widget);
                    dialog.run();

                    unselectAllColors();
                }
                else {
                    // Set color to editing tiles
                    _controller.setSelectedColorId(colorId);
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

    _controller.unsetSelectedColor();

    _updatingValues = oldState;
}
