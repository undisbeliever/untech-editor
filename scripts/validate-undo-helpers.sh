#!/usr/bin/env bash

# Find all usages of undohelper or UndoHelper that is:
#  * NOT inside the accessor directory
#  * NOT inside a file named resourceitem, accessor, accessors or document
INVALID_UNDO_HELPERS=$(grep -i -r 'UndoHelper' src/ |
                       grep -v '^src/gui-qt/accessor/' |
                       grep -v -E '^[^:]+resourceitem.(cpp|h):' |
                       grep -v -E '^[^:]+accessors?.(cpp|h):' |
                       grep -v -E '^[^:]+metasprite/[^:]+/document.(cpp|h):' )


if [[ ! -z "$INVALID_UNDO_HELPERS" ]] ; then
    echo 'An UndoHelper class is used outside an undo.cpp file'
    echo '===================================================='
    echo
    echo "$INVALID_UNDO_HELPERS"
    echo
    exit 1
fi

