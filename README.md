UnTech Editor Suite
===================

This repository contains the utilities used to create and edit assets
used by the
[UnTech Game Engine](https://github.com/undisbeliever/untech-engine)

This editor is a work in progress and the data formats may be subject to
changes in the future.

It currently contains 8 apps:

 * `untech-metasprite-gui`: A graphical editor for UnTech MetaSprites
 * `untech-spriteimporter-gui`: A graphical UI for creating a sprite
   sheet importer file.
 * `untech-utsi2utms`: A CLI utility for converting sprite sheets into
   MetaSprites.
 * `untech-msc`: A CLI utility for compiling MetaSprites to an assembly
   include file.
 * `untech-png2tileset`: A CLI utility for converting an indexed png
   image to a SNES tileset/palette combo.
 * `untech-png2snes`: A CLI utility for converting an indexed png image
   to a SNES tileset/tilemap/palette combo.
 * `untech-write-sfc-checksum`: A CLI utility that corrects the internal
   checksum of an unheadered .sfc ROM file.
 * `untech-lz4c`: A LZ4 HC block compressor.\
    (NOTE: untech-lz4c uses a modified block frame to save 2 bytes of
    ROM and is incompatible with the lz4 standard).


License
=======
This repository's code is licensed under [The MIT License](LICENSE).

The code requires some third party libraries. Please see
[THIRDPARTYLIBS.md](THIRDPARTYLIBS.md) for more details.


