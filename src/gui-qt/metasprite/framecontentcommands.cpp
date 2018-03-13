/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentcommands.h"
#include "metasprite/document.h"
#include "spriteimporter/document.h"

#include <QCoreApplication>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {

// RaiseLowerFrameContents
// =======================

template <class DT>
RaiseLowerFrameContents<DT>::RaiseLowerFrameContents(DocumentT* document, FrameT* frame,
                                                     const QString& text)
    : QUndoCommand(text)
    , _document(document)
    , _frame(frame)
{
}

template <class DT>
void RaiseLowerFrameContents<DT>::raiseItems(const std::set<SelectedItem>& items)
{
    bool frameObjectListChanged = false;
    bool actionPointListChanged = false;
    bool entityHitboxListChanged = false;

    for (const auto& item : items) {
        if (item.index == 0) {
            continue;
        }

        auto moveItem = [&](auto& list) {
            std::swap(list.at(item.index),
                      list.at(item.index - 1));
        };

        switch (item.type) {
        case SelectedItem::FRAME_OBJECT:
            frameObjectListChanged = true;
            moveItem(_frame->objects);
            break;

        case SelectedItem::ACTION_POINT:
            actionPointListChanged = true;
            moveItem(_frame->actionPoints);
            break;

        case SelectedItem::ENTITY_HITBOX:
            entityHitboxListChanged = true;
            moveItem(_frame->entityHitboxes);
            break;

        default:
            break;
        }
    }

    emit _document->frameContentsMoved(_frame, items, -1);

    if (frameObjectListChanged) {
        emit _document->frameObjectListChanged(_frame);
    }
    if (actionPointListChanged) {
        emit _document->actionPointListChanged(_frame);
    }
    if (entityHitboxListChanged) {
        emit _document->entityHitboxListChanged(_frame);
    }
}

template <class DT>
void RaiseLowerFrameContents<DT>::lowerItems(const std::set<SelectedItem>& items)
{
    bool frameObjectListChanged = false;
    bool actionPointListChanged = false;
    bool entityHitboxListChanged = false;

    for (auto it = items.rbegin(); it != items.rend(); it++) {
        const auto& item = *it;

        auto fn = [&](auto& list) {
            if (item.index + 1 < list.size()) {
                std::swap(list.at(item.index),
                          list.at(item.index + 1));
            }
        };

        switch (item.type) {
        case SelectedItem::FRAME_OBJECT:
            frameObjectListChanged = true;
            fn(_frame->objects);
            break;

        case SelectedItem::ACTION_POINT:
            actionPointListChanged = true;
            fn(_frame->actionPoints);
            break;

        case SelectedItem::ENTITY_HITBOX:
            entityHitboxListChanged = true;
            fn(_frame->entityHitboxes);
            break;

        default:
            break;
        }
    }

    emit _document->frameContentsMoved(_frame, items, 1);

    if (frameObjectListChanged) {
        emit _document->frameObjectListChanged(_frame);
    }
    if (actionPointListChanged) {
        emit _document->actionPointListChanged(_frame);
    }
    if (entityHitboxListChanged) {
        emit _document->entityHitboxListChanged(_frame);
    }
}

// RaiseFrameContents
// ==================

template <class DT>
RaiseFrameContents<DT>::RaiseFrameContents(DocumentT* document, FrameT* frame,
                                           const std::set<SelectedItem>& items)
    : RaiseLowerFrameContents<DT>(document, frame, QCoreApplication::tr("Raise"))
    , _undoItems(moveSelectedItems(items, -1))
    , _redoItems(items)
{
}

template <class DT>
void RaiseFrameContents<DT>::undo()
{
    this->lowerItems(_undoItems);
}

template <class DT>
void RaiseFrameContents<DT>::redo()
{
    this->raiseItems(_redoItems);
}

// LowerFrameContents
// ==================

template <class DT>
LowerFrameContents<DT>::LowerFrameContents(DocumentT* document, FrameT* frame,
                                           const std::set<SelectedItem>& items)
    : RaiseLowerFrameContents<DT>(document, frame, QCoreApplication::tr("Lower"))
    , _undoItems(moveSelectedItems(items, 1))
    , _redoItems(items)
{
}

template <class DT>
void LowerFrameContents<DT>::undo()
{
    this->raiseItems(_undoItems);
}

template <class DT>
void LowerFrameContents<DT>::redo()
{
    this->lowerItems(_redoItems);
}

template class RaiseLowerFrameContents<SpriteImporter::Document>;
template class RaiseFrameContents<SpriteImporter::Document>;
template class LowerFrameContents<SpriteImporter::Document>;

template class RaiseLowerFrameContents<MetaSprite::Document>;
template class RaiseFrameContents<MetaSprite::Document>;
template class LowerFrameContents<MetaSprite::Document>;
}
}
}
