
PROFILE     ?= release
CXX         ?= g++
CC          ?= gcc

GUI_QT_MODULES := Qt5Core Qt5Gui Qt5Widgets Qt5Svg

CXXFLAGS    += -std=c++17

ifeq ($(OS),Windows_NT)
  BIN_EXT         := .exe
  RM_COMMAND       = -del /f $(subst /,\,$1)
  # Prevents text spam
  _MISSING_DIRS    = $(foreach p,$1,$(if $(wildcard $p),,$p))
  MKDIR_P_COMMAND  = $(if $(call _MISSING_DIRS,$1),-mkdir $(subst /,\,$(call _MISSING_DIRS,$1)))

  LIBS := -lshlwapi

  VENDOR_CXXFLAGS := -Wno-deprecated

  ifndef QT_PATH
    # Find QT install dir from PATH
    QT_PATH := $(foreach p,$(subst ;, ,$(PATH)),$(if $(wildcard $p/moc.exe),$p))
    QT_PATH := $(patsubst %\bin,%,$(QT_PATH))
  endif

  GUI_QT_CXXFLAGS := -I$(QT_PATH)/include $(foreach l,$(GUI_QT_MODULES:Qt5%=Qt%),-I$(QT_PATH)/include/$l)
  GUI_QT_LIBS     := -mwindows $(foreach l,$(GUI_QT_MODULES),-l$l) -L$(QT_PATH)/lib

  MOC := moc
  UIC := uic
  RCC := rcc

else
  # Linux/BSD

  BIN_EXT         :=
  RM_COMMAND       = rm -f $1
  MKDIR_P_COMMAND  = mkdir -p $1

  LIBS :=

  VENDOR_CXXFLAGS := -Wno-deprecated

  GUI_QT_CXXFLAGS := $(shell pkg-config --cflags $(GUI_QT_MODULES))
  GUI_QT_LIBS     := $(shell pkg-config --libs $(GUI_QT_MODULES))

  ifneq ($(filter reduce_relocations, $(shell pkg-config --variable qt_config $(GUI_QT_MODULES))),)
    GUI_QT_CXXFLAGS := $(GUI_QT_CXXFLAGS) -fPIC
  endif

  MOC := moc-qt5
  UIC := uic-qt5
  RCC := rcc-qt5
endif

GEN_DIR	:= gen

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

  GUI_QT_CXXFLAGS := -fPIC $(shell pkg-config --define-variable=prefix=$(PREFIX) --cflags $(GUI_QT_MODULES))
  GUI_QT_LIBS     := $(shell pkg-config --define-variable=prefix=$(PREFIX) --libs $(GUI_QT_MODULES))

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
  # Prevent clang from spamming errors
  CXXFLAGS      += -Wno-undefined-var-template
  GUI_QT_CXXFLAGS += -Wno-deprecated

  # LTO on clang causes "signal not found" errors in Qt
  CXXFLAGS := $(filter-out -flto,$(CXXFLAGS))
  CFLAGS   := $(filter-out -flto,$(CFLAGS))
  LDFLAGS  := $(filter-out -flto,$(LDFLAGS))
endif


ifeq ($(CXXWARNINGS),)
  CXXWARNINGS := -Wall -Wextra -Wdeprecated

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

GUI_QT_SRC         := $(filter src/models/% src/gui-qt/%, $(SRCS))
GUI_QT_OBJS        := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(GUI_QT_SRC))
GUI_QT_APP         := $(BIN_DIR)/untech-editor-gui$(BIN_EXT)

GUI_QT_MOC_HEADERS := $(wildcard src/gui-qt/*.h src/gui-qt/*/*.h src/gui-qt/*/*/*.h src/gui-qt/*/*/*/*.h)
GUI_QT_MOC_GEN     := $(patsubst src/%.h,$(GEN_DIR)/%.moc.cpp,$(GUI_QT_MOC_HEADERS))
GUI_QT_MOC_OBJS    := $(patsubst src/%.h,$(OBJ_DIR)/%.moc.o,$(GUI_QT_MOC_HEADERS))

GUI_QT_UI_SRC      := $(wildcard src/gui-qt/*.ui src/gui-qt/*/*.ui src/gui-qt/*/*/*.ui)
GUI_QT_UI_GEN      := $(patsubst src/%.ui,$(GEN_DIR)/%.ui.h,$(GUI_QT_UI_SRC))
GUI_QT_UI_OBJS     := $(patsubst src/%.ui,$(OBJ_DIR)/%.o,$(GUI_QT_UI_SRC))

GUI_QT_RES_QRC     := $(wildcard resources/*.qrc)
GUI_QT_RES_GEN     := $(patsubst resources/%.qrc,$(GEN_DIR)/resources/%.cpp,$(GUI_QT_RES_QRC))
GUI_QT_RES_OBJS    := $(patsubst resources/%.qrc,$(OBJ_DIR)/resources/%.o,$(GUI_QT_RES_QRC))

GEN_QT_OBJS        := $(GUI_QT_UI_OBJS) $(GUI_QT_MOC_OBJS) $(GUI_QT_RES_OBJS)

# Third party libs
THIRD_PARTY_LODEPNG := $(OBJ_DIR)/vendor/lodepng/lodepng.o
THIRD_PARTY_LZ4     := $(OBJ_DIR)/vendor/lz4/lib/lz4.o $(OBJ_DIR)/vendor/lz4/lib/lz4hc.o

THIRD_PARTY_OBJS := $(THIRD_PARTY_LODEPNG) $(THIRD_PARTY_LZ4)


.PHONY: all
all: cli test-utils gui-qt

.PHONY: cli
cli: dirs $(CLI_APPS)

.PHONY: test-utils
test-utils: dirs $(TEST_UTILS)

.PHONY: gui-qt
gui-qt: dirs $(GUI_QT_APP)


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
$(call cli-modules, untech-compiler,            common snes project entity resources metasprite metatiles lz4)
$(call cli-modules, untech-lz4c,                common lz4)
$(call cli-modules, untech-png2tileset,         common snes)
$(call cli-modules, untech-png2snes,            common snes)
$(call cli-modules, untech-write-sfc-checksum,  common snes)

$(call test-util-modules, serializer-test,      common snes project entity resources metasprite metatiles lz4)

$(GUI_QT_APP): $(GUI_QT_OBJS) $(GEN_QT_OBJS) $(THIRD_PARTY_OBJS)


# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules


DEPS := $(OBJS:.o=.d)
DEPS += $(GEN_QT_OBJS:.o=.d)
DEPS += $(THIRD_PARTY_OBJS:.o=.d)
-include $(DEPS)

$(GUI_QT_APP):
	$(CXX) $(LDFLAGS) $(CXXWARNINGS) -o $@ $^ $(GUI_QT_LIBS) $(LIBS)

$(TEST_UTILS): $(BIN_DIR)/test-utils/%$(BIN_EXT): $(OBJ_DIR)/test-utils/%.o
	$(CXX) $(LDFLAGS) $(CXXWARNINGS) -o $@ $^ $(LIBS)

$(CLI_APPS): $(BIN_DIR)/%$(BIN_EXT): $(OBJ_DIR)/cli/%.o
	$(CXX) $(LDFLAGS) $(CXXWARNINGS) -o $@ $^ $(LIBS)


$(GUI_QT_UI_OBJS): $(OBJ_DIR)/gui-qt/%.o: src/gui-qt/%.cpp $(GEN_DIR)/gui-qt/%.ui.h
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(GUI_QT_CXXFLAGS) -I$(GEN_DIR) -c -o $@ $<

$(OBJ_DIR)/gui-qt/%.o: src/gui-qt/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(GUI_QT_CXXFLAGS) -I$(GEN_DIR) -c -o $@ $<

$(OBJ_DIR)/gui-qt/%.moc.o: $(GEN_DIR)/gui-qt/%.moc.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(GUI_QT_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/resources/%.o: $(GEN_DIR)/resources/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) -c -o $@ $<


$(OBJ_DIR)/vendor/%.o: src/vendor/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(VENDOR_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/%.o: src/vendor/%.c
	$(CC) $(CFLAGS) $(CXXWARNINGS) -c -o $@ $<


$(OBJ_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) -c -o $@ $<



.SECONDARY: $(GUI_QT_MOC_GEN)
$(GEN_DIR)/gui-qt/%.moc.cpp: src/gui-qt/%.h
	$(MOC) -o $@ $<

.SECONDARY: $(GUI_QT_UI_GEN)
$(GEN_DIR)/gui-qt/%.ui.h: src/gui-qt/%.ui
	$(UIC) -o $@ $<

.SECONDARY: $(GUI_QT_RES_GEN)
$(GEN_DIR)/resources/%.cpp: resources/%.qrc
	$(RCC) -o $@ $<

$(foreach r,$(GUI_QT_RES_QRC),$(eval $(r:resources/%.qrc=$(GEN_DIR)/resources/%.cpp): $(shell $(RCC) --list $r)))



.PHONY: dirs
OBJECT_DIRS := $(sort $(dir $(OBJS) $(GEN_QT_OBJS) $(THIRD_PARTY_OBJS)))
GEN_DIRS := $(sort $(dir $(GUI_QT_UI_GEN) $(GUI_QT_MOC_GEN) $(GUI_QT_RES_GEN)))
dirs: $(BIN_DIR)/ $(BIN_DIR)/test-utils/ $(GEN_DIR)/ $(OBJECT_DIRS) $(GEN_DIRS)
$(BIN_DIR)/ $(BIN_DIR)/test-utils/ $(GEN_DIR)/ $(OBJECT_DIRS) $(GEN_DIRS):
	$(call MKDIR_P_COMMAND,$@)



.PHONY: clean
clean:
	$(call RM_COMMAND, $(DEPS))
	$(call RM_COMMAND, $(OBJS))
	$(call RM_COMMAND, $(THIRD_PARTY_OBJS))
	$(call RM_COMMAND, $(GUI_QT_MOC_GEN))
	$(call RM_COMMAND, $(GUI_QT_MOC_OBJS))
	$(call RM_COMMAND, $(GUI_QT_UI_GEN))
	$(call RM_COMMAND, $(GUI_QT_RES_OBJS))
	$(call RM_COMMAND, $(GUI_QT_RES_GEN) $(GUI_QT_RES_GEN:.cpp=.d))



.PHONY: style
style:
	find src/ -path src/vendor -prune -o \( -name '*.h' -or -name '*.hpp' -or -name '*.cpp' \) -print0 | xargs -0 clang-format -i

