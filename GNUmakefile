
CXX		?= g++
CXXFLAGS	+= -std=c++14 -Werror -Wall -Wextra -MMD -Isrc
CXXFLAGS	+= -g
LDFLAGS		+= -Werror -Wall -Wextra

# gtkmm3
GUI_CXXFLAGS	= $(shell pkg-config --cflags gtkmm-3.0)
GUI_LDFLAGS	= $(shell pkg-config --libs gtkmm-3.0)


MODEL_SRC	= $(wildcard src/models/*.cpp src/models/*/*.cpp src/models/*/*/*.cpp)
MODEL_OBJ	= $(patsubst src/%.cpp,obj/%.o,$(MODEL_SRC))

CLI_SRC		= $(wildcard src/cli/*.cpp)
CLI_OBJ		= $(patsubst src/%.cpp,obj/%.o,$(CLI_SRC))
CLI_APPS	= $(patsubst src/cli/%.cpp,bin/%,$(CLI_SRC))

GUI_SRC		= $(wildcard src/gui/*.cpp src/gui/*/*.cpp src/gui/*/*/*.cpp)
GUI_OBJ		= $(patsubst src/gui/%.cpp,obj/gui/%.o,$(GUI_SRC))
GUI_APPS	= $(patsubst src/gui/%.cpp,bin/%-gui,$(wildcard src/gui/*.cpp))

OBJS		= $(MODEL_OBJ) $(CLI_OBJ) $(GUI_OBJ)
DEPS		= $(OBJS:.o=.d)

.PHONY: all
all: dirs $(CLI_APPS) $(GUI_APPS)


PERCENT = %
define app-models
$(filter $(patsubst %,obj/models/%/$(PERCENT),$1), $(MODEL_OBJ))
endef

PERCENT = %
define gui-widgets
$(filter $(patsubst %,obj/gui/widgets/%/$(PERCENT),$1), $(GUI_OBJ))
endef

# Select the models used by the apps
bin/untech-spriteimporter: $(call app-models, common sprite-importer)
bin/untech-spriteimporter-gui: $(call app-models, common sprite-importer)
bin/untech-spriteimporter-gui: $(call gui-widgets, common sprite-importer)



# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules

-include $(DEPS)

bin/%-gui: obj/gui/%.o
	$(CXX) $(LDFLAGS) $(GUI_LDFLAGS) -o $@ $^

bin/%: obj/cli/%.o
	$(CXX) $(LDFLAGS) -o $@ $^

obj/gui/%.o: src/gui/%.cpp
	$(CXX) $(CXXFLAGS) $(GUI_CXXFLAGS) -o $@ -c $<

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<


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
	find src/ \( -name '*.h' -or -name '*.cpp' \) -print0 | xargs -0 clang-format -i

