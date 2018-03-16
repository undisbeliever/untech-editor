
PROFILE     ?= release
CXX         ?= g++
CC          ?= gcc

ifeq ($(CXXWARNINGS),)
  CXXWARNINGS := -Wall -Wextra -Wdeprecated

  ifneq ($(findstring g++,$(CXX)),)
    GCC_MAJOR := $(firstword $(subst ., ,$(shell $(CXX) -dumpversion)))

    ifeq ($(GCC_MAJOR),5)
       CXXWARNINGS += -Wlogical-op -Wdouble-promotion -Wformat=2
    else ifeq ($(GCC_MAJOR),6)
       CXXWARNINGS += -Wduplicated-cond -Wlogical-op -Wnull-dereference -Wdouble-promotion -Wformat=2
    else ifeq ($(GCC_MAJOR),7)
       CXXWARNINGS += -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wformat=2
    endif
  endif
endif


GUI_QT_MODULES := Qt5Core Qt5Gui Qt5Widgets Qt5Svg

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

  CXXFLAGS      += -std=c++14 -O2 -flto -fdata-sections -ffunction-sections -MMD -Isrc
  CFLAGS        += -O2 -flto -fdata-sections -ffunction-sections -MMD -Isrc
  LDFLAGS       += -O2 -flto -Wl,-gc-sections

else ifeq ($(PROFILE),debug)
  OBJ_DIR       := obj/debug
  BIN_DIR       := bin/debug

  CXXFLAGS      += -std=c++14 -g -MMD -Isrc -Werror -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
  CFLAGS        += -g -MMD -Isrc -Werror
  LDFLAGS       += -g -Werror

else ifeq ($(PROFILE),mingw)
  # MinGW cross platform compiling
  CXX_MINGW     ?= x86_64-w64-mingw32-g++
  PREFIX	?= $(HOME)/.local/x86_64-w64-mingw32

  OBJ_DIR       := obj/mingw
  BIN_DIR       := bin/mingw
  BIN_EXT	:= .exe

  CXX           := $(CXX_MINGW)
  CXXFLAGS      += -std=c++14 -O2 -flto -MMD -Isrc
  CFLAGS        += -O2 -flto -MMD -Isrc
  LDFLAGS       += -O2 -flto
  LIBS          += -lshlwapi

  GUI_QT_CXXFLAGS := -fPIC $(shell pkg-config --define-variable=prefix=$(PREFIX) --cflags $(GUI_QT_MODULES))
  GUI_QT_LIBS     := $(shell pkg-config --define-variable=prefix=$(PREFIX) --libs $(GUI_QT_MODULES))

else
  $(error unknown profile)
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



SRCS            := $(wildcard src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp src/*/*/*/*/*.cpp)
OBJS            := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

CLI_SRC         := $(wildcard src/cli/*.cpp)
CLI_APPS        := $(patsubst src/cli/%.cpp,$(BIN_DIR)/%$(BIN_EXT),$(CLI_SRC))

TEST_SRC        := $(wildcard src/test-utils/*.cpp)
TEST_UTILS      := $(patsubst src/test-utils/%.cpp,$(BIN_DIR)/test-utils/%$(BIN_EXT),$(TEST_SRC))

GUI_QT_SRC         := $(wildcard src/gui-qt/*.cpp)
GUI_QT_APPS        := $(patsubst src/gui-qt/%.cpp,$(BIN_DIR)/%-gui$(BIN_EXT),$(GUI_QT_SRC))

GUI_QT_MOC_HEADERS := $(wildcard src/gui-qt/*.h src/gui-qt/*/*.h src/gui-qt/*/*/*.h src/gui-qt/*/*/*/*.h)
GUI_QT_MOC_GEN     := $(patsubst src/%.h,$(GEN_DIR)/%.moc.cpp,$(GUI_QT_MOC_HEADERS))
GUI_QT_MOC_OBJS    := $(patsubst src/%.h,$(OBJ_DIR)/%.moc.o,$(GUI_QT_MOC_HEADERS))

GUI_QT_UI_SRC      := $(wildcard src/gui-qt/*/*.ui src/gui-qt/*/*/*.ui src/gui-qt/*/*/*/*.ui)
GUI_QT_UI_GEN      := $(patsubst src/%.ui,$(GEN_DIR)/%.ui.h,$(GUI_QT_UI_SRC))
GUI_QT_UI_OBJS     := $(patsubst src/%.ui,$(OBJ_DIR)/%.o,$(GUI_QT_UI_SRC))

GUI_QT_RES_QRC     := $(wildcard resources/*.qrc)
GUI_QT_RES_GEN     := $(patsubst resources/%.qrc,$(GEN_DIR)/resources/%.cpp,$(GUI_QT_RES_QRC))
GUI_QT_RES_OBJS    := $(patsubst resources/%.qrc,$(OBJ_DIR)/resources/%.o,$(GUI_QT_RES_QRC))

GEN_OBJS           := $(GUI_QT_UI_OBJS) $(GUI_QT_MOC_OBJS) $(GUI_QT_RES_OBJS)

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
gui-qt: dirs $(GUI_QT_APPS)


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

# usage: <bin> <resource file> <modules>
define gui-qt-modules
$(BIN_DIR)/$(strip $1)$(BIN_EXT): \
  $(filter $(patsubst %,$(OBJ_DIR)/models/%/$(PERCENT),$3), $(OBJS)) \
  $(filter $(patsubst %,$(OBJ_DIR)/gui-qt/%/$(PERCENT),$3), $(OBJS)) \
  $(filter $(patsubst %,$(OBJ_DIR)/gui-qt/%/$(PERCENT),$3), $(GUI_QT_MOC_OBJS)) \
  $(OBJ_DIR)/resources/$(strip $2).o \
  $(THIRD_PARTY_LODEPNG) \
  $(if $(filter lz4,$3), $(THIRD_PARTY_LZ4))
endef


# Select the modules used by the apps
$(call cli-modules, untech-lz4c,                common lz4)
$(call cli-modules, untech-msc,                 common snes metasprite)
$(call cli-modules, untech-png2tileset,         common snes)
$(call cli-modules, untech-png2snes,            common snes)
$(call cli-modules, untech-resc,                common lz4 resources metatiles snes)
$(call cli-modules, untech-utsi2utms,           common snes metasprite)
$(call cli-modules, untech-write-sfc-checksum,  common snes)

$(call test-util-modules, metasprite-serializer-test,           common snes metasprite)
$(call test-util-modules, spriteimporter-serializer-test,       common snes metasprite)
$(call test-util-modules, resources-file-serializer-test,       common snes resources metatiles lz4)
$(call test-util-modules, metatiles-tileset-serializer-test,    common snes resources metatiles lz4)

$(call gui-qt-modules, untech-metasprite-gui,     metasprite, common snes metasprite)
$(call gui-qt-modules, untech-spriteimporter-gui, metasprite, common snes metasprite)
$(call gui-qt-modules, untech-resources-gui,      resources, common snes metatiles resources lz4)



# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules


DEPS := $(OBJS:.o=.d)
DEPS += $(GEN_OBJS:.o=.d)
DEPS += $(THIRD_PARTY_OBJS:.o=.d)
-include $(DEPS)

$(GUI_QT_APPS): $(BIN_DIR)/%-gui$(BIN_EXT): $(OBJ_DIR)/gui-qt/%.o
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
OBJECT_DIRS := $(sort $(dir $(OBJS) $(GEN_OBJS) $(THIRD_PARTY_OBJS)))
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

