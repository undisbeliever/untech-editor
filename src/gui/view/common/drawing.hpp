#pragma once
#include <functional>
#include <wx/dc.h>
#include <wx/pen.h>

namespace UnTech {
namespace View {
namespace DC {

const static wxPen ORIGIN_PEN(wxColor(112, 112, 112, 112), 1, wxPENSTYLE_SHORT_DASH);

const static wxPen FRAME_OBJECTS_PEN(wxColor(64, 112, 64, 192), 1);
const static wxPen ACTION_POINT_PEN(wxColor(192, 192, 192, 240), 2);

const static wxPen ENTITY_HITBOX_PEN(wxColor(32, 0, 192, 240), 1);
const static wxBrush ENTITY_HITBOX_BRUSH(wxColor(32, 0, 192, 32));

const static wxPen TILE_HITBOX_PEN(wxColor(192, 0, 0, 240), 1);
const static wxBrush TILE_HITBOX_BRUSH(wxColor(192, 0, 0, 32));

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

inline void DrawRectangleZoom(wxDC& dc,
                              double zoomX, double zoomY,
                              int x, int y, int width, int height)
{
    // This prevents rounding errors
    const int xStart = x * zoomX;
    const int xEnd = (x + width) * zoomX;

    const int yStart = y * zoomY;
    const int yEnd = (y + height) * zoomY;

    dc.DrawRectangle(xStart, yStart, xEnd - xStart, yEnd - yStart);
}

inline void DrawRectangleOffset(wxDC& dc,
                                double zoomX, double zoomY,
                                int xOffset, int yOffset,
                                int x, int y, int width, int height)
{
    DrawRectangleZoom(dc, zoomX, zoomY,
                      x + xOffset, y + yOffset,
                      width, height);
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
    // This prevents rounding errors
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

inline void DrawCross(wxDC& dc,
                      int x, int y)
{
    const static int SIZE = 4;

    dc.DrawLine(x, y - SIZE, x, y + SIZE);
    dc.DrawLine(x - SIZE, y, x + SIZE, y);
}

inline void DrawCrossOffset(wxDC& dc,
                            double zoomX, double zoomY,
                            int xOffset, int yOffset,
                            int x, int y)
{
    DrawCross(dc, (x + xOffset) * zoomX, (y + yOffset) * zoomY);
}

inline void DrawSelectedCross(wxDC& dc,
                              int x, int y)
{
    const static int SIZE = 6;

    dc.SetPen(wxPen(SELECTED_OUTER, 3, wxPENSTYLE_SOLID));
    dc.DrawLine(x, y - SIZE, x, y + SIZE);
    dc.DrawLine(x - SIZE, y, x + SIZE, y);

    dc.SetPen(wxPen(SELECTED_INNER, 1, wxPENSTYLE_SOLID));
    dc.DrawLine(x, y - SIZE, x, y + SIZE);
    dc.DrawLine(x - SIZE, y, x + SIZE, y);
}

inline void DrawSelectedCrossOffset(wxDC& dc,
                                    double zoomX, double zoomY,
                                    int xOffset, int yOffset,
                                    int x, int y)
{
    DrawSelectedCross(dc, (x + xOffset) * zoomX, (y + yOffset) * zoomY);
}
}
}
}
