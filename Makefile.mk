# Define compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -DGOOGLE_TEST
LDFLAGS =

# Define the target executable name
TARGET = x86simulator

# Define include directories
INCLUDES = -I../libpqxx/include

# Define library source files (all sources except main.cpp)
LIB_SRCS = \
	avx_core.cpp \
	string_utils.cpp \
	parser_utils.cpp \
	memory.cpp \
	x86_simulator_input.cpp \
	x86_simulator_private_helpers.cpp \
	x86_simulator_state.cpp \
	x86_simulator_core.cpp \
	system_bus.cpp \
	register_rflags.cpp \
	register_map.cpp \
	DatabaseManager.cpp \
	file_system_device.cpp \
	ui_manager.cpp \
	decoder.cpp \
	CodeGenerator.cpp \
	instruction_describer.cpp \
	ir_executor_helpers.cpp \
	program_decoder.cpp \
	formatting_utils.cpp \
	architecture.cpp \
	x86_to_ir.cpp

# Define object files
LIB_OBJS = $(LIB_SRCS:.cpp=.o)
MAIN_OBJ = main.o

# Define libraries to link
# The order matters for static libraries. libpqxx needs libpq, so it comes first.
LIBS = -L/var/local -lpqxx -lpq -lncursesw

# --- Build Targets ---

# Default target
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(MAIN_OBJ) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_OBJ) $(LIB_OBJS) $(LIBS)

# --- Compilation Rules ---

# Generic rule for compiling C++ source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


# --- Test Targets ---
TEST_SRCS = \
	tests/ir_executor_test.cpp \
	tests/mock_database_manager.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = x86_decoder_test

# Add gtest flags
GTEST_LIBS = -lgtest -lgtest_main -pthread
TEST_MAIN_OBJ = tests/test_main.o

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(LIB_OBJS) $(TEST_OBJS) $(TEST_MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(LIB_OBJS) $(TEST_OBJS) $(TEST_MAIN_OBJ) $(LIBS) $(GTEST_LIBS)

# Target for cleaning up generated files
.PHONY: clean
clean:	
	rm -f $(LIB_OBJS) $(MAIN_OBJ) $(TEST_OBJS) $(TEST_MAIN_OBJ) $(TARGET) $(TEST_TARGET)

# Target for running the executable
.PHONY: run
run: $(TARGET)
	./$(TARGET)
