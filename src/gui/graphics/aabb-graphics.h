/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entity-graphics.h"
#include "two-point-rect.h"
#include "gui/imgui-drawing.h"
#include "gui/selection.h"
#include "gui/texture.h"
#include "models/common/aabb.h"
#include "models/common/image.h"
#include "models/common/ms8aabb.h"

namespace UnTech::Gui {

class AabbGraphics {
public:
    constexpr static ImU32 dragSelectCol = IM_COL32(128, 128, 128, 64);
    constexpr static ImU32 dragSelectOutline = IM_COL32(128, 128, 128, 192);

    // Line thickness of 1.0f is too small,
    constexpr static float lineThickness = 2.0f;
    constexpr static float selectedLineThickness = 4.0f;
    constexpr static float filledOutlineThickness = 1.0f;

private:
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

private:
    std::vector<SelectedAabb> _selectedAabb;

    State _currentState;

    ImVec2 _zoom;
    TwoPointRect _bounds;
    ImVec2 _sceneOffset;
    ImVec2 _offset;

    point _mouseScenePos;
    point _previousMouseScenePos;
    point _mousePos;

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

    bool inClickState() const { return _currentState == State::CLICK; }

    bool isHoveredAndNotEditing() const { return _isHovered && isNonInteractiveState(); }

    const point& mousePos() const { return _mousePos; }

    upoint mousePosUpoint() const
    {
        if (_mousePos.x >= 0 && _mousePos.y >= 0) {
            return upoint(_mousePos.x, _mousePos.y);
        }
        else {
            return upoint(INT_MAX, INT_MAX);
        }
    }

    void resetState()
    {
        _currentState = State::NONE;
    }

    void setOrigin(const int x, const int y)
    {
        const auto oldMousePos = _mousePos;

        _offset.x = _sceneOffset.x + x * _zoom.x;
        _offset.y = _sceneOffset.y + y * _zoom.y;

        _mousePos.x = _mouseScenePos.x - x;
        _mousePos.y = _mouseScenePos.y - y;

        // ::TODO find a way to eliminate this code::
        // ::: _bounds and _dragSelect are only accessed in the selected SpriteImporter Frame::
        const int deltaX = _mousePos.x - oldMousePos.x;
        const int deltaY = _mousePos.y - oldMousePos.y;

        _bounds.x1 += deltaX;
        _bounds.x2 += deltaX;
        _bounds.y1 += deltaY;
        _bounds.y2 += deltaY;

        _dragSelect.x1 += deltaX;
        _dragSelect.x2 += deltaX;
        _dragSelect.y1 += deltaY;
        _dragSelect.y2 += deltaY;
    }

    void setDisabled(bool disabled)
    {
        if (disabled) {
            _currentState = State::DISABLED;
        }
        else if (_currentState == State::DISABLED) {
            _currentState = State::NONE;
        }
    }

    template <typename... SelectionT>
    void startLoop(const char* strId, const rect& sceneRect, const rect& bounds, const ImVec2& zoom,
                   SelectionT*... sel)
    {
        _selectedAabb.clear();
        _lastClickedSelector = nullptr;
        _isHovered = false;
        _previouslySelectedItemClicked = false;

        _hasSingleSelection = (0 + ... + int(sel->hasSingleSelection())) == 1;

        _bounds.x1 = bounds.left();
        _bounds.x2 = bounds.right();
        _bounds.y1 = bounds.top();
        _bounds.y2 = bounds.bottom();

        _zoom = zoom;
        const ImVec2 drawSize(sceneRect.width * zoom.x, sceneRect.height * zoom.y);
        const ImVec2 screenOffset = centreOffset(drawSize);

        _sceneOffset = screenOffset - ImVec2(sceneRect.x * zoom.x, sceneRect.y * zoom.y);
        _offset = _sceneOffset;

        ImGui::SetCursorScreenPos(screenOffset);
        ImGui::InvisibleButton(strId, drawSize);

        _mouseScenePos = toPoint(ImGui::GetMousePos());
        _mousePos = _mouseScenePos;

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
                    _dragSelect.x1 = std::min(_mouseScenePos.x, _previousMouseScenePos.x);
                    _dragSelect.x2 = std::max(_mouseScenePos.x, _previousMouseScenePos.x);
                    _dragSelect.y1 = std::min(_mouseScenePos.y, _previousMouseScenePos.y);
                    _dragSelect.y2 = std::max(_mouseScenePos.y, _previousMouseScenePos.y);

                    (..., sel->clearSelection());
                }
                else {
                    _currentState = State::NONE;
                }
            } break;

            case State::MOVE_DRAG:
            case State::RESIZE_DRAG: {
                if (ImGui::IsMouseDown(0)) {
                    if (_bounds.contains(_mouseScenePos)) {
                        _dragMove.x = _mouseScenePos.x - _previousMouseScenePos.x;
                        _dragMove.y = _mouseScenePos.y - _previousMouseScenePos.y;
                        _previousMouseScenePos = _mouseScenePos;
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

    template <typename... SelectionT>
    void startLoop(const char* strId, const rect& sceneRect, const ImVec2& zoom,
                   SelectionT*... sel)
    {
        startLoop(strId, sceneRect, sceneRect, zoom, sel...);
    }

    void drawBackgroundColor(ImDrawList* drawList, const ImU32 color)
    {
        const auto pos = ImGui::GetWindowPos();
        const auto size = ImGui::GetWindowSize();

        drawList->AddRectFilled(pos, pos + size, color, 0.0f, ImDrawCornerFlags_None);
    }

    template <typename RectT>
    void drawCrosshair(ImDrawList* drawList, const int x, const int y, const RectT& rect, const ImU32 color)
    {
        const ImVec2 p = toVec2(x, y);
        const ImVec2 min = toVec2(rect.left(), rect.top());
        const ImVec2 max = toVec2(rect.right(), rect.bottom());

        drawList->AddLine(ImVec2(p.x, min.y), ImVec2(p.x, max.y), color);
        drawList->AddLine(ImVec2(min.x, p.y), ImVec2(max.x, p.y), color);
    }

    void drawBoundedCrosshair(ImDrawList* drawList, const int x, const int y, const ImU32 color)
    {
        const ImVec2 p = toVec2(x, y);
        const ImVec2 min = toVec2(_bounds.x1, _bounds.y1);
        const ImVec2 max = toVec2(_bounds.x2, _bounds.y2);

        drawList->AddLine(ImVec2(p.x, min.y), ImVec2(p.x, max.y), color);
        drawList->AddLine(ImVec2(min.x, p.y), ImVec2(max.x, p.y), color);
    }

    template <typename RectT>
    void drawAntiHighlight(ImDrawList* drawList, const RectT& rect, const ImU32 color)
    {
        const ImVec2 p1 = toVec2(rect.left(), rect.top());
        const ImVec2 p2 = toVec2(rect.right(), rect.bottom());

        const ImVec2 wp1 = ImGui::GetWindowPos();
        const ImVec2 wp2 = wp1 + ImGui::GetWindowSize();

        // top
        if (wp1.y < p1.y) {
            drawList->AddRectFilled(ImVec2(wp1.x, wp1.y), ImVec2(wp2.x, p1.y), color, 0.0f, ImDrawCornerFlags_None);
        }
        // bottom
        if (wp2.y > p2.y) {
            drawList->AddRectFilled(ImVec2(wp1.x, p2.y), ImVec2(wp2.x, wp2.y), color, 0.0f, ImDrawCornerFlags_None);
        }
        // left
        if (wp1.x < p1.x && p2.y > p1.y) {
            drawList->AddRectFilled(ImVec2(wp1.x, p1.y), ImVec2(p1.x, p2.y), color, 0.0f, ImDrawCornerFlags_None);
        }
        // right
        if (wp2.x > p2.x && p2.y > p1.y) {
            drawList->AddRectFilled(ImVec2(p2.x, p1.y), ImVec2(wp2.x, p2.y), color, 0.0f, ImDrawCornerFlags_None);
        }
    }

    inline void drawRectFilled(ImDrawList* drawList, const TwoPointRect& rect, const ImU32 fillCol, const ImU32 outlineCol, const bool selected = false)
    {
        const ImVec2 pMin = toVec2(rect.x1, rect.y1);
        const ImVec2 pMax = toVec2(rect.x2, rect.y2);

        drawList->AddRectFilled(pMin, pMax, fillCol, 0.0f, ImDrawCornerFlags_None);
        drawList->AddRect(pMin, pMax, outlineCol, 0.0f, ImDrawCornerFlags_None, filledOutlineThickness);

        if (selected) {
            _selectedAabb.emplace_back(pMin, pMax, outlineCol);
        }
    }

    template <typename PointT>
    inline void addPointRect(ImDrawList* drawList, const PointT* point, const ImU32 color, const bool selected = false)
    {
        const ImVec2 pMin = toVec2(point->x, point->y);
        const ImVec2 pMax = toVec2(point->x + 1, point->y + 1);

        drawList->AddRect(pMin, pMax, color, 0.0f, ImDrawCornerFlags_None, lineThickness);

        if (selected) {
            _selectedAabb.emplace_back(pMin, pMax, color);
        }
    }

    template <typename RectT>
    inline void addRect(ImDrawList* drawList, const RectT* rect, const ImU32 color, const bool selected = false)
    {
        const ImVec2 pMin = toVec2(rect->left(), rect->top());
        const ImVec2 pMax = toVec2(rect->right(), rect->bottom());

        drawList->AddRect(pMin, pMax, color, 0.0f, ImDrawCornerFlags_None, lineThickness);

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

    template <typename PointT, typename SelectionT>
    void addEntity(ImDrawList* drawList, PointT* point,
                   ImTextureID textureId, const DrawEntitySettings& ds,
                   const ImU32 fillCol, const ImU32 outlineCol, const ImU32 tintColor,
                   SelectionT* sel, const unsigned index)
    {
        bool selected = sel->isSelected(index);

        const TwoPointRect r{
            int(point->x) + ds.hitboxRect.x1,
            int(point->x) + ds.hitboxRect.x2,
            int(point->y) + ds.hitboxRect.y1,
            int(point->y) + ds.hitboxRect.y2
        };

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
            if (_dragSelect.contains(point->x, point->y)) {
                sel->appendSelection(index);
            }
        } break;

        case State::MOVE_DRAG: {
            if (selected) {
                if constexpr (std::is_unsigned_v<decltype(PointT::x)>) {
                    static_assert(std::is_unsigned_v<decltype(PointT::y)>);
                    point->x = std::max(0, std::clamp<int>(point->x + _dragMove.x, _bounds.x1, _bounds.x2 - 1));
                    point->y = std::max(0, std::clamp<int>(point->y + _dragMove.y, _bounds.y1, _bounds.y2 - 1));
                }
                else {
                    static_assert(std::is_signed_v<decltype(PointT::y)>);
                    point->x = std::clamp<int>(point->x + _dragMove.x, _bounds.x1, _bounds.x2 - 1);
                    point->y = std::clamp<int>(point->y + _dragMove.y, _bounds.y1, _bounds.y2 - 1);
                }
            }
        } break;

        case State::RESIZE_DRAG: {
        } break;
        }

        const TwoPointRect ir{
            int(point->x) + ds.imageRect.x1,
            int(point->x) + ds.imageRect.x2,
            int(point->y) + ds.imageRect.y1,
            int(point->y) + ds.imageRect.y2
        };

        drawList->AddImage(textureId, toVec2(ir.x1, ir.y1), toVec2(ir.x2, ir.y2),
                           ds.uvMin, ds.uvMax, tintColor);

        if (_isHovered || selected) {
            drawRectFilled(drawList, r, fillCol, outlineCol, selected);
        }
    }

    template <typename PointT>
    inline void drawEntity(ImDrawList* drawList, const PointT* point,
                           ImTextureID textureId, const DrawEntitySettings& ds, const ImU32 tintColor)
    {
        const TwoPointRect r{
            int(point->x) + ds.imageRect.x1,
            int(point->x) + ds.imageRect.x2,
            int(point->y) + ds.imageRect.y1,
            int(point->y) + ds.imageRect.y2
        };

        drawList->AddImage(textureId, toVec2(r.x1, r.y1), toVec2(r.x2, r.y2),
                           ds.uvMin, ds.uvMax, tintColor);
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
            int(rect->left()),
            int(rect->right()),
            int(rect->top()),
            int(rect->bottom()),
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
                rect->x = std::clamp<int>(rect->x + _dragMove.x, _bounds.x1, _bounds.x2 - rect->width);
                rect->y = std::clamp<int>(rect->y + _dragMove.y, _bounds.y1, _bounds.y2 - rect->height);
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
                    rect->width = std::clamp<int>(_mousePos.x - r.x1, 1, _bounds.x2 - r.x1);
                }

                if (_resizeNodes & Resize_Top) {
                    int newY = std::clamp<int>(_mousePos.y, _bounds.y1, r.y2 - 1);
                    assert(newY < r.y2);
                    rect->height = r.y2 - newY;
                    rect->y = newY;
                }
                else if (_resizeNodes & Resize_Bottom) {
                    rect->height = std::clamp<int>(_mousePos.y - r.y1, 1, _bounds.y2 - r.y1);
                }
            }
        } break;
        }

        addRect(drawList, rect, color, selected);
    }

    template <typename PointT>
    void addFixedSizeSquare(ImDrawList* drawList, const PointT* point, const unsigned squareSize,
                            const ImU32 color, bool selected = false)
    {
        const ImVec2 pMin = toVec2(point->x, point->y);
        const ImVec2 pMax = toVec2(point->x + squareSize, point->y + squareSize);

        drawList->AddRect(pMin, pMax, color, 0.0f, ImDrawCornerFlags_None, lineThickness);

        if (selected) {
            _selectedAabb.emplace_back(pMin, pMax, color);
        }
    }

    template <typename PointT, typename SelectionT>
    void addFixedSizeSquare(ImDrawList* drawList, PointT* point, unsigned squareSize, const ImU32 color,
                            SelectionT* sel, const unsigned index)
    {
        bool selected = sel->isSelected(index);

        const TwoPointRect r{
            int(point->x),
            int(point->x + squareSize),
            int(point->y),
            int(point->y + squareSize),
        };

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
            }
        } break;

        case State::MOVE_DRAG: {
            if (selected) {
                point->x = std::clamp<int>(point->x + _dragMove.x, _bounds.x1, _bounds.x2 - squareSize);
                point->y = std::clamp<int>(point->y + _dragMove.y, _bounds.y1, _bounds.y2 - squareSize);
            }
        } break;

        case State::RESIZE_DRAG: {
        } break;
        }

        addFixedSizeSquare(drawList, point, squareSize, color, selected);
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
                drawList->AddRect(pMin, pMax, outlineColor, 0.0f, ImDrawCornerFlags_None, lineThickness);
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
                point->x = std::clamp<int>(point->x + _dragMove.x, _bounds.x1, _bounds.x2 - imageSize);
                point->y = std::clamp<int>(point->y + _dragMove.y, _bounds.y1, _bounds.y2 - imageSize);
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
                drawList->AddRect(pMin, pMax, outlineColor, 0.0f, ImDrawCornerFlags_None, lineThickness);
                _selectedAabb.emplace_back(pMin, pMax, outlineColor);
            }
            else {
                switch (_currentState) {
                case State::DISABLED:
                case State::NONE:
                case State::CLICK:
                case State::SELECT_DRAG: {
                    drawList->AddRect(pMin, pMax, outlineColor, 0.0f, ImDrawCornerFlags_None, lineThickness);
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

    void drawImage(ImDrawList* drawList, const Texture& texture, const int x, const int y)
    {
        const ImVec2 pMin = toVec2(x, y);
        const ImVec2 pMax = toVec2(x + texture.width(), y + texture.height());

        drawList->AddImage(texture.imguiTextureId(), pMin, pMax);
    }

    template <typename... SelectionT>
    void endLoop(ImDrawList* drawList, SelectionT*... selection)
    {
        // MUST not use _mousePos, _bounds or _dragSelect in the function.

        for (const auto& sel : _selectedAabb) {
            drawList->AddRect(sel.pMin, sel.pMax, sel.col, 0.0f, ImDrawCornerFlags_None, selectedLineThickness);
        }

        switch (_currentState) {
        case State::NONE:
        case State::DISABLED: {
        } break;

        case State::CLICK: {
            _previousMouseScenePos = _mouseScenePos;

            if (_previouslySelectedItemClicked) {
                _currentState = State::MOVE_DRAG;
            }
            else if (_lastClickedSelector != nullptr) {
                const bool ctrlClicked = ImGui::GetIO().KeyCtrl;
                (..., endLoop_processLastClickedSelector(selection, ctrlClicked));

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

private:
    template <typename SelectionT>
    void endLoop_processLastClickedSelector(SelectionT* sel, bool ctrlClicked);
};

template <>
inline void AabbGraphics::endLoop_processLastClickedSelector<GroupMultipleSelection>(GroupMultipleSelection* sel, bool ctrlClicked)
{
    if (not ctrlClicked) {
        // Not a ctrl click - change all child selectors
        for (MultipleSelection& childSel : sel->childSelections) {
            if (&childSel == _lastClickedSelector) {
                childSel.setSelected(_selectedIndex);
            }
            else {
                childSel.clearSelection();
            }
        }
    }
    else {
        // Ctrl click - only change the _lastClickedSelector
        for (MultipleSelection& childSel : sel->childSelections) {
            if (&childSel == _lastClickedSelector) {
                childSel.selectionClicked(_selectedIndex, true);
            }
        }
    }
}

template <typename SelectionT>
void AabbGraphics::endLoop_processLastClickedSelector(SelectionT* sel, bool ctrlClicked)
{
    if (sel == _lastClickedSelector) {
        sel->selectionClicked(_selectedIndex, ctrlClicked);
    }
    else {
        sel->clearSelection();
    }
}

}
