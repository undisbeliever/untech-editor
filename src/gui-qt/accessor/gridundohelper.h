/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/call.h"
#include "models/common/grid.h"
#include "models/common/vectorset-upoint.h"
#include <QCoreApplication>
#include <QUndoCommand>
#include <functional>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

template <class AccessorT>
class GridUndoHelper {

public:
    using DataT = typename AccessorT::DataT;
    using GridT = typename AccessorT::GridT;
    using index_type = typename AccessorT::index_type;
    using selection_type = typename AccessorT::selection_type;
    using ArgsT = typename AccessorT::ArgsT;

    static_assert(std::is_same<GridT, grid<DataT>>::value, "Unexpected GridT type");
    static_assert(std::is_same<index_type, upoint>::value, "Unexpected index_type type");
    static_assert(std::is_same<selection_type, upoint_vectorset>::value, "Unexpected selection_type type");

private:
    static inline QString tr(const char* s)
    {
        return QCoreApplication::tr(s);
    }

    class BaseCommand : public QUndoCommand {
    protected:
        AccessorT* const _accessor;
        const ArgsT _args;

    public:
        BaseCommand(AccessorT* accessor, const ArgsT& args,
                    const QString& text)
            : QUndoCommand(text)
            , _accessor(accessor)
            , _args(args)
        {
        }
        ~BaseCommand() = default;

    protected:
        inline GridT* getGrid()
        {
            auto f = std::mem_fn(&AccessorT::getGrid);
            return mem_fn_call(f, _accessor, _args);
        }

        inline void emitGridChanged()
        {
            auto f = std::mem_fn(&AccessorT::gridChanged);
            mem_fn_call(f, _accessor, _args);

            _accessor->resourceItem()->dataChanged();
        }

        // When this signal is emitted you MUST close all editors
        // accessing the list to prevent data corruption
        inline void emitGridAboutToBeResized()
        {
            auto f = std::mem_fn(&AccessorT::gridAboutToBeResized);
            mem_fn_call(f, _accessor, _args);
        }

        inline void emitGridResized()
        {
            auto f = std::mem_fn(&AccessorT::gridResized);
            mem_fn_call(f, _accessor, _args);
        }
    };

    class EditGridCommand : public BaseCommand {
    private:
        const GridT _oldGrid;
        const GridT _newGrid;

    public:
        EditGridCommand(AccessorT* accessor, const ArgsT& args,
                        const grid<DataT>& oldGrid, const grid<DataT>& newGrid,
                        const QString& text)
            : BaseCommand(accessor, args, text)
            , _oldGrid(oldGrid)
            , _newGrid(newGrid)
        {
        }

        EditGridCommand(AccessorT* accessor, const ArgsT& args,
                        const grid<DataT>& oldGrid, grid<DataT>&& newGrid,
                        const QString& text)
            : BaseCommand(accessor, args, text)
            , _oldGrid(oldGrid)
            , _newGrid(std::move(newGrid))
        {
        }
        ~EditGridCommand() = default;

        virtual void undo() final
        {
            setGrid(_oldGrid);
        }

        virtual void redo() final
        {
            setGrid(_newGrid);
        }

        inline void setGrid(const GridT& toReplace)
        {
            GridT* grid = this->getGrid();
            Q_ASSERT(grid);

            bool gridResized = grid->size() != toReplace.size();

            if (gridResized) {
                this->emitGridAboutToBeResized();
            }

            *grid = toReplace;

            if (gridResized) {
                this->emitGridResized();
            }

            this->emitGridChanged();
        }
    };

    class EditCellsCommand : public BaseCommand {
    private:
        const upoint _position;
        const GridT _oldCells;
        const GridT _newCells;

    public:
        EditCellsCommand(AccessorT* accessor, const ArgsT& args, const upoint& position,
                         grid<DataT>&& oldCells, const grid<DataT>& newCells,
                         const QString& text)
            : BaseCommand(accessor, args, text)
            , _position(position)
            , _oldCells(std::move(oldCells))
            , _newCells(newCells)
        {
        }
        EditCellsCommand(AccessorT* accessor, const ArgsT& args, const upoint& position,
                         grid<DataT>&& oldCells, grid<DataT>&& newCells,
                         const QString& text)
            : BaseCommand(accessor, args, text)
            , _position(position)
            , _oldCells(std::move(oldCells))
            , _newCells(std::move(newCells))
        {
        }
        ~EditCellsCommand() = default;

        virtual void undo() final
        {
            setGridCells(_oldCells);
        }

        virtual void redo() final
        {
            setGridCells(_newCells);
        }

        void setGridCells(const GridT& cells)
        {
            GridT* grid = this->getGrid();
            Q_ASSERT(grid);
            Q_ASSERT(_position.x >= 0 && _position.x + cells.width() <= grid->width());
            Q_ASSERT(_position.y >= 0 && _position.y + cells.height() <= grid->height());

            auto it = cells.cbegin();
            for (unsigned y = 0; y < cells.height(); y++) {
                for (unsigned x = 0; x < cells.width(); x++) {
                    grid->set(_position.x + x, _position.y + y, *it++);
                }
            }
            Q_ASSERT(it == cells.cend());

            this->emitGridChanged();
        }
    };

private:
    AccessorT* const _accessor;

public:
    GridUndoHelper(AccessorT* accessor)
        : _accessor(accessor)
    {
    }

private:
    inline const ArgsT selectedGridTuple()
    {
        return _accessor->selectedGridTuple();
    }

    inline const GridT* getGrid(const ArgsT& gridArgs)
    {
        auto f = std::mem_fn(&AccessorT::getGrid);
        return mem_fn_call(f, _accessor, gridArgs);
    }

public:
    // will return nullptr if grid could not be accessed or is equal to newGrid
    QUndoCommand* editGridCommand(const GridT& newGrid, const QString& text)
    {
        const ArgsT gridArgs = _accessor->selectedGridTuple();
        const GridT* oldGrid = getGrid(gridArgs);
        if (oldGrid == nullptr) {
            return nullptr;
        }

        if (*oldGrid == newGrid) {
            return nullptr;
        }
        return new EditGridCommand(_accessor, gridArgs, *oldGrid, newGrid, text);
    }

    bool editGrid(const GridT& newGrid, const QString& text)
    {
        QUndoCommand* e = editGridCommand(newGrid, text);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // will return nullptr if grid could not be accessed, has a size equal to newSize or newSize is larger than maxSize
    QUndoCommand* resizeGridCommand(const usize& newSize, const DataT& defaultValue, const QString& text)
    {
        const ArgsT gridArgs = _accessor->selectedGridTuple();
        const GridT* grid = getGrid(gridArgs);
        if (grid == nullptr) {
            return nullptr;
        }

        if (grid->size() == newSize) {
            return nullptr;
        }
        const usize maxSize = _accessor->maxSize();
        if (newSize.width > maxSize.width || newSize.height > maxSize.height) {
            return nullptr;
        }

        const GridT resizedGrid = grid->resized(newSize, defaultValue);

        return new EditGridCommand(_accessor, gridArgs, *grid, std::move(resizedGrid), text);
    }

    bool resizeGrid(const usize& newSize, const DataT& defaultValue, const QString& text)
    {
        QUndoCommand* e = resizeGridCommand(newSize, defaultValue, text);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // will return nullptr if cells could not be accessed or is equal to newCells
    QUndoCommand* editCellsCommand(const upoint& pos, const GridT& newCells,
                                   const QString& text)
    {
        if (newCells.empty()) {
            return nullptr;
        }

        const ArgsT gridArgs = _accessor->selectedGridTuple();
        const GridT* grid = getGrid(gridArgs);
        if (grid == nullptr) {
            return nullptr;
        }
        if (grid->empty()
            || pos.x >= grid->width()
            || pos.y >= grid->height()
            || pos.x + newCells.width() >= grid->width()
            || pos.y + newCells.height() >= grid->height()) {

            return nullptr;
        }
        GridT oldCells = grid->subGrid(pos, newCells.size());

        if (newCells == oldCells) {
            return nullptr;
        }
        return new EditCellsCommand(_accessor, gridArgs, pos, std::move(oldCells), newCells, text);
    }

    bool editCells(const upoint& pos, const GridT& newCells, const QString& text)
    {
        QUndoCommand* e = editCellsCommand(pos, newCells, text);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // will return nullptr if cells could not be accessed or is equal to newCells
    template <typename UnaryFunction>
    QUndoCommand* editCellsWithCroppingAndCellTestCommand(const point& pos, const GridT& newCells,
                                                          const QString& text,
                                                          UnaryFunction validCellTest)
    {
        if (newCells.empty()
            || pos.x < -int(newCells.width())
            || pos.y < -int(newCells.height())) {

            return nullptr;
        }

        const ArgsT gridArgs = _accessor->selectedGridTuple();
        const GridT* grid = getGrid(gridArgs);
        if (grid == nullptr) {
            return nullptr;
        }

        if (grid->empty()
            || pos.x >= int(grid->width())
            || pos.y >= int(grid->height())) {

            return nullptr;
        }

        // crop newCells to fit grid
        const upoint croppedPos(std::max(0, pos.x),
                                std::max(0, pos.y));
        const upoint croppedOffset(croppedPos.x - pos.x,
                                   croppedPos.y - pos.y);
        const usize croppedSize(std::min(newCells.width() - croppedOffset.x, grid->width() - croppedPos.x),
                                std::min(newCells.height() - croppedOffset.y, grid->height() - croppedPos.y));

        GridT croppedCells = newCells.subGrid(croppedOffset, croppedSize);
        GridT oldCells = grid->subGrid(croppedPos, croppedSize);

        Q_ASSERT(croppedCells.empty() == false);
        Q_ASSERT(oldCells.empty() == false);

        // apply the validCellTest function to each of the grid cells
        auto croppedIt = croppedCells.begin();
        auto oldIt = oldCells.cbegin();
        while (croppedIt != croppedCells.end()) {
            bool ok = validCellTest(const_cast<const DataT&>(*croppedIt));
            if (!ok) {
                *croppedIt = *oldIt;
            }

            croppedIt++;
            oldIt++;
        }
        Q_ASSERT(oldIt == oldCells.cend());

        if (croppedCells == oldCells) {
            return nullptr;
        }
        return new EditCellsCommand(_accessor, gridArgs, croppedPos,
                                    std::move(oldCells), std::move(croppedCells), text);
    }

    template <typename UnaryFunction>
    bool editCellsWithCroppingAndCellTest(const point& pos, const GridT& newCells,
                                          const QString& text,
                                          UnaryFunction validCellTest)
    {
        QUndoCommand* e = editCellsWithCroppingAndCellTestCommand(pos, newCells, text, validCellTest);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }
};
}
}
}
