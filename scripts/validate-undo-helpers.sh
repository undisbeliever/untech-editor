#!/usr/bin/env bash

# Find all usages of undohelper or UndoHelper that is:
#  * NOT a friend class declaration
#  * NOT inside the accessor directory
#  * NOT inside a file called undo.cpp
INVALID_UNDO_HELPERS=$(grep -i -r 'UndoHelper' src/ |
                       grep -v -E 'friend class[[:space:]][^[:space:]]+UndoHelper' |
                       grep -v '^src/gui-qt/accessor/' |
                       grep -v '^[^:]*/undo.cpp:' )


if [[ ! -z "$INVALID_UNDO_HELPERS" ]] ; then
    echo 'An UndoHelper class is used outside an undo.cpp file'
    echo '===================================================='
    echo
    echo "$INVALID_UNDO_HELPERS"
    echo
    exit 1
fi

