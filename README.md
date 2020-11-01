UnTech Editor Suite
===================

This repository contains the utilities used to create and edit assets
used by the
[UnTech Game Engine](https://github.com/undisbeliever/untech-engine)

This editor is a work in progress and the data formats may be subject to
changes in the future.

It currently contains 6 apps:

 * `untech-editor-gui`: A graphical UI for editing files used by the
    compilers.
 * `untech-compiler`: A CLI utility for compiling an untech-editor Project.
 * `untech-png2tileset`: A CLI utility for converting an indexed png
   image to a SNES tileset/palette combo.
 * `untech-png2snes`: A CLI utility for converting an indexed png image
   to a SNES tileset/tilemap/palette combo.
 * `untech-write-sfc-checksum`: A CLI utility that corrects the internal
   checksum of an unheadered .sfc ROM file.
 * `untech-lz4c`: A LZ4 HC block compressor.\
    (NOTE: untech-lz4c uses a modified block frame to save 2 bytes of
    ROM and is incompatible with the lz4 standard).


NOTICE
======

The UnTech Editor suite is a hobbyist project and I cannot guarantee
that this code is bug free, nor can I guarantee that it won't crash
unexpectedly.

I **seriously recommend** that you use version control and save
regularly if you use this program.

<br/>
<br/>

The UnTech Editor suite is not intended to be installed on your PATH.
It will create an `untech-editor-gui.ini` file in the same directory
as the `untech-editor-gui` executable.



Build Requirements
==================

To build the UnTech Editor the following must be installed and available
in your $PATH:

 * GNU Make
 * A C++17 compiler (g++ or clang++)
 * Qt 5.8 or higher


License
=======
This repository's code is licensed under [The MIT License](LICENSE).

The code requires some third party libraries. Please see
[THIRDPARTYLIBS.md](THIRDPARTYLIBS.md) for more details.


