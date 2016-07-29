#pragma once
#include "models/common/ms8aabb.h"
#include <functional>
#include <wx/dc.h>
#include <wx/pen.h>

namespace UnTech {
namespace View {

const static wxColour SELECTED_COLOR_OUTER(0, 0, 0);
const static wxColour SELECTED_COLOR_INNER(255, 255, 255);

const static wxPen ORIGIN_PEN(wxColor(112, 112, 112, 112), 1, wxPENSTYLE_SHORT_DASH);

const static wxPen FRAME_OBJECTS_PEN(wxColor(64, 112, 64, 192), 1);
const static wxPen ACTION_POINT_PEN(wxColor(192, 192, 192, 240), 2);

const static wxPen ENTITY_HITBOX_PEN(wxColor(32, 0, 192, 240), 1);
const static wxBrush ENTITY_HITBOX_BRUSH(wxColor(32, 0, 192, 32));

const static wxPen TILE_HITBOX_PEN(wxColor(192, 0, 0, 240), 1);
const static wxBrush TILE_HITBOX_BRUSH(wxColor(192, 0, 0, 32));

class DrawingHelper {
    const static int DEFAULT_CROSS_SIZE = 4;
    const static int SELECTED_CROSS_SIZE = 6;

public:
    wxDC& dc;
    const double zoomX, zoomY;
    const int xOffset, yOffset;

public:
    DrawingHelper(wxDC& dc,
                  double zoomX, double zoomY,
                  int xPos, int yPos)
        : dc(dc)
        , zoomX(zoomX)
        , zoomY(zoomY)
        , xOffset(-xPos)
        , yOffset(-yPos)
    {
    }

    inline void DrawRectangle(int x, int y, unsigned width, unsigned height)
    {
        // This prevents rounding errors
        const int xStart = (x + xOffset) * zoomX;
        const int xEnd = (x + xOffset + width) * zoomX;

        const int yStart = (y + yOffset) * zoomY;
        const int yEnd = (y + yOffset + height) * zoomY;

        dc.DrawRectangle(xStart, yStart, xEnd - xStart, yEnd - yStart);
    }

    inline void DrawRectangle(const ms8rect& r)
    {
        DrawRectangle(r.x, r.y, r.width, r.height);
    }

    inline void DrawSquare(const ms8point& p, unsigned size)
    {
        DrawRectangle(p.x, p.y, size, size);
    }

    inline void DrawCross(int x, int y, int size = DEFAULT_CROSS_SIZE)
    {
        const int rx = (x + xOffset) * zoomX;
        const int ry = (y + yOffset) * zoomY;

        dc.DrawLine(rx, ry - size, rx, ry + size);
        dc.DrawLine(rx - size, ry, rx + size, ry);
    }

    inline void DrawCross(const ms8point& p, int size = DEFAULT_CROSS_SIZE)
    {
        DrawCross(p.x, p.y, size);
    }

    // Selected
    // --------
    inline void DrawSelectedRectangle(int x, int y, unsigned width, unsigned height)
    {
        // This prevents rounding errors
        const int xStart = (x + xOffset) * zoomX;
        const int xEnd = (x + xOffset + width) * zoomX;

        const int yStart = (y + yOffset) * zoomY;
        const int yEnd = (y + yOffset + height) * zoomY;

        const int w = xEnd - xStart;
        const int h = yEnd - yStart;

        dc.SetBrush(wxNullBrush);

        dc.SetPen(wxPen(SELECTED_COLOR_INNER, 1, wxPENSTYLE_SOLID));
        dc.DrawRectangle(xStart + 1, yStart + 1, w - 1, h - 1);

        dc.SetPen(wxPen(SELECTED_COLOR_OUTER, 1, wxPENSTYLE_SOLID));
        dc.DrawRectangle(xStart, yStart, w + 1, h + 1);
    }

    inline void DrawSelectedRectangle(const ms8rect& r)
    {
        DrawSelectedRectangle(r.x, r.y, r.width, r.height);
    }

    inline void DrawSelectedSquare(const ms8point& p, unsigned size)
    {
        DrawSelectedRectangle(p.x, p.y, size, size);
    }

    inline void DrawSelectedCross(int x, int y)
    {
        const static int SIZE = SELECTED_CROSS_SIZE;
        const int rx = (x + xOffset) * zoomX;
        const int ry = (y + yOffset) * zoomY;

        dc.SetPen(wxPen(SELECTED_COLOR_OUTER, 3, wxPENSTYLE_SOLID));
        dc.DrawLine(rx, ry - SIZE, rx, ry + SIZE);
        dc.DrawLine(rx - SIZE, ry, rx + SIZE, ry);

        dc.SetPen(wxPen(SELECTED_COLOR_INNER, 1, wxPENSTYLE_SOLID));
        dc.DrawLine(rx, ry - SIZE, rx, ry + SIZE);
        dc.DrawLine(rx - SIZE, ry, rx + SIZE, ry);
    }

    inline void DrawSelectedCross(const ms8point& p)
    {
        DrawSelectedCross(p.x, p.y);
    }
};

// Dashed Pen
// ----------
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
}
}
