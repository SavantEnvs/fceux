# Headless build of the fceux emulation core (no Qt/SDL frontend) + fuzz harnesses.
# Invoked by mayhem/build.sh; BUILD/CXXFLAGS select the sanitized fuzz build or the plain test build.
S      := $(abspath ..)/src
H      := $(abspath .)/harnesses
BUILD  ?= build/fuzz
CC     ?= clang
CXX    ?= clang++
FLAGS  ?=
DEFS   := -DFCEUDEF_DEBUGGER -DHAVE_ASPRINTF -DPSS_STYLE=1
INCS   := -I$(S) -I$(S)/drivers
CXXFLAGS_ALL := $(FLAGS) -w -std=gnu++17 $(DEFS) $(INCS)

CORE_SRCS := $(filter-out $(S)/lua-engine.cpp $(S)/boards/__dummy_mapper.cpp $(S)/utils/backward.cpp,\
  $(wildcard $(S)/*.cpp $(S)/boards/*.cpp $(S)/input/*.cpp $(S)/utils/*.cpp)) \
  $(S)/boards/emu2413.c $(S)/utils/ConvertUTF.c
CORE_OBJS := $(patsubst $(S)/%,$(BUILD)/core/%.o,$(CORE_SRCS))
AVI_SRCS  := $(wildcard $(S)/drivers/Qt/avi/*.cpp)
AVI_OBJS  := $(patsubst $(S)/%,$(BUILD)/core/%.o,$(AVI_SRCS))

all: $(BUILD)/libfceuxcore.a $(BUILD)/libgwavi.a $(BUILD)/driver_stub.o

$(BUILD)/libfceuxcore.a: $(CORE_OBJS)
	rm -f $@ && ar rcs $@ $^
$(BUILD)/libgwavi.a: $(AVI_OBJS)
	rm -f $@ && ar rcs $@ $^

# fceu.cpp relies on the frontend header for pal_emulation; force-include the core driver API.
$(BUILD)/core/fceu.cpp.o: EXTRA := -include $(H)/fceu_pre.h
$(BUILD)/core/%.o: $(S)/%
	@mkdir -p $(dir $@)
	$(CXX) -x c++ $(CXXFLAGS_ALL) $(EXTRA) -c $< -o $@

$(BUILD)/driver_stub.o: $(H)/driver_stub.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_ALL) -c $< -o $@

.PHONY: all
