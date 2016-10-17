#include "palettelist.h"
#include "gui/view/defaults.h"
#include <wx/bitmap.h>
#include <wx/imaglist.h>
#include <wx/rawbmp.h>

using namespace UnTech;
using namespace UnTech::View::MetaSprite::MetaSprite;

PaletteListCtrl::PaletteListCtrl(wxWindow* parent, wxWindowID id,
                                 MS::PaletteController& controller)
    : wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
                 wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
    , _controller(controller)
{
    AppendColumn("Id", wxLIST_FORMAT_LEFT, SIDEBAR_WIDTH - IMAGE_WIDTH - 50);
    AppendColumn("Palette", wxLIST_FORMAT_LEFT, IMAGE_WIDTH);

    /*
     * EVENTS
     * ======
     */
    this->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxCommandEvent&) {
        long i = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (i != wxNOT_FOUND) {
            _controller.selectIndex(i);
        }
        else {
            _controller.selectNone();
        }
    });

    /*
     * SLOTS
     * =====
     */
    _controller.signal_listChanged().connect(
        [this](void) {
            const MS::PaletteController::list_type* list = _controller.list();

            if (list) {
                RedrawImageList();
                SetItemCount(list->size());
                Refresh();
                Enable();
            }
            else {
                SetItemCount(0);
                Disable();
            }
        });

    _controller.signal_selectedChanged().connect(
        [this](void) {
            const MS::PaletteController::list_type* list = _controller.list();
            optional<size_t> index = _controller.selectedIndex();

            if (list && index) {
                SetItemCount(list->size()); // BUGFIX
                SetItemState(index.value(), wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                EnsureVisible(index.value());
            }
            else {
                // deselect the item(s)
                long i = -1;
                while ((i = GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED))
                       != wxNOT_FOUND) {
                    SetItemState(i, 0, wxLIST_STATE_SELECTED);
                }
            }
        });

    _controller.signal_listChanged().connect(
        [this](void) {
            const MS::PaletteController::list_type* list = _controller.list();
            if (list) {
                RedrawImageList();
                SetItemCount(list->size());

                optional<size_t> index = _controller.selectedIndex();
                if (list && index) {
                    SetItemCount(list->size()); // BUGFIX
                    SetItemState(index.value(), wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                    EnsureVisible(index.value());
                }
            }
        });

    _controller.signal_dataChanged().connect(
        [this](void) {
            Refresh();
        });
}

void PaletteListCtrl::RedrawImageList()
{
    const MS::PaletteController::list_type* list = _controller.list();

    if (list && list->size() > 0) {
        auto* imageList = new wxImageList(IMAGE_WIDTH, IMAGE_HEIGHT,
                                          false, list->size());

        wxImage image(IMAGE_WIDTH, IMAGE_HEIGHT, true);

        for (auto& palette : *list) {
            {
                uint8_t* data = reinterpret_cast<uint8_t*>(image.GetData());

                for (const Snes::SnesColor& color : palette.colors()) {
                    for (unsigned i = 0; i < COLOR_SIZE; i++) {
                        *data++ = color.rgb().red;
                        *data++ = color.rgb().green;
                        *data++ = color.rgb().blue;
                    }
                    data += COLOR_SPACING * 3;
                }
            }
            {
                char* data = reinterpret_cast<char*>(image.GetData());
                const char* firstRow = data;
                unsigned rowsize = IMAGE_WIDTH * 3;

                for (unsigned i = 1; i < COLOR_SIZE; i++) {
                    memcpy(data + rowsize * i, firstRow, rowsize);
                }
            }

            imageList->Add(wxBitmap(image));
        }

        AssignImageList(imageList, wxIMAGE_LIST_SMALL);
    }
    else {
        AssignImageList(nullptr, wxIMAGE_LIST_SMALL);
    }
}

wxString PaletteListCtrl::OnGetItemText(long item, long column) const
{
    if (column == 0) {
        wxString ret;
        ret << item;
        return ret;
    }
    else {
        return wxEmptyString;
    }
}

int PaletteListCtrl::OnGetItemColumnImage(long item, long column) const
{
    if (column == 1)
        return static_cast<int>(item);
    else {
        return -1;
    }
}
