
PROFILE     ?= release
CXX         ?= g++
CC          ?= gcc

CXXFLAGS    += -std=c++17

ifeq ($(OS),Windows_NT)
  BIN_EXT         := .exe
  RM_COMMAND       = -del /f $(subst /,\,$1)
  # Prevents text spam
  _MISSING_DIRS    = $(foreach p,$1,$(if $(wildcard $p),,$p))
  MKDIR_P_COMMAND  = $(if $(call _MISSING_DIRS,$1),-mkdir $(subst /,\,$(call _MISSING_DIRS,$1)))

  LIBS := -lshlwapi

  VENDOR_CXXFLAGS := -Wno-deprecated

else
  # Linux/BSD

  BIN_EXT         :=
  RM_COMMAND       = rm -f $1
  MKDIR_P_COMMAND  = mkdir -p $1

  LIBS :=

  VENDOR_CXXFLAGS := -Wno-deprecated
endif



ifeq ($(PROFILE),release)
  OBJ_DIR       := obj/release
  BIN_DIR       := bin

  CXXFLAGS      += -O2 -flto -fdata-sections -ffunction-sections -MMD -Isrc
  CFLAGS        += -O2 -flto -fdata-sections -ffunction-sections -MMD -Isrc
  LDFLAGS       += -O2 -flto -Wl,-gc-sections

  # Do not use split DWARF on release profile as it increases the build time
  NO_SPLIT_DWARF := 1

else ifeq ($(PROFILE),debug)
  OBJ_DIR       := obj/debug-$(firstword $(CXX))
  BIN_DIR       := bin/debug-$(firstword $(CXX))

  CXXFLAGS      += -g -Og -MMD -Isrc -Werror -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
  CFLAGS        += -g -Og -MMD -Isrc -Werror
  LDFLAGS       += -g -Og -Werror

else ifeq ($(PROFILE),asan)
  # Address sanitiser

  # Reccomended environment to run asan binaries with
  #  ASAN_OPTIONS=detect_leaks=1:check_initialization_order=1:detect_leaks=1:atexit=1

  OBJ_DIR       := obj/asan-$(firstword $(CXX))
  BIN_DIR       := bin/asan-$(firstword $(CXX))

  ASAN_FLAGS    := -fsanitize=address,undefined -g -fno-omit-frame-pointer

  CXXFLAGS      += $(ASAN_FLAGS) -O1 -MMD -Isrc
  CFLAGS        += $(ASAN_FLAGS) -O1 -MMD -Isrc
  LDFLAGS       += $(ASAN_FLAGS) -O1 -Wl,-gc-sections

else ifeq ($(PROFILE),msan)
  # Memory sanitiser

  # Reccomended environment to run asan binaries with
  #  MSAN_OPTIONS=poison_in_dtor=1:detect_leaks=1:atexit=1

  # only available on clang
  CXX           := clang++
  CC            := clang

  OBJ_DIR       := obj/msan
  BIN_DIR       := bin/msan

  MSAN_FLAGS    := -fsanitize=memory,undefined -fsanitize-memory-track-origins -fsanitize-memory-use-after-dtor -g -fno-omit-frame-pointer

  CXXFLAGS      += $(MSAN_FLAGS) -O1 -MMD -Isrc
  CFLAGS        += $(MSAN_FLAGS) -O1 -MMD -Isrc
  LDFLAGS       += $(MSAN_FLAGS) -O1 -Wl,-gc-sections

else ifeq ($(PROFILE),ubsan)
  # Undefined Behaviour Sanitizer

  # only available on clang
  CXX           := clang++
  CC            := clang

  OBJ_DIR       := obj/ubsan
  BIN_DIR       := bin/ubsan

  UBSAN_FLAGS    := -fsanitize=undefined,integer,nullability -g -fno-omit-frame-pointer

  CXXFLAGS      += $(UBSAN_FLAGS) -O1 -MMD -Isrc -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
  CFLAGS        += $(UBSAN_FLAGS) -O1 -MMD -Isrc
  LDFLAGS       += $(UBSAN_FLAGS) -O1 -Wl,-gc-sections

else ifeq ($(PROFILE),mingw)
  # MinGW cross platform compiling
  CXX_MINGW     ?= x86_64-w64-mingw32-g++
  PREFIX	?= $(HOME)/.local/x86_64-w64-mingw32

  OBJ_DIR       := obj/mingw
  BIN_DIR       := bin/mingw
  BIN_EXT	:= .exe

  CXX           := $(CXX_MINGW)
  CXXFLAGS      += -O2 -flto -MMD -Isrc
  CFLAGS        += -O2 -flto -MMD -Isrc
  LDFLAGS       += -O2 -flto
  LIBS          += -lshlwapi

else
  $(error unknown profile)
endif


ifndef NO_SPLIT_DWARF
  CXXFLAGS      += -gsplit-dwarf
  CFLAGS        += -gsplit-dwarf
  LDFLAGS       += -gsplit-dwarf
endif


ifndef NO_PROTECTIONS
  PROTECTIONS :=

  # Redhat recommended compiler and linker flags for GCC
  # https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc/

  ifneq ($(or $(findstring -O2,$(CXXFLAGS)), $(findstring -O3,$(CXXFLAGS))),)
    # Run-time buffer overflow detection and std bound checking
    PROTECTIONS += -D_FORTIFY_SOURCE=2
  endif

  # Run-time bounds checking for C++ strings and containers
  PROTECTIONS   += -D_GLIBCXX_ASSERTIONS

  # Increased reliability of backtraces
  PROTECTIONS   += -fasynchronous-unwind-tables
  # Enable table-based thread cancellation
  PROTECTIONS   += -fexceptions
  # Full ASLR for executables
  PROTECTIONS   += -fpic -fpie
  # Stack smashing protector
  PROTECTIONS   += -fstack-protector-strong

  # Avoid temporary files, speeding up builds
  PROTECTIONS   += -pipe

  ifeq ($(OS),Windows_NT)
    # Enable DEP and ASLR
    LDFLAGS     += -Wl,--nxcompat -Wl,--dynamicbase
  else
    # Linux/BSD

    # Enable ASLR for executables
    LDFLAGS       += -Wl,-pie

    # Detect and reject underlinking
    # Disable lazy binding
    # Read-only segments after relocation
    LDFLAGS     += -Wl,-z,defs -Wl,-z,now -Wl,-z,relro
  endif

  ifneq ($(findstring g++,$(CXX)),)
    GCC_MAJOR := $(firstword $(subst ., ,$(shell $(CXX) -dumpversion)))
    ifeq ($(GCC_MAJOR),8)
      # Increased reliability of stack overflow detection
      PROTECTIONS += -fstack-clash-protection

      ## Control flow integrity protection
      #PROTECTIONS += -mcet -fcf-protection
      #
      # Skipped: My build of gcc 8.1.0 does not support this yet (-mcet is unrecognized)
    endif
  endif

  CXXFLAGS  += $(PROTECTIONS)
  CFLAGS    += $(PROTECTIONS)
  LDFLAGS   += $(PROTECTIONS)
endif


ifneq ($(findstring clang,$(CXX) $(CC)),)
  # LTO on clang causes "signal not found" errors in Qt
  CXXFLAGS := $(filter-out -flto,$(CXXFLAGS))
  CFLAGS   := $(filter-out -flto,$(CFLAGS))
  LDFLAGS  := $(filter-out -flto,$(LDFLAGS))
endif


ifeq ($(CXXWARNINGS),)
  CXXWARNINGS := -Wall -Wextra -Wdeprecated -Wvla

  ifneq ($(findstring g++,$(CXX)),)
    GCC_MAJOR := $(firstword $(subst ., ,$(shell $(CXX) -dumpversion)))

    ifeq ($(GCC_MAJOR),4)
        CXXWARNINGS += -Wno-missing-field-initializers
    else ifeq ($(GCC_MAJOR),5)
       CXXWARNINGS += -Wlogical-op -Wdouble-promotion -Wformat=2
    else ifeq ($(GCC_MAJOR),6)
       CXXWARNINGS += -Wduplicated-cond -Wlogical-op -Wnull-dereference -Wdouble-promotion -Wformat=2
    else ifeq ($(GCC_MAJOR),7)
       CXXWARNINGS += -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wformat=2
    else ifeq ($(GCC_MAJOR),8)
       CXXWARNINGS += -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wformat=2
    endif
  endif

  ifneq ($(findstring clang,$(CXX) $(CC)),)
    # Using -Wshadow on g++ is too aggressive
    CXXWARNINGS += -Wshadow
  endif
endif


SRCS            := $(wildcard src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp src/*/*/*/*/*.cpp)
OBJS            := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

CLI_SRC         := $(wildcard src/cli/*.cpp)
CLI_APPS        := $(patsubst src/cli/%.cpp,$(BIN_DIR)/%$(BIN_EXT),$(CLI_SRC))

TEST_SRC        := $(wildcard src/test-utils/*.cpp)
TEST_UTILS      := $(patsubst src/test-utils/%.cpp,$(BIN_DIR)/test-utils/%$(BIN_EXT),$(TEST_SRC))

GUI_SRC         := $(filter src/models/% src/gui/%, $(SRCS))
GUI_OBJS        := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(GUI_SRC))
GUI_APP         := $(BIN_DIR)/untech-editor-newgui$(BIN_EXT)


# Third party libs
THIRD_PARTY_LODEPNG := $(OBJ_DIR)/vendor/lodepng/lodepng.o
THIRD_PARTY_LZ4     := $(OBJ_DIR)/vendor/lz4/lib/lz4.o $(OBJ_DIR)/vendor/lz4/lib/lz4hc.o

THIRD_PARTY_OBJS := $(THIRD_PARTY_LODEPNG) $(THIRD_PARTY_LZ4)


THIRD_PARTY_IMGUI_OBJS := $(addprefix $(OBJ_DIR)/vendor/imgui/, imgui.o imgui_draw.o imgui_widgets.o imgui_stdlib.o)
IMGUI_CXXFLAGS         := -DIMGUI_IMPL_SDL_OPENGL -DIMGUI_IMPL_OPENGL_LOADER_GL3W -Isrc/vendor/imgui/examples/libs/gl3w -Isrc/vendor/imgui

#::TODO make impl configurable::
THIRD_PARTY_IMGUI_IMPL_OBJS := $(addprefix $(OBJ_DIR)/vendor/imgui/, gl3w.o imgui_impl_sdl.o imgui_impl_opengl3.o)
IMGUI_IMPL_CXXFLAGS         := -Isrc/vendor/imgui/examples/libs/gl3w

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux) #LINUX
  IMGUI_LDFLAGS   += -lGL -ldl $(shell sdl2-config --libs)
  IMGUI_CXXFLAGS  += $(shell sdl2-config --cflags)
endif
ifeq ($(UNAME_S), Darwin) #APPLE
  IMGUI_LDFLAGS   += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo $(shell sdl2-config --libs) -L/usr/local/lib -L/opt/local/lib
  IMGUI_CXXFLAGS  += $(shell sdl2-config --cflags) -I/usr/local/include -I/opt/local/include
endif
ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
  IMGUI_LDFLAGS   += -lgdi32 -lopengl32 -limm32 $(shell pkg-config --static --libs sdl2)
  IMGUI_CXXFLAGS  += $(shell pkg-config --cflags sdl2)
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
$(call cli-modules, untech-compiler,            common snes project entity resources metasprite metatiles rooms lz4)
$(call cli-modules, untech-lz4c,                common lz4)
$(call cli-modules, untech-png2tileset,         common snes)
$(call cli-modules, untech-png2snes,            common snes)
$(call cli-modules, untech-write-sfc-checksum,  common snes)

$(call test-util-modules, serializer-test,      common snes project entity resources metasprite metatiles rooms lz4)

$(GUI_APP): $(GUI_OBJS) $(THIRD_PARTY_OBJS) $(THIRD_PARTY_IMGUI_OBJS) $(THIRD_PARTY_IMGUI_IMPL_OBJS)



# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules


DEPS := $(OBJS:.o=.d)
DEPS += $(THIRD_PARTY_OBJS:.o=.d)
-include $(DEPS)

$(TEST_UTILS): $(BIN_DIR)/test-utils/%$(BIN_EXT): $(OBJ_DIR)/test-utils/%.o
	$(CXX) $(LDFLAGS) $(CXXWARNINGS) -o $@ $^ $(LIBS)

$(CLI_APPS): $(BIN_DIR)/%$(BIN_EXT): $(OBJ_DIR)/cli/%.o
	$(CXX) $(LDFLAGS) $(CXXWARNINGS) -o $@ $^ $(LIBS)

$(GUI_APP):
	$(CXX) $(LDFLAGS) $(CXXWARNINGS) $(IMGUI_LDFLAGS) -o $@ $^ $(GUI_LIBS) $(LIBS)


$(OBJ_DIR)/vendor/%.o: src/vendor/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(VENDOR_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/%.o: src/vendor/%.c
	$(CC) $(CFLAGS) $(CXXWARNINGS) -c -o $@ $<



$(OBJ_DIR)/gui/%.o: src/gui/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(IMGUI_CXXFLAGS) -c -o $@ $<


$(OBJ_DIR)/vendor/imgui/%.o: src/vendor/imgui/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(IMGUI_CXXFLAGS) $(IMGUI_IMPL_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/imgui/%.o: src/vendor/imgui/examples/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(IMGUI_CXXFLAGS) $(IMGUI_IMPL_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/imgui/%.o: src/vendor/imgui/misc/cpp/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(IMGUI_CXXFLAGS) $(IMGUI_IMPL_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/imgui/%.o: src/vendor/imgui/examples/libs/gl3w/GL/%.c
	$(CC) $(CFLAGS) $(CXXWARNINGS) -Isrc/vendor/imgui/examples/libs/gl3w -c -o $@ $<


$(OBJ_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) -c -o $@ $<



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



.PHONY: style
style:
	find src/ -path src/vendor -prune -o \( -name '*.h' -or -name '*.hpp' -or -name '*.cpp' \) -print0 | xargs -0 clang-format -i

