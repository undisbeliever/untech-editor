/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/imgui-drawing.h"
#include "gui/selection.h"
#include "gui/texture.h"
#include "models/common/aabb.h"
#include "models/common/image.h"
#include "models/common/ms8aabb.h"

namespace UnTech::Gui {

class AabbGraphics {
private:
    constexpr static ImU32 dragSelectCol = IM_COL32(128, 128, 128, 64);
    constexpr static ImU32 dragSelectOutline = IM_COL32(128, 128, 128, 192);

    // Line thickness of 1.0f is too small,
    constexpr static float _lineThickness = 2.0f;
    constexpr static float _selectedLineThickness = 4.0f;

    struct SelectedAabb {
        ImVec2 pMin;
        ImVec2 pMax;
        ImU32 col;

        SelectedAabb(const ImVec2& pMin_, const ImVec2& pMax_, ImU32 col_)
            : pMin(pMin_)
            , pMax(pMax_)
            , col(col_)
        {
        }
    };

    struct TwoPointRect {
        // NOTE: This code assumes x2 > x1 and y2 > y1.

        int x1, x2;
        int y1, y2;

        TwoPointRect() = default;

        TwoPointRect(int x1_, int x2_, int y1_, int y2_)
            : x1(x1_)
            , x2(x2_)
            , y1(y1_)
            , y2(y2_)
        {
        }

        bool contains(int x, int y) const
        {
            return x >= this->x1 && x < this->x2
                   && y >= this->y1 && y <= this->y2;
        }

        bool contains(const point p) const
        {
            return p.x >= this->x1 && p.x < this->x2
                   && p.y >= this->y1 && p.y <= this->y2;
        }

        bool contains(const TwoPointRect r) const
        {
            return r.x1 >= this->x1 && r.x2 <= this->x2
                   && r.y1 >= this->y1 && r.y2 <= this->y2;
        }
    };

    enum class State {
        DISABLED,
        NONE,
        CLICK,
        SELECT_DRAG,
        MOVE_DRAG,
        RESIZE_DRAG,

        EDITING_FINISHED,
    };

    enum ResizeNodes {
        Resize_None = 0,
        Resize_Top = 1 << 0,
        Resize_Bottom = 1 << 1,
        Resize_Left = 1 << 2,
        Resize_Right = 1 << 3,
        Resize_TopLeft = Resize_Top | Resize_Left,
        Resize_TopRight = Resize_Top | Resize_Right,
        Resize_BottomLeft = Resize_Bottom | Resize_Left,
        Resize_BottomRight = Resize_Bottom | Resize_Right
    };

    std::vector<SelectedAabb> _selectedAabb;

    State _currentState;

    TwoPointRect _bounds;
    ImVec2 _offset;
    ImVec2 _zoom;

    point _mousePos;
    point _previousMousePos;

    TwoPointRect _dragSelect;
    point _dragMove;
    ResizeNodes _resizeNodes;

    const void* _lastClickedSelector;
    unsigned _selectedIndex;

    bool _isHovered;
    bool _hasSingleSelection;
    bool _previouslySelectedItemClicked;

public:
    AabbGraphics()
    {
        resetState();
    }
    ~AabbGraphics() = default;

    bool isNonInteractiveState() const
    {
        return _currentState == State::NONE || _currentState == State::DISABLED;
    }

    point toPoint(const ImVec2& globalPos) const
    {
        const ImVec2 pos = ImVec2(globalPos.x - _offset.x, globalPos.y - _offset.y);
        return point(std::floor(pos.x / _zoom.x), std::floor(pos.y / _zoom.y));
    }

    ImVec2 toVec2(const int x, const int y) const
    {
        return ImVec2((x * _zoom.x) + _offset.x, (y * _zoom.y) + _offset.y);
    }

    bool isHoveredAndNotEditing() const { return _isHovered && isNonInteractiveState(); }

    void resetState()
    {
        _currentState = State::NONE;
    }

    void setBounds(const rect& rect)
    {
        _bounds.x1 = rect.left();
        _bounds.x2 = rect.right();
        _bounds.y1 = rect.top();
        _bounds.y2 = rect.bottom();
    }

    template <typename... SelectionT>
    void startLoop(const char* strId, const rect& rect, const ImVec2& zoom,
                   SelectionT*... sel)
    {
        _selectedAabb.clear();
        _lastClickedSelector = nullptr;
        _isHovered = false;
        _previouslySelectedItemClicked = false;

        _hasSingleSelection = (0 + ... + int(sel->hasSingleSelection())) == 1;

        setBounds(rect);

        _zoom = zoom;
        const ImVec2 drawSize(rect.width * zoom.x, rect.height * zoom.y);
        const ImVec2 screenOffset = centreOffset(drawSize);

        _offset = screenOffset - ImVec2(rect.x * zoom.x, rect.y * zoom.y);

        ImGui::SetCursorScreenPos(screenOffset);
        ImGui::InvisibleButton(strId, drawSize);

        _mousePos = toPoint(ImGui::GetMousePos());

        if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
                _currentState = State::NONE;
            }

            switch (_currentState) {
            case State::DISABLED: {
                _resizeNodes = Resize_None;
            } break;

            case State::NONE: {
                _resizeNodes = Resize_None;

                if (ImGui::IsMouseClicked(0)) {
                    _currentState = State::CLICK;
                }
            } break;

            case State::CLICK: {
                _resizeNodes = Resize_None;
            } break;

            case State::SELECT_DRAG: {
                if (ImGui::IsMouseDown(0)) {
                    _dragSelect.x1 = std::min(_mousePos.x, _previousMousePos.x);
                    _dragSelect.x2 = std::max(_mousePos.x, _previousMousePos.x);
                    _dragSelect.y1 = std::min(_mousePos.y, _previousMousePos.y);
                    _dragSelect.y2 = std::max(_mousePos.y, _previousMousePos.y);

                    (..., sel->clearSelection());
                }
                else {
                    _currentState = State::NONE;
                }
            } break;

            case State::MOVE_DRAG:
            case State::RESIZE_DRAG: {
                if (ImGui::IsMouseDown(0)) {
                    if (_bounds.contains(_mousePos)) {
                        _dragMove.x = _mousePos.x - _previousMousePos.x;
                        _dragMove.y = _mousePos.y - _previousMousePos.y;
                        _previousMousePos = _mousePos;
                    }
                    else {
                        _dragMove.x = 0;
                        _dragMove.y = 0;
                    }
                }
                else {
                    _currentState = State::EDITING_FINISHED;
                }

                if (_currentState == State::RESIZE_DRAG) {
                    // Only allow resizing if a single item is selected
                    if (_hasSingleSelection == false) {
                        _currentState = State::EDITING_FINISHED;
                    }
                }
            } break;

            case State::EDITING_FINISHED: {
                _currentState = State::NONE;
            } break;
            }
        }
        else {
            // Not active or hoverred
            switch (_currentState) {
            case State::DISABLED:
            case State::NONE: {
            } break;

            case State::CLICK:
            case State::SELECT_DRAG:
            case State::EDITING_FINISHED: {
                _currentState = State::NONE;
            } break;

            case State::MOVE_DRAG:
            case State::RESIZE_DRAG: {
                _currentState = State::EDITING_FINISHED;
            } break;
            }
        }
    }

    void drawBackgroundColor(ImDrawList* drawList, const ImU32 color)
    {
        const auto pos = ImGui::GetWindowPos();
        const auto size = ImGui::GetWindowSize();

        drawList->AddRectFilled(pos, pos + size, color, 0.0f, ImDrawCornerFlags_None);
    }

    void drawBoundedCrosshair(ImDrawList* drawList, const int x, const int y, const ImU32 color)
    {
        const ImVec2 p = toVec2(x, y);
        const ImVec2 min = toVec2(_bounds.x1, _bounds.y1);
        const ImVec2 max = toVec2(_bounds.x2, _bounds.y2);

        drawList->AddLine(ImVec2(p.x, min.y), ImVec2(p.x, max.y), color);
        drawList->AddLine(ImVec2(min.x, p.y), ImVec2(max.x, p.y), color);
    }

    template <typename PointT>
    inline void addPointRect(ImDrawList* drawList, const PointT* point, const ImU32 color, const bool selected = false)
    {
        const ImVec2 pMin = toVec2(point->x, point->y);
        const ImVec2 pMax = toVec2(point->x + 1, point->y + 1);

        drawList->AddRect(pMin, pMax, color, 0.0f, ImDrawCornerFlags_None, _lineThickness);

        if (selected) {
            _selectedAabb.emplace_back(pMin, pMax, color);
        }
    }

    template <typename RectT>
    inline void addRect(ImDrawList* drawList, const RectT* rect, const ImU32 color, const bool selected = false)
    {
        const ImVec2 pMin = toVec2(rect->left(), rect->top());
        const ImVec2 pMax = toVec2(rect->right(), rect->bottom());

        drawList->AddRect(pMin, pMax, color, 0.0f, ImDrawCornerFlags_None, _lineThickness);

        if (selected) {
            _selectedAabb.emplace_back(pMin, pMax, color);
        }
    }

    template <typename PointT, typename SelectionT>
    void addPointRect(ImDrawList* drawList, PointT* point, const ImU32 color,
                      SelectionT* sel, const unsigned index)
    {
        bool selected = sel->isSelected(index);

        // Convert to int to silence unsigned/signed comparison warning
        const int x = point->x;
        const int y = point->y;

        _isHovered = _mousePos.x == x && _mousePos.y == y;

        switch (_currentState) {
        case State::EDITING_FINISHED:
        case State::DISABLED: {
        } break;

        case State::NONE: {
        } break;

        case State::CLICK: {
            if (_isHovered) {
                _lastClickedSelector = sel;
                _selectedIndex = index;

                _previouslySelectedItemClicked |= selected;
            }
        } break;

        case State::SELECT_DRAG: {
            if (_dragSelect.contains(x, y)) {
                sel->appendSelection(index);
            }
        } break;

        case State::MOVE_DRAG: {
            if (selected) {
                point->x = std::clamp<int>(point->x + _dragMove.x, _bounds.x1, _bounds.x2 - 1);
                point->y = std::clamp<int>(point->y + _dragMove.y, _bounds.y1, _bounds.y2 - 1);
            }
        } break;

        case State::RESIZE_DRAG: {
        } break;
        }

        addPointRect(drawList, point, color, selected);
    }

    ResizeNodes setResizeMouseCursor(const TwoPointRect& r) const
    {
        // Using + 1 for top/left so user can resize 1 pixel wide/tall rects

        int rn = 0;
        if (_mousePos.y >= (r.y1 - 1) && _mousePos.y <= r.y2) {
            if (_mousePos.x == (r.x1 - 1)) {
                rn |= Resize_Left;
            }
            if (_mousePos.x == r.x2) {
                rn |= Resize_Right;
            }
        }
        if (_mousePos.x >= (r.x1 - 1) && _mousePos.x <= r.x2) {
            if (_mousePos.y == (r.y1 - 1)) {
                rn |= Resize_Top;
            }
            if (_mousePos.y == r.y2) {
                rn |= Resize_Bottom;
            }
        }

        switch (ResizeNodes(rn)) {
        case Resize_Top:
        case Resize_Bottom:
        case Resize_Left:
        case Resize_Right:
        case Resize_TopLeft:
        case Resize_BottomRight:
        case Resize_TopRight:
        case Resize_BottomLeft:
            return ResizeNodes(rn);

        default:
            return Resize_None;
        }
        return Resize_None;
    }

    void setResizeNodeMouseCursor(const ResizeNodes rn) const
    {
        switch (rn) {
        case Resize_None:
            break;

        case Resize_Top:
        case Resize_Bottom:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            break;

        case Resize_Left:
        case Resize_Right:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            break;

        case Resize_TopLeft:
        case Resize_BottomRight:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
            break;

        case Resize_TopRight:
        case Resize_BottomLeft:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
            break;
        }
    }

    template <typename RectT, typename SelectionT>
    void addRect(ImDrawList* drawList, RectT* rect, const ImU32 color,
                 SelectionT* sel, const unsigned index)
    {
        bool selected = sel->isSelected(index);

        const TwoPointRect r{
            rect->left(),
            rect->right(),
            rect->top(),
            rect->bottom(),
        };

        _isHovered = r.contains(_mousePos);

        switch (_currentState) {
        case State::EDITING_FINISHED:
        case State::DISABLED: {
        } break;

        case State::NONE: {
            if (selected && _hasSingleSelection) {
                const auto rn = setResizeMouseCursor(r);
                setResizeNodeMouseCursor(rn);
            }
        } break;

        case State::CLICK: {
            if (selected && _hasSingleSelection) {
                auto rn = setResizeMouseCursor(r);
                if (rn != Resize_None) {
                    _resizeNodes = rn;
                    setResizeNodeMouseCursor(_resizeNodes);
                    _currentState = State::RESIZE_DRAG;
                }
            }
            if (_isHovered && _currentState == State::CLICK) {
                _lastClickedSelector = sel;
                _selectedIndex = index;

                _previouslySelectedItemClicked |= selected;
            }
        } break;

        case State::SELECT_DRAG: {
            if (_dragSelect.contains(r)) {
                sel->appendSelection(index);
                selected = true;
            }
        } break;

        case State::MOVE_DRAG: {
            if (selected) {
                rect->x = std::clamp<int>(rect->x + _dragMove.x, _bounds.x1, _bounds.x2 - rect->width + 1);
                rect->y = std::clamp<int>(rect->y + _dragMove.y, _bounds.y1, _bounds.y2 - rect->height + 1);
            }
        } break;

        case State::RESIZE_DRAG: {
            if (selected) {
                setResizeNodeMouseCursor(_resizeNodes);

                if (_resizeNodes & Resize_Left) {
                    int newX = std::clamp<int>(_mousePos.x, _bounds.x1, r.x2 - 1);
                    assert(newX < r.x2);
                    rect->width = r.x2 - newX;
                    rect->x = newX;
                }
                else if (_resizeNodes & Resize_Right) {
                    rect->width = std::clamp<int>(_mousePos.x - r.x1, 1, _bounds.x2 - r.x1 + 1);
                }

                if (_resizeNodes & Resize_Top) {
                    int newY = std::clamp<int>(_mousePos.y, _bounds.y1, r.y2 - 1);
                    assert(newY < r.y2);
                    rect->height = r.y2 - newY;
                    rect->y = newY;
                }
                else if (_resizeNodes & Resize_Bottom) {
                    rect->height = std::clamp<int>(_mousePos.y - r.y1, 1, _bounds.y2 - r.y1 + 1);
                }
            }
        } break;
        }

        addRect(drawList, rect, color, selected);
    }

    template <typename PointT>
    void addSquareImage(ImDrawList* drawList, PointT* point, const unsigned imageSize,
                        const ImTextureID textureId, const ImVec2& uv0, const ImVec2& uv1, const ImU32 outlineColor)
    {
        const TwoPointRect r(
            point->x,
            point->x + imageSize,
            point->y,
            point->y + imageSize);

        const ImVec2 pMin = toVec2(point->x, point->y);
        const ImVec2 pMax = toVec2(point->x + imageSize, point->y + imageSize);

        drawList->AddImage(textureId, pMin, pMax, uv0, uv1);

        if (r.contains(_mousePos)) {
            switch (_currentState) {
            case State::DISABLED:
            case State::NONE:
            case State::CLICK:
            case State::SELECT_DRAG: {
                drawList->AddRect(pMin, pMax, outlineColor, 0.0f, ImDrawCornerFlags_None, _lineThickness);
            } break;

            case State::MOVE_DRAG:
            case State::RESIZE_DRAG:
            case State::EDITING_FINISHED: {
                // Do not show outline of unselected images when moving or resizing items
            } break;
            }
        }
    }

    template <typename PointT, typename SelectionT>
    void addSquareImage(ImDrawList* drawList, PointT* point, const unsigned imageSize,
                        const ImTextureID textureId, const ImVec2& uv0, const ImVec2& uv1, const ImU32 outlineColor,
                        SelectionT* sel, const unsigned index)
    {
        bool selected = sel->isSelected(index);

        // Convert to int to silence unsigned/signed comparison warning
        const TwoPointRect r(
            point->x,
            point->x + imageSize,
            point->y,
            point->y + imageSize);

        _isHovered = r.contains(_mousePos);

        switch (_currentState) {
        case State::EDITING_FINISHED:
        case State::DISABLED: {
        } break;

        case State::NONE: {
        } break;

        case State::CLICK: {
            if (_isHovered) {
                _lastClickedSelector = sel;
                _selectedIndex = index;

                _previouslySelectedItemClicked |= selected;
            }
        } break;

        case State::SELECT_DRAG: {
            if (_dragSelect.contains(r)) {
                sel->appendSelection(index);
                selected = true;
            }
        } break;

        case State::MOVE_DRAG: {
            if (selected) {
                point->x = std::clamp<int>(point->x + _dragMove.x, _bounds.x1, _bounds.x2 - imageSize + 1);
                point->y = std::clamp<int>(point->y + _dragMove.y, _bounds.y1, _bounds.y2 - imageSize + 1);
            }
        } break;

        case State::RESIZE_DRAG: {
        } break;
        }

        const ImVec2 pMin = toVec2(point->x, point->y);
        const ImVec2 pMax = toVec2(point->x + imageSize, point->y + imageSize);

        drawList->AddImage(textureId, pMin, pMax, uv0, uv1);

        if (_isHovered || selected) {

            if (selected) {
                drawList->AddRect(pMin, pMax, outlineColor, 0.0f, ImDrawCornerFlags_None, _lineThickness);
                _selectedAabb.emplace_back(pMin, pMax, outlineColor);
            }
            else {
                switch (_currentState) {
                case State::DISABLED:
                case State::NONE:
                case State::CLICK:
                case State::SELECT_DRAG: {
                    drawList->AddRect(pMin, pMax, outlineColor, 0.0f, ImDrawCornerFlags_None, _lineThickness);
                } break;

                case State::MOVE_DRAG:
                case State::RESIZE_DRAG:
                case State::EDITING_FINISHED: {
                    // Do not show outline of unselected images when moving or resizing items
                } break;
                }
            }
        }
    }

    template <typename... SelectionT>
    void endLoop(ImDrawList* drawList, SelectionT*... selection)
    {
        for (const auto& sel : _selectedAabb) {
            drawList->AddRect(sel.pMin, sel.pMax, sel.col, 0.0f, ImDrawCornerFlags_None, _selectedLineThickness);
        }

        switch (_currentState) {
        case State::NONE:
        case State::DISABLED: {
        } break;

        case State::CLICK: {
            _previousMousePos = _mousePos;

            if (_previouslySelectedItemClicked) {
                _currentState = State::MOVE_DRAG;
            }
            else if (_lastClickedSelector != nullptr) {
                const bool ctrlClicked = ImGui::GetIO().KeyCtrl;

                auto f = [&](auto* sel) {
                    if (sel == _lastClickedSelector) {
                        sel->selectionClicked(_selectedIndex, ctrlClicked);
                    }
                    else {
                        sel->clearSelection();
                    }
                };
                (..., f(selection));

                _currentState = State::MOVE_DRAG;
            }
            else {
                // No item selected, this is a click+drag selection
                _currentState = State::SELECT_DRAG;
            }
        } break;

        case State::SELECT_DRAG: {
            const ImVec2 p1 = ImGui::GetMousePos();
            const ImVec2 p2 = p1 - ImGui::GetMouseDragDelta(0);

            drawList->AddRectFilled(p1, p2, dragSelectCol, 0.0f, ImDrawCornerFlags_None);
            drawList->AddRect(p1, p2, dragSelectOutline, 0.0f, ImDrawCornerFlags_None, 1.0f);
        } break;

        case State::MOVE_DRAG:
        case State::RESIZE_DRAG:
        case State::EDITING_FINISHED: {
        } break;
        }
    }

    bool isEditingFinished() { return _currentState == State::EDITING_FINISHED; }
};
}
