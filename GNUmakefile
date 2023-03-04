# UnTech Editor Makefile

PROFILE ?= release


CXXFLAGS      := -std=c++20
CFLAGS        := -std=c11
LDFLAGS       := -std=c++20

# Used with C and C++ compilers
COMPILE_FLAGS := -Isrc



ifeq ($(OS),Windows_NT)
  BIN_EXT         := .exe

  RM_COMMAND       = -del /f $(subst /,\,$1)
  # Prevents text spam
  _MISSING_DIRS    = $(foreach p,$1,$(if $(wildcard $p),,$p))
  MKDIR_P_COMMAND  = $(if $(call _MISSING_DIRS,$1),-mkdir $(subst /,\,$(call _MISSING_DIRS,$1)))

  LIBS := -lshlwapi

else
  # Linux/BSD

  BIN_EXT         :=

  RM_COMMAND       = rm -f $1
  MKDIR_P_COMMAND  = mkdir -p $1

  LIBS :=
endif



ifeq ($(PROFILE),release)
  # Release build
  COMPILE_FLAGS += -O2 -fdata-sections -ffunction-sections
  LDFLAGS       += -O2 -Wl,-gc-sections

  # Do not use split DWARF on release profile as it increases the build time
  NO_SPLIT_DWARF := 1

else ifeq ($(PROFILE),debug)
  # Debug build
  COMPILE_FLAGS += -g -Og -Werror -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
  LDFLAGS       += -g -Og -Werror

else ifeq ($(PROFILE),asan)
  # Address sanitiser

  # Reccomended environment to run asan binaries with
  #  ASAN_OPTIONS=detect_leaks=1:check_initialization_order=1:detect_leaks=1:atexit=1

  ASAN_FLAGS    := -fsanitize=address,undefined -g -fno-omit-frame-pointer

  COMPILE_FLAGS += $(ASAN_FLAGS) -O1
  LDFLAGS       += $(ASAN_FLAGS) -O1 -Wl,-gc-sections

else ifeq ($(PROFILE),msan)
  # Memory sanitiser

  # Reccomended environment to run asan binaries with
  #  MSAN_OPTIONS=poison_in_dtor=1:detect_leaks=1:atexit=1

  # only available on clang
  COMPILER := clang

  MSAN_FLAGS    := -fsanitize=memory,undefined -fsanitize-memory-track-origins -fsanitize-memory-use-after-dtor -g -fno-omit-frame-pointer

  COMPILE_FLAGS += $(MSAN_FLAGS) -O1
  LDFLAGS       += $(MSAN_FLAGS) -O1 -Wl,-gc-sections

else ifeq ($(PROFILE),ubsan)
  # Undefined Behaviour Sanitizer

  # only available on clang
  COMPILER := clang

  UBSAN_FLAGS    := -fsanitize=undefined,integer,nullability -g -fno-omit-frame-pointer

  COMPILE_FLAGS += $(UBSAN_FLAGS) -O1 -Isrc -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
  LDFLAGS       += $(UBSAN_FLAGS) -O1 -Wl,-gc-sections

else
  $(error unknown profile)
endif



# Compiler Warnings (not used when compiling `src/vendor/` files)
WARNING_FLAGS += -Wall -Wextra -Wdeprecated -Wimplicit-fallthrough -Wvla -pedantic
WARNING_FLAGS += -Wnull-dereference -Wdouble-promotion -Wformat=2
# Disable variable-length arrays
WARNING_FLAGS += -Werror=vla


# Redhat recommended compiler and linker flags for GCC
# https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc/
# Run-time buffer overflow detection and std bound checking
COMPILE_FLAGS	+= -D_FORTIFY_SOURCE=2
# Run-time bounds checking for C++ strings and containers
COMPILE_FLAGS += -D_GLIBCXX_ASSERTIONS
# Increased reliability of backtraces
COMPILE_FLAGS += -fasynchronous-unwind-tables
# Enable table-based thread cancellation
COMPILE_FLAGS += -fexceptions
# Full ASLR for executables
COMPILE_FLAGS += -fpic -fpie
# Stack smashing protector
COMPILE_FLAGS += -fstack-protector-strong
# Avoid temporary files, speeding up builds
COMPILE_FLAGS	+= -pipe
# Control flow integrity protection
COMPILE_FLAGS += -fcf-protection
# Increased reliability of stack overflow detection
COMPILE_FLAGS += -fstack-clash-protection

LDFLAGS += -fstack-protector


# Disable deprecated warnings on vendor code
VENDOR_CXXFLAGS := -Wno-deprecated



ifdef COMPILER
  # Ensure CXX/CC variables are overridden
  undefine CXX
  undefine CC
else
  ifdef CXX
    ifneq ($(findstring mingw,$(CXX) $(CC)),)
      COMPILER    := mingw
    else ifneq ($(findstring clang,$(CXX) $(CC)),)
      COMPILER  := clang
    else ifneq ($(findstring g++,$(CXX) $(CC)),)
      COMPILER    := gcc
    else
      $(error unknown CXX compiler)
    endif
  else
     $(error missing CXX variable)
  endif
endif



ifeq ($(COMPILER), gcc)
  CXX   ?= g++
  CC    ?= gcc

  # Extra compiler warnings
  WARNING_FLAGS += -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict

else ifeq ($(COMPILER), clang)
  CXX   ?= clang++
  CC    ?= clang

else ifeq ($(COMPILER), mingw)
  CXX   ?= x86_64-w64-mingw32-g++
  CC	?= x86_64-w64-mingw32-gcc

  # Split drawf causes a 'not supported on this system' error when running the binaries in wine
  NO_SPLIT_DWARF  := 1

  OS	          := Windows_NT
  BIN_EXT	  := .exe

  LIBS            += -lshlwapi

  # Location of the `x86_64-w64-mingw32` directory inside the SDL 2 (MinGW 32/64-bit) folder
  # This variable cannot contain spaces
  SDL_DIR         ?= $(HOME)/.local/x86_64-w64-mingw32

  ifneq (1, $(words $(SDL_DIR)))
    $(error 'SDL_DIR cannot contain a space')
  endif

  # `-fstack-clash-protection` causes an "internal compiler error" on my system
  COMPILE_FLAGS := $(filter-out -fstack-clash-protection,$(COMPILE_FLAGS))

  # `-Wnull-dereference` causes lots of "potential null pointer dereference" warnings in `stl_vector.h`
  WARNING_FLAGS := $(filter-out -Wnull-dereference,$(WARNING_FLAGS))

else
  $(error unknown compiler)
endif



ifndef NO_SPLIT_DWARF
  CXXFLAGS      += -gsplit-dwarf
  CFLAGS        += -gsplit-dwarf
  LDFLAGS       += -gsplit-dwarf
endif



ifeq ($(OS),Windows_NT)
  # Enable DEP and ASLR
  LDFLAGS     += -Wl,--nxcompat -Wl,--dynamicbase
else
  # Linux/BSD

  # Enable ASLR for executables
  LDFLAGS     += -Wl,-pie

  # Detect and reject underlinking
  # Disable lazy binding
  # Read-only segments after relocation
  LDFLAGS     += -Wl,-z,defs -Wl,-z,now -Wl,-z,relro
endif



CXXFLAGS += $(COMPILE_FLAGS)
CFLAGS   += $(COMPILE_FLAGS)




OBJ_DIR := obj/$(PROFILE)-$(COMPILER)
ifeq ($(PROFILE), release)
  ifneq ($(COMPILER), mingw)
    BIN_DIR := bin
  else
    BIN_DIR := bin/$(PROFILE)-$(COMPILER)
  endif
else
  BIN_DIR := bin/$(PROFILE)-$(COMPILER)
endif



SRCS            := $(wildcard src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp src/*/*/*/*/*.cpp)
OBJS            := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

CLI_SRC         := $(wildcard src/cli/*.cpp)
CLI_APPS        := $(patsubst src/cli/%.cpp,$(BIN_DIR)/%$(BIN_EXT),$(CLI_SRC))

TEST_SRC        := $(wildcard src/test-utils/*.cpp)
TEST_UTILS      := $(patsubst src/test-utils/%.cpp,$(BIN_DIR)/test-utils/%$(BIN_EXT),$(TEST_SRC))

GUI_SRC         := $(filter src/models/% src/gui/%, $(SRCS))
GUI_OBJS        := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(GUI_SRC))
GUI_APP         := $(BIN_DIR)/untech-editor-gui$(BIN_EXT)


# Third party libs
THIRD_PARTY_LODEPNG := $(OBJ_DIR)/vendor/lodepng/lodepng.o
THIRD_PARTY_LZ4     := $(OBJ_DIR)/vendor/lz4/lib/lz4.o $(OBJ_DIR)/vendor/lz4/lib/lz4hc.o

THIRD_PARTY_OBJS := $(THIRD_PARTY_LODEPNG) $(THIRD_PARTY_LZ4)


THIRD_PARTY_IMGUI_OBJS := $(addprefix $(OBJ_DIR)/vendor/imgui/, imgui.o imgui_draw.o imgui_tables.o imgui_widgets.o imgui_stdlib.o)

THIRD_PARTY_IMGUI_IMPL_OBJS := $(addprefix $(OBJ_DIR)/vendor/imgui/, gl3w.o imgui_impl_sdl2.o)
IMGUI_CXXFLAGS              := -DIMGUI_IMPL_SDL_OPENGL -Isrc/vendor/gl3w/include
IMGUI_IMPL_CXXFLAGS         := -Isrc/vendor/imgui -Wno-deprecated-enum-enum-conversion

# Required to compile editor GUI in ubuntu. On ubuntu `sdl2-config --libs` does output `-pthread`
IMGUI_LDFLAGS               := -pthread

# Disable to-be-obsoleted Dear ImGui symbols
IMGUI_CXXFLAGS += -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS

# Only show the Dear ImGui Metrics/Debugger window on debug (non-release) builds
ifeq ($(PROFILE),release)
  IMGUI_CXXFLAGS += -DIMGUI_DISABLE_DEBUG_TOOLS
endif


$(OBJ_DIR)/gui/main.o: IMGUI_CXXFLAGS += -Isrc/vendor/imgui


UNAME_S := $(shell uname -s)

ifeq ($(COMPILER), mingw) # Cross compiling windows
  IMGUI_LDFLAGS   += -lgdi32 -lopengl32 -limm32
  IMGUI_LDFLAGS   += $(shell PKG_CONFIG_LIBDIR='$(SDL_DIR)/lib/pkgconfig' pkg-config --define-prefix --libs sdl2)
  IMGUI_CXXFLAGS  += $(shell PKG_CONFIG_LIBDIR='$(SDL_DIR)/lib/pkgconfig' pkg-config --define-prefix --cflags sdl2)

  ifneq ($(.SHELLSTATUS), 0)
    $(error Cannot find the sdl2 library, please set the `SDL_DIR` variable)
  endif

else ifeq ($(UNAME_S), Linux) #LINUX
  IMGUI_LDFLAGS   += -lGL -ldl $(shell sdl2-config --libs)
  IMGUI_CXXFLAGS  += $(shell sdl2-config --cflags)

else ifeq ($(UNAME_S), Darwin) #APPLE
  IMGUI_LDFLAGS   += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo $(shell sdl2-config --libs) -L/usr/local/lib -L/opt/local/lib
  IMGUI_CXXFLAGS  += $(shell sdl2-config --cflags) -I/usr/local/include -I/opt/local/include

else ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
  IMGUI_LDFLAGS   += -lgdi32 -lopengl32 -limm32 $(shell pkg-config --libs sdl2)
  IMGUI_CXXFLAGS  += $(shell pkg-config --cflags sdl2)

else
  $(error Unknown system)
endif



.PHONY: all
all: cli test-utils gui

.PHONY: cli
cli: dirs $(CLI_APPS)

.PHONY: test-utils
test-utils: dirs $(TEST_UTILS)

.PHONY: gui
gui: dirs $(GUI_APP)


PERCENT = %
define cli-modules
$(BIN_DIR)/$(strip $1)$(BIN_EXT): \
  $(filter $(patsubst %,$(OBJ_DIR)/models/%/$(PERCENT),$2), $(OBJS)) \
  $(filter $(OBJ_DIR)/cli/helpers/%, $(OBJS)) \
  $(THIRD_PARTY_LODEPNG) \
  $(if $(filter lz4,$2), $(THIRD_PARTY_LZ4))
endef

define test-util-modules
$(call cli-modules, test-utils/$(strip $1), $(strip $2))
endef


# Select the modules used by the apps
$(call cli-modules, untech-compiler,            common snes project entity resources metasprite metatiles rooms scripting lz4)
$(call cli-modules, untech-lz4c,                common lz4)
$(call cli-modules, untech-png2tileset,         common snes)
$(call cli-modules, untech-png2snes,            common snes)
$(call cli-modules, untech-write-sfc-checksum,  common snes)

$(call test-util-modules, serializer-test,      common snes project entity resources metasprite metatiles rooms scripting lz4)

$(GUI_APP): $(GUI_OBJS) $(THIRD_PARTY_OBJS) $(THIRD_PARTY_IMGUI_OBJS) $(THIRD_PARTY_IMGUI_IMPL_OBJS)



# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules



# Add automatic dependency generation to the Makefile
# The `-MP` prevents a 'No rule to make target' make error when a header file is deleted.
CFLAGS      := -MMD -MP $(CFLAGS)
CXXFLAGS    := -MMD -MP $(CXXFLAGS)

DEPS := $(OBJS:.o=.d)
DEPS += $(THIRD_PARTY_OBJS:.o=.d)
-include $(DEPS)



$(TEST_UTILS): $(BIN_DIR)/test-utils/%$(BIN_EXT): $(OBJ_DIR)/test-utils/%.o
	$(CXX) $(LDFLAGS) $(WARNING_FLAGS) -o $@ $^ $(LIBS)

$(CLI_APPS): $(BIN_DIR)/%$(BIN_EXT): $(OBJ_DIR)/cli/%.o
	$(CXX) $(LDFLAGS) $(WARNING_FLAGS) -o $@ $^ $(LIBS)

$(GUI_APP):
	$(CXX) $(LDFLAGS) $(WARNING_FLAGS) -o $@ $^ $(GUI_LIBS) $(LIBS) $(IMGUI_LDFLAGS)


$(OBJ_DIR)/vendor/%.o: src/vendor/%.cpp
	$(CXX) $(CXXFLAGS) $(VENDOR_CXXFLAGS) $(WARNING_FLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/%.o: src/vendor/%.c
	$(CC) $(CFLAGS) $(WARNING_FLAGS) -c -o $@ $<



$(OBJ_DIR)/gui/%.o: src/gui/%.cpp
	$(CXX) $(CXXFLAGS) $(IMGUI_CXXFLAGS) $(WARNING_FLAGS) -c -o $@ $<


$(OBJ_DIR)/vendor/imgui/%.o: src/vendor/imgui/%.cpp
	$(CXX) $(CXXFLAGS) $(IMGUI_CXXFLAGS) $(IMGUI_IMPL_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/imgui/%.o: src/vendor/imgui/backends/%.cpp
	$(CXX) $(CXXFLAGS) $(IMGUI_CXXFLAGS) $(IMGUI_IMPL_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/imgui/%.o: src/vendor/imgui/misc/cpp/%.cpp
	$(CXX) $(CXXFLAGS) $(IMGUI_CXXFLAGS) $(IMGUI_IMPL_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/imgui/gl3w.o: src/vendor/gl3w/src/gl3w.c
	$(CC) $(CFLAGS) -Isrc/vendor/gl3w/include -c -o $@ $<


$(OBJ_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(WARNING_FLAGS) -c -o $@ $<



.PHONY: dirs
OBJECT_DIRS := $(sort $(dir $(OBJS) $(THIRD_PARTY_OBJS)))
dirs: $(BIN_DIR)/ $(BIN_DIR)/test-utils/ $(OBJECT_DIRS)
$(BIN_DIR)/ $(BIN_DIR)/test-utils/ $(OBJECT_DIRS):
	$(call MKDIR_P_COMMAND,$@)



.PHONY: clean
clean:
	$(call RM_COMMAND, $(DEPS))
	$(call RM_COMMAND, $(OBJS))
	$(call RM_COMMAND, $(THIRD_PARTY_OBJS))


