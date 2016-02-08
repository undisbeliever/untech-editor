
CXX		?= g++
CXXFLAGS	+= -std=c++14 -Werror -Wall -Wextra -MMD
CXXFLAGS	+= -g
LDFLAGS		 = -Werror -Wall -Wextra

MODEL_SRC	= $(wildcard src/models/*.cpp src/models/*/*.cpp src/models/*/*/*.cpp)
MODEL_OBJ	= $(patsubst src/%.cpp,obj/%.o,$(MODEL_SRC))

CLI_SRC		= $(wildcard src/cli/*.cpp)
CLI_OBJ		= $(patsubst src/%.cpp,obj/%.o,$(CLI_SRC))
CLI_APPS	= $(patsubst src/cli/%.cpp,bin/%,$(CLI_SRC))

OBJS		= $(MODEL_OBJ) $(CLI_OBJ)
DEPS		= $(OBJS:.o=.d)


PERCENT = %
define app-models
$(filter $(patsubst %,obj/models/%/$(PERCENT),$1), $(MODEL_OBJ))
endef

# Select the models used by the apps
bin/untech-spriteimporter: $(call app-models, common sprite-importer)



# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules


.PHONY: all
all: dirs $(CLI_APPS)

-include $(DEPS)

bin/%: obj/cli/%.o
	$(CXX) $(LDFLAGS) -o $@ $^

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

