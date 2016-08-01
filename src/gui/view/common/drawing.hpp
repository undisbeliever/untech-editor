#pragma once
#include "models/common/aabb.h"
#include "models/common/ms8aabb.h"
#include <functional>
#include <wx/dc.h>
#include <wx/pen.h>

namespace UnTech {
namespace View {

const static wxColour SELECTED_COLOR_OUTER(0, 0, 0);
const static wxColour SELECTED_COLOR_INNER(255, 255, 255);

const static wxBrush ANTI_HIGHLIGHT_BRUSH(wxColor(192, 192, 192, 192));

const static wxPen ORIGIN_PEN(wxColor(112, 112, 112, 112), 1, wxPENSTYLE_SHORT_DASH);
const static wxPen ORIGIN_OPAQUE_PEN(wxColor(112, 112, 112), 1, wxPENSTYLE_SHORT_DASH);

const static wxPen FRAME_OUTLINE_PEN(wxColor(128, 128, 128, 240), 2);

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

    inline void DrawRectangle(const urect& r)
    {
        DrawRectangle(r.x, r.y, r.width, r.height);
    }

    inline void DrawSquare(const ms8point& p, unsigned size)
    {
        DrawRectangle(p.x, p.y, size, size);
    }

    inline void DrawSquare(const upoint& p, unsigned size)
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

    inline void DrawCross(const upoint& p, int size = DEFAULT_CROSS_SIZE)
    {
        DrawCross(p.x, p.y, size);
    }

    inline void DrawCrossHair(int x, int y, unsigned boundaryWidth, unsigned boundaryHeight)
    {
        const int xStart = xOffset * zoomX;
        const int yStart = yOffset * zoomY;

        const int rx = (x + xOffset) * zoomX;
        const int ry = (y + yOffset) * zoomY;

        const int xEnd = (boundaryWidth + xOffset) * zoomX;
        const int yEnd = (boundaryHeight + yOffset) * zoomY;

        if (rx >= xStart && rx < xEnd) {
            dc.DrawLine(rx, yStart, rx, yEnd);
        }

        if (ry >= yStart && ry < yEnd) {
            dc.DrawLine(xStart, ry, xEnd, ry);
        }
    }

    inline void DrawCrossHair(const upoint& p, const usize& boundary)
    {
        DrawCrossHair(p.x, p.y, boundary.width, boundary.height);
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

    inline void DrawSelectedRectangle(const urect& r)
    {
        DrawSelectedRectangle(r.x, r.y, r.width, r.height);
    }

    inline void DrawSelectedSquare(const ms8point& p, unsigned size)
    {
        DrawSelectedRectangle(p.x, p.y, size, size);
    }

    inline void DrawSelectedSquare(const upoint& p, unsigned size)
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

    inline void DrawSelectedCross(const upoint& p)
    {
        DrawSelectedCross(p.x, p.y);
    }

    // Anti highlight rectangle
    // ------------------------
    inline void AntiHighlightRectangle(int x, int y, unsigned width, unsigned height)
    {
        // highlight everything that is not the rectangle
        dc.SetPen(wxNullPen);
        dc.SetBrush(ANTI_HIGHLIGHT_BRUSH);

        int cWidth, cHeight;
        dc.GetSize(&cWidth, &cHeight);

        const int xStart = (x + xOffset) * zoomX;
        const int xEnd = (x + xOffset + width) * zoomX;

        const int yStart = (y + yOffset) * zoomY;
        const int yEnd = (y + yOffset + height) * zoomY;

        dc.DrawRectangle(0, 0, xStart - 1, cHeight);

        dc.DrawRectangle(xStart - 1, 0, xEnd - xStart + 1, yStart - 1);
        dc.DrawRectangle(xStart - 1, yEnd, xEnd - xStart + 1, cHeight - yEnd);

        dc.DrawRectangle(xEnd, 0, cWidth - xEnd - 1, cHeight);
    }

    inline void AntiHighlightRectangle(const urect& r)
    {
        AntiHighlightRectangle(r.x, r.y, r.width, r.height);
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
