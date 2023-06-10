cmake_minimum_required(VERSION 3.18)

# ::TODO figure out how to compile asan, msan, ubsan builds::
# ::TODO figure out how to do a mingw cross compilation in Linux::


# Set default build type to release
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
    message("HO")
endif()


project(untech-editor LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# External dependencies
find_package(SDL2 REQUIRED)
find_package(GSL REQUIRED)


# Compiler Flags
# ==============


# CMake adds `-DNDEBUG` to release builds, undefine it with `/UNDEBUG` or `-UNDEBUG`.
# I want to use `assert()` in the release build.
if (MSVC)
    add_compile_options(/UNDEBUG)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-UNDEBUG)
endif()



option(HARDENING_FLAGS "Add hardening flags to gcc/clang compile/link options" ON)

# Enable compiler hardening options
if (HARDENING_FLAGS)
    # Debug options
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            add_definitions(-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC)
        endif()

        # _FORTIFY_SOURCE requires compiling with optimization
        if (NOT CMAKE_CXX_FLAGS_DEBUG MATCHES "(^ )-O")
            add_compile_options(-Og)
        endif()
    endif()


    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        # Extra compile warnings

        add_compile_options(-Wall -Wextra -Wdeprecated -Wimplicit-fallthrough -Wvla -pedantic)
        add_compile_options(-Wnull-dereference -Wdouble-promotion -Wformat=2)
        # Disable variable-length arrays
        add_compile_options(-Werror=vla)

        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            # Extra compile warnings
            add_compile_options(-Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict)
        endif()


        # Hardening flags from:
        # https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc/

        # Run-time buffer overflow detection and std bound checking
        add_definitions(-D_FORTIFY_SOURCE=2)

        # Run-time bounds checking for C++ strings and containers
        add_definitions(-D_GLIBCXX_ASSERTIONS)

        # Increased reliability of backtraces
        add_compile_options(-fasynchronous-unwind-tables)

        # Enable table-based thread cancellation
        add_compile_options(-fexceptions)

        # Full ASLR for executables
        add_compile_options(-fpie)
        add_link_options(-Wl,-pie)

        # Stack smashing protector
        add_compile_options(-fstack-protector-strong)
        add_link_options(-fstack-protector-strong)

        # Control flow integrity protection
        add_compile_options(-fcf-protection)

        # Increased reliability of stack overflow detection
        add_compile_options(-fstack-clash-protection)

        # Detect and reject underlinking
        add_link_options(-Wl,-z,defs)

        # Disable lazy binding
        add_link_options(-Wl,-z,now)

        # Read-only segments after relocation
        add_link_options(-Wl,-z,relro)
    endif()
endif()



# Modules
# =======

include_directories(src)


add_library(common OBJECT
    src/models/common/base64.cpp
    src/models/common/errorlist.cpp
    src/models/common/file.cpp
    src/models/common/string.cpp
    src/models/common/stringbuilder.cpp
    src/models/common/u8strings.cpp
)

add_library(images OBJECT
    src/models/common/image.cpp
    src/models/common/imagecache.cpp
    src/models/common/indexedimage.cpp
    src/models/common/rgba.cpp
)

add_library(snes OBJECT
    src/models/snes/bit-depth.cpp
    src/models/snes/image2snes.cpp
    src/models/snes/image2tileset.cpp
    src/models/snes/snescolor.cpp
    src/models/snes/tile-data.cpp
    src/models/snes/tile.cpp
    src/models/snes/tilemap.cpp
)

add_library(compiler OBJECT
    src/models/common/xml/xmlreader.cpp
    src/models/common/xml/xmlwriter.cpp

    src/models/entity/entityromdata-serializer.cpp
    src/models/entity/entityromdata.cpp

    src/models/lz4/lz4.cpp

    src/models/metasprite/actionpointfunstions-serializer.cpp
    src/models/metasprite/animation/animation.cpp
    src/models/metasprite/animation/previewstate.cpp
    src/models/metasprite/animation/serializer.cpp
    src/models/metasprite/common.cpp
    src/models/metasprite/compiler/animationcompiler.cpp
    src/models/metasprite/compiler/combinesmalltiles.cpp
    src/models/metasprite/compiler/compiler.cpp
    src/models/metasprite/compiler/framesetcompiler.cpp
    src/models/metasprite/compiler/framesetexportlist.cpp
    src/models/metasprite/compiler/palettecompiler.cpp
    src/models/metasprite/compiler/references.cpp
    src/models/metasprite/compiler/tilesetinserter.cpp
    src/models/metasprite/compiler/tilesetlayout.cpp
    src/models/metasprite/frameset-exportorder-serializer.cpp
    src/models/metasprite/frameset-exportorder.cpp
    src/models/metasprite/framesetfile-serializer.cpp
    src/models/metasprite/framesetfile.cpp
    src/models/metasprite/metasprite-serializer.cpp
    src/models/metasprite/metasprite.cpp
    src/models/metasprite/spriteimporter-serializer.cpp
    src/models/metasprite/spriteimporter.cpp
    src/models/metasprite/utsi2utms/utsi2utms.cpp

    src/models/metatiles/interactive-tiles.cpp
    src/models/metatiles/metatile-tileset.cpp
    src/models/metatiles/metatiles-serializer.cpp

    src/models/project/compiler-status.cpp
    src/models/project/project-compiler.cpp
    src/models/project/project-data.cpp
    src/models/project/project-serializer.cpp
    src/models/project/project.cpp
    src/models/project/resource-compiler.cpp

    src/models/resources/animated-tileset.cpp
    src/models/resources/background-image.cpp
    src/models/resources/drawing.cpp
    src/models/resources/invalid-image-error.cpp
    src/models/resources/palette.cpp
    src/models/resources/resources-serializer.cpp
    src/models/resources/scenes.cpp

    src/models/rooms/room-error.cpp
    src/models/rooms/rooms-serializer.cpp
    src/models/rooms/rooms.cpp

    src/models/scripting/bytecode.cpp
    src/models/scripting/game-state.cpp
    src/models/scripting/script.cpp
    src/models/scripting/scripting-serializer.cpp
)


# CLI apps
# ========

add_executable(untech-compiler src/cli/untech-compiler.cpp)
target_link_libraries(untech-compiler PRIVATE common snes images compiler lodepng lz4)

add_executable(untech-lz4c src/cli/untech-lz4c.cpp src/models/lz4/lz4.cpp)
target_link_libraries(untech-lz4c PRIVATE common lz4)

add_executable(untech-png2snes src/cli/untech-png2snes.cpp)
target_link_libraries(untech-png2snes PRIVATE common snes images lodepng)

add_executable(untech-png2tileset src/cli/untech-png2tileset.cpp)
target_link_libraries(untech-png2tileset PRIVATE common snes images lodepng)

add_executable(untech-write-sfc-checksum src/cli/untech-write-sfc-checksum.cpp src/models/snes/cartridge.cpp)
target_link_libraries(untech-write-sfc-checksum PRIVATE common)


# untech-editor-gui
# =================

set(UNTECH_EDITOR_GUI_DEFINITIONS IMGUI_IMPL_SDL_OPENGL IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND UNTECH_EDITOR_GUI_DEFINITIONS IMGUI_DISABLE_DEBUG_TOOLS)
endif()



add_executable(untech-editor-gui
    src/gui/abstract-editor.cpp
    src/gui/background-thread.cpp
    src/gui/imgui-combos.cpp
    src/gui/imgui-filebrowser.cpp
    src/gui/imgui-u8string.cpp
    src/gui/imgui.cpp
    src/gui/selection.cpp
    src/gui/shaders.cpp
    src/gui/splitter.cpp
    src/gui/style.cpp
    src/gui/texture.cpp
    src/gui/undostack.cpp
    src/gui/untech-editor.cpp

    src/gui/editors/abstract-metasprite-editor.cpp
    src/gui/editors/abstract-metatile-editor.cpp
    src/gui/editors/action-points-editor.cpp
    src/gui/editors/background-image-editor.cpp
    src/gui/editors/bytecode-editor.cpp
    src/gui/editors/entity-rom-data-editor.cpp
    src/gui/editors/frameset-export-order-editor.cpp
    src/gui/editors/game-state-editor.cpp
    src/gui/editors/interactive-tiles-editor.cpp
    src/gui/editors/metasprite-editor.cpp
    src/gui/editors/metatile-tileset-editor.cpp
    src/gui/editors/palette-editor.cpp
    src/gui/editors/project-settings-editor.cpp
    src/gui/editors/room-editor.cpp
    src/gui/editors/room-script-gui.cpp
    src/gui/editors/scenes-editor-editor.cpp
    src/gui/editors/spriteimporter-editor.cpp

    src/gui/graphics/entity-graphics.cpp
    src/gui/graphics/invalid-image-error-graphics.cpp
    src/gui/graphics/invalid-room-tile-graphics.cpp
    src/gui/graphics/tilecollisionimage.cpp

    src/gui/windows/about-popup.cpp
    src/gui/windows/error-list-window.cpp
    src/gui/windows/message-box.cpp
    src/gui/windows/projectlist.cpp
)
target_link_libraries(untech-editor-gui PRIVATE common snes images compiler lz4 lodepng dear_imgui _gui_main)
target_link_libraries(untech-editor-gui PRIVATE dear_imgui_sdl2_opengl3)
target_link_libraries(untech-editor-gui PRIVATE ${SDL2_LIBRARIES})

target_include_directories(untech-editor-gui PRIVATE src/vendor/gl3w/include)

target_compile_definitions(untech-editor-gui PRIVATE ${UNTECH_EDITOR_GUI_DEFINITIONS})




# Hack to ensure the `src/vendor/imgui` directory is only included for `src/gui/main.cpp`.
add_library(_gui_main OBJECT
    src/gui/main.cpp
)
target_include_directories(_gui_main PRIVATE
    src/vendor/imgui
    src/vendor/gl3w/include
    ${SDL2_INCLUDE_DIRS}
)
target_compile_definitions(_gui_main PRIVATE ${UNTECH_EDITOR_GUI_DEFINITIONS})


# Third party libraries
# =====================

add_library(lodepng OBJECT
    src/vendor/lodepng/lodepng.cpp
)

add_library(lz4 OBJECT
    src/vendor/lz4/lib/lz4.c
    src/vendor/lz4/lib/lz4hc.c
)

add_library(dear_imgui OBJECT
    src/vendor/imgui/imgui.cpp
    src/vendor/imgui/imgui_draw.cpp
    src/vendor/imgui/imgui_tables.cpp
    src/vendor/imgui/imgui_widgets.cpp
)
target_include_directories(dear_imgui PRIVATE
    src/vendor/imgui
)
target_compile_definitions(dear_imgui PRIVATE ${UNTECH_EDITOR_GUI_DEFINITIONS})


add_library(dear_imgui_sdl2_opengl3 OBJECT
    src/vendor/imgui/backends/imgui_impl_opengl3.cpp
    src/vendor/imgui/misc/cpp/imgui_stdlib.cpp

    src/vendor/gl3w/src/gl3w.c
    src/vendor/imgui/backends/imgui_impl_sdl2.cpp
)
target_include_directories(dear_imgui_sdl2_opengl3 PRIVATE
    src/vendor/imgui
    src/vendor/gl3w/include
    ${SDL2_INCLUDE_DIRS}
)
target_compile_definitions(dear_imgui_sdl2_opengl3 PRIVATE ${UNTECH_EDITOR_GUI_DEFINITIONS})


# Disable -Werror on third party libraries
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(lodepng PRIVATE -Wno-error)
    target_compile_options(lz4 PRIVATE -Wno-error)
    target_compile_options(dear_imgui PRIVATE -Wno-error)
    target_compile_options(dear_imgui_sdl2_opengl3 PRIVATE -Wno-error)
endif()


