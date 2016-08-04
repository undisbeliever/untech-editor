
PROFILE	?= release

ifeq ($(PROFILE),release)
  CXX           ?= g++
  CXXFLAGS      += -std=c++14 -O2 -flto -Wall -Wextra -MMD -Isrc
  LDFLAGS       += -O2 -flto -Wall -Wextra
  OBJ_DIR       := obj/release
  BIN_DIR       := bin
else ifeq ($(PROFILE),debug)
  CXX           ?= g++
  CXXFLAGS      += -std=c++14 -g -Werror -Wall -Wextra -MMD -Isrc
  LDFLAGS       += -g -Werror -Wall -Wextra
  OBJ_DIR       := obj/debug
  BIN_DIR       := bin/debug
else
  $(error unknown profile)
endif

CONTROLLER_CXXFLAGS := $(shell pkg-config --cflags sigc++-2.0)
CONTROLLER_LDFLAGS := $(shell pkg-config --libs sigc++-2.0)

VIEW_CXXFLAGS   := $(shell wx-config --cxxflags)
VIEW_LDFLAGS    := $(shell wx-config --libs)

SRCS            := $(wildcard src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp)
OBJS            := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

CLI_SRC         := $(wildcard src/cli/*.cpp)
CLI_APPS        := $(patsubst src/cli/%.cpp,$(BIN_DIR)/%,$(CLI_SRC))

GUI_SRC         := $(filter-out %-gtk.cpp, $(wildcard src/gui/*.cpp))
GUI_APPS        := $(patsubst src/gui/%.cpp,$(BIN_DIR)/%-gui,$(GUI_SRC))

# Third party libs
THIRD_PARTY     := $(OBJ_DIR)/vendor/lodepng/lodepng.o


.PHONY: all
all: cli gui

.PHONY: cli
cli: dirs $(CLI_APPS)

.PHONY: gui
gui: dirs $(GUI_APPS)


PERCENT = %
define cli-modules
  $(filter $(patsubst %,$(OBJ_DIR)/models/%/$(PERCENT),$1), $(OBJS)) \
  $(THIRD_PARTY)
endef

define gui-modules
  $(filter $(patsubst %,$(OBJ_DIR)/models/%/$(PERCENT),$1), $(OBJS)) \
  $(filter $(patsubst %,$(OBJ_DIR)/gui/view/%/$(PERCENT),$1), $(OBJS)) \
  $(filter $(patsubst %,$(OBJ_DIR)/gui/controllers/%.o,$1), $(OBJS)) \
  $(filter $(patsubst %,$(OBJ_DIR)/gui/controllers/%/$(PERCENT),$1), $(OBJS)) \
  $(filter $(OBJ_DIR)/gui/controllers/undo/%, $(OBJS)) \
  $(OBJ_DIR)/gui/controllers/basecontroller.o \
  $(OBJ_DIR)/gui/controllers/settings.o \
  $(THIRD_PARTY)
endef

# Select the modules used by the apps
$(BIN_DIR)/untech-msc: $(call cli-modules, common snes metasprite metasprite-common metasprite-compiler)
$(BIN_DIR)/untech-utsi2utms: $(call cli-modules, common snes sprite-importer metasprite metasprite-common utsi2utms)

$(BIN_DIR)/untech-metasprite-gui: $(call gui-modules, common snes metasprite metasprite-common)
$(BIN_DIR)/untech-spriteimporter-gui: $(call gui-modules, common snes sprite-importer metasprite-common)

# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules

DEPS		= $(OBJS:.o=.d)
-include $(DEPS)

$(GUI_APPS): $(BIN_DIR)/%-gui: $(OBJ_DIR)/gui/%.o
	$(CXX) $(LDFLAGS) $(VIEW_LDFLAGS) $(CONTROLLER_LDFLAGS) -o $@ $^

$(CLI_APPS): $(BIN_DIR)/%: $(OBJ_DIR)/cli/%.o
	$(CXX) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/gui/%.o: src/gui/%.cpp
	$(CXX) $(CXXFLAGS) $(VIEW_CXXFLAGS) $(CONTROLLER_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/gui/controllers/%.o: src/gui/controllers/%.cpp
	$(CXX) $(CXXFLAGS) $(CONTROLLER_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/gui/view/%.o: src/gui/view/%.cpp
	$(CXX) $(CXXFLAGS) $(VIEW_CXXFLAGS) $(CONTROLLER_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/gui/widgets/%.o: src/gui/widgets/%.cpp
	$(CXX) $(CXXFLAGS) $(GTK_CXXFLAGS) $(CONTROLLER_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


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

