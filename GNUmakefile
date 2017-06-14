
PROFILE	?= release

CXXWARNINGS	?= -Wall -Wextra -Wdeprecated

ifeq ($(PROFILE),release)
  OBJ_DIR       := obj/release
  BIN_DIR       := bin
  BIN_EXT	:=

  CXX           ?= g++
  CXXFLAGS      += -std=c++14 -O2 -flto -MMD -Isrc
  LDFLAGS       += -O2 -flto
  LIBS	        :=
  WX_CONTROLLER_CXXFLAGS := $(shell pkg-config --cflags sigc++-2.0)
  WX_CONTROLLER_LIBS := $(shell pkg-config --libs sigc++-2.0)
  WX_VIEW_CXXFLAGS := $(shell wx-config --cxxflags)
  WX_VIEW_LIBS := $(shell wx-config --libs)
  VENDOR_CXXFLAGS := -Wno-deprecated

else ifeq ($(PROFILE),debug)
  OBJ_DIR       := obj/debug
  BIN_DIR       := bin/debug
  BIN_EXT	:=

  CXX           ?= g++
  CXXFLAGS      += -std=c++14 -g -MMD -Isrc -Werror
  LDFLAGS       += -g -Werror
  LIBS	        :=
  WX_CONTROLLER_CXXFLAGS := $(shell pkg-config --cflags sigc++-2.0)
  WX_CONTROLLER_LIBS := $(shell pkg-config --libs sigc++-2.0)
  WX_VIEW_CXXFLAGS := $(shell wx-config --cxxflags)
  WX_VIEW_LIBS := $(shell wx-config --libs)
  VENDOR_CXXFLAGS := -Wno-deprecated

else ifeq ($(PROFILE),mingw)
  # MinGW cross platform compiling
  CXX_MINGW     ?= x86_64-w64-mingw32-g++
  PREFIX	?= $(HOME)/.local/x86_64-w64-mingw32

  OBJ_DIR       := obj/mingw
  BIN_DIR       := bin/mingw
  BIN_EXT	:= .exe

  CXX           := $(CXX_MINGW)
  CXXFLAGS      += -std=c++14 -O2 -flto -MMD -Isrc
  LDFLAGS       += -O2 -flto
  LIBS	        := -lshlwapi
  WX_CONTROLLER_CXXFLAGS := $(shell pkg-config --define-variable=prefix=$(PREFIX) --cflags sigc++-2.0)
  WX_CONTROLLER_LIBS := $(shell pkg-config --define-variable=prefix=$(PREFIX) --libs sigc++-2.0)
  WX_VIEW_CXXFLAGS := $(shell $(PREFIX)/bin/wx-config --cxxflags)
  WX_VIEW_LIBS := $(shell $(PREFIX)/bin/wx-config --libs)
  VENDOR_CXXFLAGS := -Wno-deprecated

else
  $(error unknown profile)
endif

ifneq ($(findstring clang,$(CXX)),)
  # Prevent clang from spamming errors
  CXXFLAGS      += -Wno-undefined-var-template
  WX_VIEW_CXXFLAGS += -Wno-potentially-evaluated-expression -Wno-deprecated
endif

SRCS            := $(wildcard src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp src/*/*/*/*/*.cpp)
OBJS            := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

CLI_SRC         := $(wildcard src/cli/*.cpp)
CLI_APPS        := $(patsubst src/cli/%.cpp,$(BIN_DIR)/%$(BIN_EXT),$(CLI_SRC))

GUI_WX_SRC         := $(filter-out %-gtk.cpp, $(wildcard src/gui-wx/*.cpp))
GUI_WX_APPS        := $(patsubst src/gui-wx/%.cpp,$(BIN_DIR)/%-wxgui$(BIN_EXT),$(GUI_WX_SRC))

# Third party libs
THIRD_PARTY     := $(OBJ_DIR)/vendor/lodepng/lodepng.o


.PHONY: all
all: cli gui

.PHONY: cli
cli: dirs $(CLI_APPS)

.PHONY: gui
gui: dirs $(GUI_WX_APPS)


PERCENT = %
define cli-modules
$(BIN_DIR)/$(strip $1)$(BIN_EXT): \
  $(filter $(patsubst %,$(OBJ_DIR)/models/%/$(PERCENT),$2), $(OBJS)) \
  $(filter $(OBJ_DIR)/cli/helpers/%, $(OBJS)) \
  $(THIRD_PARTY)
endef

define gui-modules
$(BIN_DIR)/$(strip $1)$(BIN_EXT): \
  $(filter $(patsubst %,$(OBJ_DIR)/models/%/$(PERCENT),$2), $(OBJS)) \
  $(filter $(patsubst %,$(OBJ_DIR)/gui-wx/view/%/$(PERCENT),$2), $(OBJS)) \
  $(filter $(patsubst %,$(OBJ_DIR)/gui-wx/controllers/%.o,$2), $(OBJS)) \
  $(filter $(patsubst %,$(OBJ_DIR)/gui-wx/controllers/%/$(PERCENT),$2), $(OBJS)) \
  $(filter $(OBJ_DIR)/gui-wx/controllers/undo/%, $(OBJS)) \
  $(OBJ_DIR)/gui-wx/controllers/basecontroller.o \
  $(THIRD_PARTY)
endef

# Select the modules used by the apps
$(call cli-modules, untech-msc,		common snes metasprite)
$(call cli-modules, untech-png2tileset, common snes)
$(call cli-modules, untech-png2snes,    common snes)
$(call cli-modules, untech-utsi2utms,	common snes metasprite)

$(call gui-modules, untech-metasprite-wxgui,     common snes metasprite)
$(call gui-modules, untech-spriteimporter-wxgui, common snes metasprite)

# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules

DEPS = $(OBJS:.o=.d)
-include $(DEPS)

$(GUI_WX_APPS): $(BIN_DIR)/%-wxgui$(BIN_EXT): $(OBJ_DIR)/gui-wx/%.o
	$(CXX) $(LDFLAGS) $(CXXWARNINGS) -o $@ $^ $(WX_VIEW_LIBS) $(WX_CONTROLLER_LIBS) $(LIBS)

$(CLI_APPS): $(BIN_DIR)/%$(BIN_EXT): $(OBJ_DIR)/cli/%.o
	$(CXX) $(LDFLAGS) $(CXXWARNINGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/gui-wx/%.o: src/gui-wx/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(WX_VIEW_CXXFLAGS) $(WX_CONTROLLER_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/gui-wx/controllers/%.o: src/gui-wx/controllers/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(WX_CONTROLLER_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/gui-wx/view/%.o: src/gui-wx/view/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(WX_VIEW_CXXFLAGS) $(WX_CONTROLLER_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/vendor/%.o: src/vendor/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) $(VENDOR_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXWARNINGS) -c -o $@ $<


.PHONY: dirs
OBJECT_DIRS := $(sort $(dir $(OBJS)))
dirs: $(BIN_DIR)/ $(OBJECT_DIRS)
$(BIN_DIR)/ $(OBJECT_DIRS):
	mkdir -p $@


.PHONY: clean
clean:
	$(RM) $(DEPS)
	$(RM) $(OBJS)

.PHONY: style
style:
	find src/ -path src/vendor -prune -o \( -name '*.h' -or -name '*.hpp' -or -name '*.cpp' \) -print0 | xargs -0 clang-format -i

