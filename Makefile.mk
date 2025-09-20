# Define compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -mavx
LDFLAGS = 

# Define the target executable name
TARGET = x86simulator

# Define include directories
INCLUDES = -I../libpqxx/include

# Define C++ source files
SRCS = \
	main.cpp \
	string_utils.cpp \
	parser_utils.cpp \
	memory.cpp \
	x86_simulator_input.cpp \
	x86_simulator_private_helpers.cpp \
	x86_simulator_state.cpp \
	x86_simulator_core.cpp \
	register_rflags.cpp \
	register_map.cpp \
	DatabaseManager.cpp \
	ui_manager.cpp \
	decoder.cpp \
	CodeGenerator.cpp \
	instruction_describer.cpp \
	program_decoder.cpp

# Define object files
OBJS = $(SRCS:.cpp=.o)

# Define libraries to link
# The order matters for static libraries. libpqxx needs libpq, so it comes first.
LIBS = -L/var/local -lpqxx -lpq -lncursesw

# --- Build Targets ---

# Default target
all: $(TARGET)

# Rule to link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)


# Rule to compile C++ source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Target for cleaning up generated files
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

# Target for running the executable
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# --- Test Targets ---
TEST_SRCS = tests/decoder_test.cpp tests/memory_test.cpp tests/instruction_describer_test.cpp tests/operand_parser_test.cpp tests/register_map_test.cpp tests/rflags_test.cpp tests/program_decoder_test.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = x86_decoder_test

# Add gtest flags
GTEST_LIBS = -lgtest -lgtest_main -pthread

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TEST_TARGET) $(filter-out main.o,$(OBJS)) $(TEST_OBJS) $(LIBS) $(GTEST_LIBS)
