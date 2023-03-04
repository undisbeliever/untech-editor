/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <string>

const unsigned UNTECH_VERSION_INT = 29;
constexpr std::u8string_view UNTECH_VERSION_STRING = u8"Version 0.29";

constexpr std::u8string_view UNTECH_ABOUT_TEXT
    = u8"Copyright (c) 2016 - 2021, Marcus Rowe"
      u8"\n"
      u8"\nPart of the UnTech Editor Suite"
      u8"\nLicensed under The MIT License."
      u8"\nhttps://github.com/undisbeliever/untech-editor/blob/master/LICENSE";

constexpr std::u8string_view THIRD_PARTY_LIBS
    = u8"\nLodePNG"
      u8"\n\thttp://lodev.org/lodepng/"
      u8"\n\tCopyright (c) 2005-2022 Lode Vandevenne"
      u8"\n\tzlib License, https://github.com/lvandeve/lodepng/blob/master/LICENSE"
      u8"\n"
      u8"\nLZ4 Library"
      u8"\n\thttps://lz4.github.io/lz4/"
      u8"\n\tCopyright (c) 2011-2020, Yann Collet"
      u8"\n\tBSD 2-Clause License, https://github.com/lz4/lz4/blob/dev/lib/LICENSE"
      u8"\n";

#ifdef IMGUI_IMPL_SDL_OPENGL
constexpr std::u8string_view THIRD_PARTY_GUI_LIBS
    = u8"\nDear ImGui"
      u8"\n\thttps://github.com/ocornut/imgui"
      u8"\n\tCopyright (c) 2014-2023 Omar Cornut"
      u8"\n\tMIT License, https://github.com/ocornut/imgui/blob/master/LICENSE.txt"
      u8"\n"
      u8"\nimgui-filebrowser"
      u8"\n\thttps://github.com/AirGuanZ/imgui-filebrowser"
      u8"\n\tCopyright (c) 2019-2022 Zhuang Guan"
      u8"\n\tMIT License, https://github.com/AirGuanZ/imgui-filebrowser/blob/master/LICENSE"
      u8"\n"
      u8"\ngl3w"
      u8"\n\thttps://github.com/skaslev/gl3w"
      u8"\n\tPublic Domain, https://github.com/skaslev/gl3w/blob/master/UNLICENSE"
      u8"\n"
      u8"\nglcorearb.h"
      u8"\n\tCopyright 2013-2020 The Khronos Group Inc."
      u8"\n\tMIT License"
      u8"\n";
#endif
