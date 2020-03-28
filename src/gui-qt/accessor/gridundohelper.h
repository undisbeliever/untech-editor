/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/common/optional.h"
#include "models/common/vectorset-upoint.h"
#include <QCoreApplication>
#include <QUndoCommand>
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

    private:
        constexpr inline auto function_args() const
        {
            return std::tuple_cat(std::make_tuple(_accessor), _args);
        }

    protected:
        inline const GridT* grid() const
        {
            return std::apply(&AccessorT::getGrid, function_args());
        }

        inline GridT* getGrid()
        {
            return std::apply(&AccessorT::getGrid, function_args());
        }

        inline void emitGridChanged()
        {
            return std::apply(&AccessorT::gridChanged, function_args());

            _accessor->resourceItem()->dataChanged();
        }

        // When this signal is emitted you MUST close all editors
        // accessing the list to prevent data corruption
        inline void emitGridAboutToBeResized()
        {
            return std::apply(&AccessorT::gridAboutToBeResized, function_args());
        }

        inline void emitGridResized()
        {
            return std::apply(&AccessorT::gridResized, function_args());
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

    class EditMultipleCellsCommand : public BaseCommand {
    public:
        struct ModifiedCell {
            size_t pos;
            DataT oldValue;
            DataT newValue;
        };

    private:
        QVector<ModifiedCell> _cells;
        const bool _first;

    public:
        EditMultipleCellsCommand(AccessorT* accessor, const ArgsT& args,
                                 QVector<ModifiedCell>&& cells,
                                 const QString& text, const bool first)
            : BaseCommand(accessor, args, text)
            , _cells(std::move(cells))
            , _first(first)
        {
        }
        ~EditMultipleCellsCommand() = default;

        virtual int id() const final
        {
            return 0x47454d43; // GEMC
        }

        virtual void undo() final
        {
            GridT* grid = this->getGrid();
            Q_ASSERT(grid);
            const size_t cellCount = grid->cellCount();

            for (const auto& mc : _cells) {
                Q_ASSERT(mc.pos < cellCount);
                *(grid->begin() + mc.pos) = mc.oldValue;
            }

            this->emitGridChanged();
        }

        virtual void redo() final
        {
            GridT* grid = this->getGrid();
            Q_ASSERT(grid);
            const size_t cellCount = grid->cellCount();

            for (const auto& mc : _cells) {
                Q_ASSERT(mc.pos < cellCount);
                *(grid->begin() + mc.pos) = mc.newValue;
            }

            this->emitGridChanged();
        }

        virtual bool mergeWith(const QUndoCommand* cmd) final
        {
            const auto* command = dynamic_cast<const EditMultipleCellsCommand*>(cmd);

            if (command
                && command->_first == false
                && command->_accessor == this->_accessor
                && command->_args == this->_args
                && command->grid() == this->grid()) {

                for (const ModifiedCell& commandCell : command->_cells) {
                    auto it = std::find_if(this->_cells.begin(), this->_cells.end(),
                                           [&](const auto& i) { return i.pos == commandCell.pos; });
                    if (it != this->_cells.end()) {
                        it->newValue = commandCell.newValue;
                    }
                    else {
                        this->_cells.append(commandCell);
                    }
                }
                return true;
            }

            return false;
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
        return std::apply(&AccessorT::getGrid,
                          std::tuple_cat(std::make_tuple(_accessor), gridArgs));
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

    // will return nullptr if old cells could not be accessed or is equal to newCells
    template <typename NCT, typename UnaryFunction>
    QUndoCommand* editCellsMergeWithCroppingAndCellTestCommand(const point& pos, const UnTech::grid<NCT>& cellsToInsert,
                                                               const QString& text, const bool first,
                                                               UnaryFunction validCellTest)
    {
        using ModifiedCell = typename EditMultipleCellsCommand::ModifiedCell;

        if (cellsToInsert.empty()
            || pos.x < -int(cellsToInsert.width())
            || pos.y < -int(cellsToInsert.height())) {

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

        const upoint gridPos(std::max(0, pos.x),
                             std::max(0, pos.y));
        const upoint ctiOffset(gridPos.x - pos.x,
                               gridPos.y - pos.y);
        const usize ctiSize(std::min(cellsToInsert.width() - ctiOffset.x, grid->width() - gridPos.x),
                            std::min(cellsToInsert.height() - ctiOffset.y, grid->height() - gridPos.y));

        // apply the validCellTest function to each of the new grid cells

        QVector<ModifiedCell> cells;
        cells.reserve(ctiSize.width * ctiSize.height);

        for (unsigned y = 0; y < ctiSize.height; y++) {
            for (unsigned x = 0; x < ctiSize.width; x++) {
                const DataT& oldCell = grid->at(gridPos.x + x, gridPos.y + y);
                const NCT& cti = cellsToInsert.at(ctiOffset.x + x, ctiOffset.y + y);

                const optional<DataT> nc = validCellTest(cti);
                if (nc) {
                    cells.append(
                        ModifiedCell{ grid->cellPos(gridPos.x + x, gridPos.y + y),
                                      oldCell,
                                      nc.value() });
                }
            }
        }

        if (cells.empty()) {
            return nullptr;
        }
        return new EditMultipleCellsCommand(_accessor, gridArgs, std::move(cells), text, first);
    }

    template <typename NCT, typename UnaryFunction>
    bool editCellsMergeWithCroppingAndCellTest(const point& pos, const UnTech::grid<NCT>& newCells,
                                               const QString& text, const bool first,
                                               UnaryFunction validCellTest)
    {
        QUndoCommand* e = editCellsMergeWithCroppingAndCellTestCommand(pos, newCells, text, first, validCellTest);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }
};
}
}
}
