
CXX		?= g++
CXXFLAGS	+= -std=c++14 -Werror -Wall -Wextra -MMD -Isrc
CXXFLAGS	+= -O2
LDFLAGS		+= -Werror -Wall -Wextra

CONTROLLER_CXXFLAGS = $(shell pkg-config --cflags sigc++-2.0)
CONTROLLER_LDFLAGS = $(shell pkg-config --libs sigc++-2.0)

GTK_CXXFLAGS	= $(shell pkg-config --cflags gtkmm-3.0)
GTK_LDFLAGS	= $(shell pkg-config --libs gtkmm-3.0)

VIEW_CXXFLAGS	= $(shell wx-config --cxxflags)
VIEW_LDFLAGS	= $(shell wx-config --libs)


SRCS		= $(wildcard src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp)
OBJS		= $(patsubst src/%.cpp,obj/%.o,$(SRCS))

CLI_SRC		= $(wildcard src/cli/*.cpp)
CLI_APPS	= $(patsubst src/cli/%.cpp,bin/%,$(CLI_SRC))

GTK_SRC		= $(wildcard src/gui/*-gtk.cpp)
GTK_APPS	= $(patsubst src/gui/%.cpp,bin/%,$(GTK_SRC))

GUI_SRC		= $(filter-out %-gtk.cpp, $(wildcard src/gui/*.cpp))
GUI_APPS	= $(patsubst src/gui/%.cpp,bin/%-gui,$(GUI_SRC))

# Third party libs
THIRD_PARTY	= obj/vendor/lodepng/lodepng.o


.PHONY: all
all: cli gui

.PHONY: cli
cli: dirs $(CLI_APPS)

.PHONY: gui
gui: dirs $(GUI_APPS) $(GTK_APPS)


PERCENT = %
define cli-modules
  $(filter $(patsubst %,obj/models/%/$(PERCENT),$1), $(OBJS)) \
  $(THIRD_PARTY)
endef

define gui-modules
  $(filter $(patsubst %,obj/models/%/$(PERCENT),$1), $(OBJS)) \
  $(filter $(patsubst %,obj/gui/view/%/$(PERCENT),$1), $(OBJS)) \
  $(filter $(patsubst %,obj/gui/controllers/%.o,$1), $(OBJS)) \
  $(filter $(patsubst %,obj/gui/controllers/%/$(PERCENT),$1), $(OBJS)) \
  $(filter obj/gui/controllers/undo/%, $(OBJS)) \
  obj/gui/controllers/basecontroller.o \
  obj/gui/controllers/settings.o \
  $(THIRD_PARTY)
endef

define gtk-modules
  $(filter $(patsubst %,obj/models/%/$(PERCENT),$1), $(OBJS)) \
  $(filter $(patsubst %,obj/gui/widgets/%/$(PERCENT),$1), $(OBJS)) \
  $(filter $(patsubst %,obj/gui/controllers/%.o,$1), $(OBJS)) \
  $(filter $(patsubst %,obj/gui/controllers/%/$(PERCENT),$1), $(OBJS)) \
  $(filter obj/gui/controllers/undo/%, $(OBJS)) \
  obj/gui/controllers/basecontroller.o \
  obj/gui/controllers/settings.o \
  $(THIRD_PARTY)
endef

# Select the modules used by the apps
bin/untech-msc: $(call cli-modules, common snes metasprite metasprite-common metasprite-compiler)
bin/untech-utsi2utms: $(call cli-modules, common snes sprite-importer metasprite metasprite-common utsi2utms)

bin/untech-metasprite-gui: $(call gui-modules, common snes metasprite metasprite-common)
bin/untech-spriteimporter-gui: $(call gui-modules, common snes sprite-importer metasprite-common)

bin/untech-spriteimporter-gtk: $(call gtk-modules, common sprite-importer metasprite-common)
bin/untech-metasprite-gtk: $(call gtk-modules, common snes metasprite metasprite-common)

# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules

DEPS		= $(OBJS:.o=.d)
-include $(DEPS)

$(GUI_APPS): bin/%-gui: obj/gui/%.o
	$(CXX) $(LDFLAGS) $(VIEW_LDFLAGS) $(CONTROLLER_LDFLAGS) -o $@ $^

$(GTK_APPS): bin/%-gtk: obj/gui/%-gtk.o
	$(CXX) $(LDFLAGS) $(GTK_LDFLAGS) $(CONTROLLER_LDFLAGS) -o $@ $^

$(CLI_APPS): bin/%: obj/cli/%.o
	$(CXX) $(LDFLAGS) -o $@ $^

obj/gui/%.o: src/gui/%.cpp
	$(CXX) $(CXXFLAGS) $(VIEW_CXXFLAGS) $(CONTROLLER_CXXFLAGS) -c -o $@ $<

obj/gui/%-gtk.o: src/gui/%-gtk.cpp
	$(CXX) $(CXXFLAGS) $(GTK_CXXFLAGS) $(CONTROLLER_CXXFLAGS) -c -o $@ $<

obj/gui/controllers/%.o: src/gui/controllers/%.cpp
	$(CXX) $(CXXFLAGS) $(CONTROLLER_CXXFLAGS) -c -o $@ $<

obj/gui/view/%.o: src/gui/view/%.cpp
	$(CXX) $(CXXFLAGS) $(VIEW_CXXFLAGS) $(CONTROLLER_CXXFLAGS) -c -o $@ $<

obj/gui/widgets/%.o: src/gui/widgets/%.cpp
	$(CXX) $(CXXFLAGS) $(GTK_CXXFLAGS) $(CONTROLLER_CXXFLAGS) -c -o $@ $<

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


.PHONY: dirs
OBJECT_DIRS = $(sort $(dir $(OBJS)))
dirs: bin/ $(OBJECT_DIRS)
bin/ $(OBJECT_DIRS):
	mkdir -p $@


.PHONY: clean
clean:
	$(RM) $(DEPS)
	$(RM) $(OBJS)

.PHONY: style
style:
	find src/ -path src/vendor -prune -o \( -name '*.h' -or -name '*.hpp' -or -name '*.cpp' \) -print0 | xargs -0 clang-format -i

