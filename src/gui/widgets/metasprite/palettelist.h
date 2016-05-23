#pragma once

#include "signals.h"
#include "gui/widgets/common/orderedlist.h"
#include "models/metasprite/palette.h"

#include <cassert>
#include <cstring>
#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

namespace Private {

class PaletteModelColumns : public Gtk::TreeModel::ColumnRecord {
    const unsigned N_COLORS = MS::Palette::N_COLORS;
    const unsigned PIXBUF_HEIGHT = 16;
    const unsigned PIXBUF_SPACING = 0;
    const unsigned PIXBUF_WIDTH = PIXBUF_HEIGHT * N_COLORS + PIXBUF_SPACING * (N_COLORS - 1);

public:
    PaletteModelColumns()
    {
        add(col_item);
        add(col_id);
        add(col_pixbuf);
    }

    Gtk::TreeModelColumn<MS::Palette*> col_item;
    Gtk::TreeModelColumn<unsigned int> col_id;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> col_pixbuf;

    inline void buildTreeViewColumns(Gtk::TreeView& treeView)
    {
        treeView.append_column(_("id"), col_id);
        treeView.append_column(_("Palette"), col_pixbuf);
    }

    inline void setRowData(Gtk::TreeRow& row, const MS::Palette* pal)
    {
        Glib::RefPtr<Gdk::Pixbuf> pixbuf = row.get_value(col_pixbuf);

        if (!pixbuf) {
            pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8,
                                         PIXBUF_WIDTH, PIXBUF_HEIGHT);
            pixbuf->fill(0);
            row.set_value(col_pixbuf, pixbuf);
        }

        assert(pixbuf->get_colorspace() == Gdk::COLORSPACE_RGB);
        assert(pixbuf->get_width() == (int)PIXBUF_WIDTH);
        assert(pixbuf->get_height() == (int)PIXBUF_HEIGHT);
        assert(pixbuf->get_bits_per_sample() == 8);
        assert(pixbuf->get_has_alpha() == true);
        assert(pixbuf->get_n_channels() == 4);

        // Draw the first scanline of the data.
        {
            rgba* scanline = reinterpret_cast<rgba*>(pixbuf->get_pixels());

            for (const Snes::SnesColor& c : pal->colors()) {
                for (unsigned i = 0; i < PIXBUF_HEIGHT; i++) {
                    *scanline++ = c.rgb().value;
                }
                scanline += PIXBUF_SPACING;
                ;
            }
        }

        // Copy scanlines for the rest of image
        {
            guint8* data = pixbuf->get_pixels();
            const int rowstride = pixbuf->get_rowstride();
            const unsigned rowsize = 4 * PIXBUF_WIDTH;

            for (unsigned i = 1; i < PIXBUF_HEIGHT; i++) {
                guint8* scanline = data + (rowstride * i);
                memcpy(scanline, data, rowsize);
            }
        }
    }

    inline static auto& signal_itemChanged() { return Signals::paletteChanged; }

    inline static auto& signal_listChanged() { return Signals::paletteListChanged; }

    inline static const char* itemTypeName() { return "Palette"; }
};
}

typedef OrderedListEditor<MS::Palette, Private::PaletteModelColumns> PaletteListEditor;
}
}
}
