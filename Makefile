
# ======== 🌟 NOTE 🌟 ========
# I used this Makefile before I implemented CMake with Conan. It is a good template for basic
# projects without CMake, justing using a Makefile.
# ============================



# Notes on Make commands
# 	`:=` is the simple expansion, the variable is evaluated only once, at the very first occurrence.
# 	`=` is the recursive expansion, the value of the variable is evaluated every time it's used. This has downsides!
# 	`@` tells `make` to execute the command after it silently, so make doesn't print the command its about to run,.

# ---------------------- COMPILERS & FLAGS ----------------------
# Assign variable `CPP` to the clang++ compiler
CPP := clang++ 
CC := clang # C compiler, this is solely used for the SQLITE .c source files

# Assign CPPFLAGS variable that defines compiler flags
# `-Wall` enable all compiler warnings
# The `-MMD` and `-MP` Generate dependency (.d) files for header tracking . When you modify a header file, 'make' will know to recompile the source files that include that header file. -Wextra enables additional useful warnings that are not included in -Wall.
CPPFLAGS := -std=c++23 -Wall -MMD -MP
CFLAGS := -O3 -DSQLITE_THREADSAFE=1


# ---------------------- DIRECTORIES ----------------------
BUILD_DIR := ./build
SRC_DIR := ./src
TESTS_DIR := ./tests

# Final executables both for the build and the test
APP_TARGET := ${BUILD_DIR}/HttpServer 
TEST_TARGET := ${BUILD_DIR}/Test_httpServer


# ---------------------- PATHS & LIBRARIES ----------------------
# Define include directory paths here for both the projects ./include and external libraries like (GTEST, etc..) from homebrew's include
INCLUDE_PATHS := /opt/homebrew/include ./include  ./libs/Bcrypt.cpp
# Linker library search paths
LIBS_PATHS := /opt/homebrew/lib 

# SQLite
SQLITE_SRC := ./libs/sqlite-amalgamation-3510000/sqlite3.c
SQLITE_OBJ := $(BUILD_DIR)/libs/sqlite-amalgamation-3510000/sqlite3.o

# Bcrypt
BCRYPT_DIR := ./libs/Bcrypt.cpp
BCRYPT_SRCS := $(wildcard $(BCRYPT_DIR)/*.cpp) # Find all .cpp files in libs/bcrypt
BCRYPT_OBJS := $(BCRYPT_SRCS:%.cpp=$(BUILD_DIR)/%.o) # Create an object file for that source files in the build

# Compiler flags
# The -I tells the compiler where to look for header files (#include <...>)
# -I for includes, -L for library paths
CPPFLAGS += $(addprefix -I,$(INCLUDE_PATHS))
LDFLAGS := $(addprefix -L, $(LIBS_PATHS))

# Libraries (GTest) to link against for the test executable (test_httpServer)
TEST_LDLIBS := -lgtest_main -lgtest -lgmock -lpthread 


# ---------------------- SOURCE & OBJECT FILES ----------------------
# Find all (.cpp) files recursively in the application source (./src) directory
APP_SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
# Find all (.cpp) files recursively in the tests (./tests) directory
TEST_SRCS := $(shell find $(TESTS_DIR) -name "*.cpp")

# Generate lists of object (.o) file paths in the build directory, with an identical layout, e.g., src/main.cpp becomes build/main.o
APP_OBJS := $(APP_SRCS:%.cpp=$(BUILD_DIR)/%.o)
# For test objects also add the objects from the final executable
TEST_OBJS := $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)

# For testing, we exclude our main() function.
APP_MAIN_OBJ := $(BUILD_DIR)/$(SRC_DIR)/main.o
APP_OBJS_FOR_TESTS := $(filter-out $(APP_MAIN_OBJ), $(APP_OBJS))

# Create a list of dependency files (.d) for header tracking
DEPS := $(APP_OBJS:.o=.d) $(TEST_OBJS:.o=.d) $(BCRYPT_OBJS:.o=.d)


# ---------------------- RULES ----------------------

# Makefile Automatic Variables 
# 	`$@`: The name of the APP_TARGET.
# 	`$^`: The names of all the prerequisites (e.g., main.o math_utils.o)
# 	`$<`: The name of the first prerequisite (e.g., main.cpp)

# PHONY targets are not actual files. This rule tells `Make` that `all`, `clean`, and 'test' are just commands to be executed. 
.PHONY: all clean test run

# Default rule, runs when `make` command is typed in the terminal, builds the main application.
all:$(APP_TARGET)

# Function to print a separator of 64 (-)
PRINT_SEPARATOR := @printf "%64s" | tr ' ' '-'

# Rule for linking all object files into the final main application executable (HttpServer).
# 	This rule only runs if the executable is missing or an object file has been modified.
$(APP_TARGET): $(APP_OBJS) $(BCRYPT_OBJS) $(SQLITE_OBJ)
	@mkdir -p $(dir $@)
	@echo "Linking Main app → $@"
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to link the Test App. It depends on the test objects and the necessary object files from the main application.
$(TEST_TARGET): $(TEST_OBJS) $(APP_OBJS_FOR_TESTS) $(BCRYPT_OBJS) $(SQLITE_OBJ)
	@mkdir -p $(dir $@)
	@echo "Linking for the test app → $@"
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(TEST_LDLIBS)

# Rule to compile any source file (.cpp) into its corresponding object (.o) file in the build directory.
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) 
	@echo "Compiling C++ source files (.cpp) into object files (.o): $<"
	$(CPP) $(CPPFLAGS) -c $< -o $@

# Since the SQLITE files are written in C, we need to use the C compiler.
$(SQLITE_OBJ): $(SQLITE_SRC)
	@mkdir -p $(dir $@)
	@echo "Compiling SQLite: $<"
	$(CC) $(CFLAGS) -c $< -o $@


# ---------------------- COMMANDS ----------------------
test: $(TEST_TARGET) # Build and run the test test_httpServer with `make test`
	@echo "\nRunning Tests: (Test_httpServer)..."
	$(PRINT_SEPARATOR)
	@./$(TEST_TARGET)

run: $(APP_TARGET) # Run app with `make run`
	@echo "\nRunning Build Application: (HttpServer)..."
	$(PRINT_SEPARATOR)
	@./$(APP_TARGET)

# Rule to clean-up all generated files. Run with `make clean`.
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)

# Include all the generated dependency files
# THe hyphen '-' tells 'make' to ignore errors if the files don't exist yet.
-include $(DEPS)
