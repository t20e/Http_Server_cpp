# Notes on Make commands
# 	`:=` is the simple expansion, the variable is evaluated only once, at the very first occurrence.
# 	`=` is the recursive expansion, the value of the variable is evaluated every time it's used. This has downsides!
# 	`@` tells `make` to execute the command after it silently, so make doesn't print the command its about to run,.


# Assign variable `CXX` to the clang++ compiler
CXX = clang++ 

# Assign CXXFLAGS variable that defines compiler flags
# `-Wall` enable all compiler warnings
# The `-MMD` and `-MP` Generate dependency (.d) files for header tracking . When you modify a header file, 'make' will know to recompile the source files that include that header file.
CXXFLAGS = -std=c++23 -Wall -MMD -MP


# ---------------------- DIRECTORIES ----------------------
BUILD_DIR := ./build
SRC_DIR := ./src
TESTS_DIR := ./tests


# ---------------------- PATHS & LIBRARIES ----------------------
# Define include directory paths here for both the projects ./include and external libraries like (GTEST, etc..) from homebrew's include
INCLUDE_PATHS := /opt/homebrew/include ./include
# Linker library search paths
LIBS_PATHS := /opt/homebrew/lib 

# Generate compiler and linker flags from paths
# The -I tells the compiler where to look for header files (#include <...>)
# -I for includes, -L for library paths
CPPFLAGS := $(addprefix -I,$(INCLUDE_PATHS))
LDFLAGS := $(addprefix -L, $(LIBS_PATHS))

# Libraries (GTest) to link against for the test executable (test_httpServer)
TEST_LDLIBS := -lgtest_main -lgtest -lgmock -lpthread 



# ---------------------- TARGETS ----------------------
# Final executables both for the build and the test
APP_TARGET := ${BUILD_DIR}/HttpServer 
TEST_TARGET := ${BUILD_DIR}/Test_httpServer


# ---------------------- SOURCE & OBJECT FILES ----------------------

# Find all (.cpp) files recursively in the application source (./src) directory
APP_SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
# Find all (.cpp) files recursively in the tests (./tests) directory
TEST_SRCS := $(shell find $(TESTS_DIR) -name "*.cpp")

# Generate lists of object (.o) file paths in the build directory. With an identical layout
# 	Eg., src/main.cpp becomes build/main.o
APP_OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(APP_SRCS))
# For test objects also add the objects from the final executable
TEST_OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(TEST_SRCS))

# For testing, we need all application objects except for the on containing our main()
APP_OBJS_FOR_TESTS := $(filter-out $(BUILD_DIR)/$(SRC_DIR)/main.o, $(APP_OBJS))

# Create a list of dependency files (.d) for header tracking
DEPS := $(APP_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# Tell 'make' where to find all source files for the compile rule below
vpath %.cpp $(sort $(dir $(APP_SRCS) $(TEST_SRCS)))



# ---------------------- RULES ----------------------

# Makefile Automatic Variables 
# 	`$@`: The name of the APP_TARGET.
# 	`$^`: The names of all the prerequisites (e.g., main.o math_utils.o)
# 	`$<`: The name of the first prerequisite (e.g., main.cpp)

# PHONY targets are not actual files. This rule tells `Make` that `all`, `clean`, and 'test' are just commands to be executed. 
.PHONY: all clean test

# Default rule, runs when `make` command is typed in the terminal, builds the main application.
all:$(APP_TARGET)

# Rule to build and run the test test_httpServer with `make test`
test: $(TEST_TARGET)
	@echo "Running tests..."
	@./$(TEST_TARGET)

# Rule for linking all object files into the final main application executable (HttpServer).
# 	This rule only runs if the executable is missing or an object file has been modified.
$(APP_TARGET): $(APP_OBJS) 
	@echo "Linking for the main app -> (HttpServer): $@"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(APP_LDLIBS)

# Rule to link the test_httpServer executable
# 	It depends on the test objects and the necessary object files from the main application
$(TEST_TARGET): $(TEST_OBJS) $(APP_OBJS_FOR_TESTS)
	@echo "Linking for the test app -> (test_httpServer): $@"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TEST_LDLIBS)


# Rule to compile any source (.cpp) file into its corresponding object (.o) file in the build directory.
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) # Create the build directory if it doesn't exist
	@echo "Compiling: $<"
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Rule to clean-up all generated files. Run with `make clean`.
clean:
	@echo "Cleaning building files..."
	@rm -rf $(BUILD_DIR)


# Include all the generated dependency files
# THe hyphen '-' tells 'make' to ignore errors if the files don't exist yet.
-include $(DEPS)


