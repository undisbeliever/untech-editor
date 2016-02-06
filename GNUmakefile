
CXX		?= g++
CXXFLAGS	+= -std=c++14 -Werror -Wall -Wextra -MMD
CXXFLAGS	+= -g

MODEL_SRC	= $(wildcard src/models/*.cpp src/models/*/*.cpp src/models/*/*/*.cpp)
MODEL_OBJ	= $(patsubst src/%.cpp,obj/%.o,$(MODEL_SRC))

OBJS		= $(MODEL_OBJ)

DEPS		= $(OBJS:.o=.d)

# Disable Builtin rules
.SUFFIXES:
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules


# Just build the modules for now
.PHONY: all
all: dirs $(OBJS)

-include $(DEPS)

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<


OBJECT_DIRS = $(sort $(dir $(OBJS)))
.PHONY: dirs
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

