MODE				?= debug
CC					= clang++
SRC					= $(wildcard source/*.cpp)
BUILD_DIR		= build
SOURCE_DIR 	= source
EXE					= $(BUILD_DIR)/$(MODE)/search
DEPS_FILE 	= $(BUILD_DIR)/conanbuildinfo.mak

ifneq ("$(wildcard $(DEPS_FILE))","")
include $(DEPS_FILE)
endif

INCLUDE_DIRS += $(CONAN_INCLUDE_DIRS)
LIB_DIRS += $(CONAN_LIB_DIRS)
LDLIBS += $(CONAN_LIBS)

INCLUDE_FLAGS = $(INCLUDE_DIRS:%=-I%) 
LINKER_FLAGS = $(INCLUDE_FLAGS) $(LIB_DIRS:%=-L%) $(LDLIBS:%=-l%)

common_flags = -Wall -Wpedantic -std=c++17
ifeq ($(MODE), debug)
	CFLAGS = -g -D_GLIBCXX_DEBUG=1
else
	CFLAGS = -O3
endif

CFLAGS += $(common_flags)

OBJS = $(SRC:$(SOURCE_DIR)/%.cpp=$(BUILD_DIR)/%.o)

$(EXE): $(OBJS)
	@mkdir -p $(dir $(EXE))
	$(CC) $(CFLAGS) -o $(EXE) $? $(LINKER_FLAGS)

$(OBJS): $(SRC)
	@./scripts/check_deps.sh $(DEPS_FILE)
	@mkdir -p $(dir $(EXE))
	$(CC) $(CFLAGS) -c$(patsubst $(BUILD_DIR)/%.cpp, $(SOURCE_DIR)/%.cpp, $*.cpp) -o $*.o $(INCLUDE_FLAGS)

.PHONY: install
install:
	@mkdir -p $(dir $(EXE))
	@cd $(BUILD_DIR) && conan install ../
	@cd ../

.PHONY: release
release:
	@$(MAKE) MODE=release --no-print-directory

.PHONY: debug
debug:
	@$(MAKE) MODE=debug --no-print-directory

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*.o
	rm -rf $(BUILD_DIR)/debug/*
	rm -rf $(BUILD_DIR)/release/*

.PHONY: count
count:
	cloc --by-file source/*

.PHONY: run
run: $(EXE)
	@$(EXE)

