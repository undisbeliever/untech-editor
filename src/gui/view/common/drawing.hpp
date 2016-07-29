#pragma once
#include <functional>
#include <wx/dc.h>
#include <wx/pen.h>

namespace UnTech {
namespace View {
namespace DC {

const static wxColour SELECTED_OUTER(0, 0, 0);
const static wxColour SELECTED_INNER(255, 255, 255);

inline void DashedPen(wxDC& dc, const wxColour& bgColor, const wxColour& fgColor,
                      std::function<void(wxDC&)> drawLines)
{
    // ::ANNOY wxGTK does not support Stipple Pens::
    // Have to draw this twice.

    // ::SHOULDO use stipple pen in wxMSW, wxOSX/Cocoa::

    dc.SetBrush(wxNullBrush);

    dc.SetPen(wxPen(bgColor, 1, wxPENSTYLE_SOLID));
    drawLines(dc);

    dc.SetPen(wxPen(fgColor, 1, wxPENSTYLE_SHORT_DASH));
    drawLines(dc);
}

inline void DashedPen(wxDC& dc, std::function<void(wxDC&)> drawLines)
{
    DashedPen(dc, *wxBLACK, *wxWHITE, drawLines);
}

inline void DrawSelectedRectangle(wxDC& dc,
                                  int x, int y, int width, int height)
{
    dc.SetBrush(wxNullBrush);

    dc.SetPen(wxPen(SELECTED_INNER, 1, wxPENSTYLE_SOLID));
    dc.DrawRectangle(x + 1, y + 1, width - 1, height - 1);

    dc.SetPen(wxPen(SELECTED_OUTER, 1, wxPENSTYLE_SOLID));
    dc.DrawRectangle(x, y, width + 1, height + 1);
}

inline void DrawSelectedRectangleZoom(wxDC& dc,
                                      double zoomX, double zoomY,
                                      int x, int y, int width, int height)
{
    const int xStart = x * zoomX;
    const int xEnd = (x + width) * zoomX;

    const int yStart = y * zoomY;
    const int yEnd = (y + height) * zoomY;

    DrawSelectedRectangle(dc, xStart, yStart,
                          xEnd - xStart, yEnd - yStart);
}

inline void DrawSelectedRectangleOffset(wxDC& dc,
                                        double zoomX, double zoomY,
                                        int xOffset, int yOffset,
                                        int x, int y, int width, int height)
{
    DrawSelectedRectangleZoom(dc, zoomX, zoomY,
                              x + xOffset, y + yOffset,
                              width, height);
}
}
}
}
