# Define compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -DGOOGLE_TEST
LDFLAGS =

# Define the target executable name
TARGET = simulator

# Define include directories
INCLUDES = -I../libpqxx/include

# Define library source files (all sources except main.cpp)
LIB_SRCS = \
	avx_core.cpp \
	string_utils.cpp \
	parser_utils.cpp \
	memory.cpp \
	x86_simulator_input.cpp \
	x86_simulator_state.cpp \
	x86_simulator_core.cpp \
	system_bus.cpp \
	register_rflags.cpp \
	register_map.cpp \
	generic_register_map.cpp \
	DatabaseManager.cpp \
	file_system_device.cpp \
	ui_manager.cpp \
	arm_ui_manager.cpp \
	decoder.cpp \
	CodeGenerator.cpp \
	instruction_describer.cpp \
	ARM_helpers.cpp \
	pipeline.cpp \
	pipelined_instruction.cpp \
	fetch_stage.cpp \
	decode_stage.cpp \
	execute_stage.cpp \
	memory_stage.cpp \
	write_back_stage.cpp \
	alu.cpp \
	fpu.cpp \
	vpu.cpp \
	program_decoder.cpp \
	formatting_utils.cpp \
	architecture.cpp \
	x86_to_ir.cpp \
	arm_to_ir.cpp \
	arm_simulator.cpp \
	ir.cpp \
	scoreboard.cpp \
	score_counter.cpp \
	execution_helpers.cpp

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
	tests/vpu_test.cpp \
	tests/decoder_test.cpp \
	tests/alu_test.cpp \
	tests/formatting_utils_test.cpp \
	tests/instruction_describer_test.cpp \
	tests/memory_test.cpp \
	tests/operand_parser_test.cpp \
	tests/program_decoder_test.cpp \
	tests/register_map_test.cpp \
	tests/arm_register_map_test.cpp \
	tests/rflags_test.cpp \
	tests/system_bus_test.cpp \
	tests/ir_translation_test.cpp \
	tests/arm_ir_translation_test.cpp \
	tests/ir_executor_test.cpp \
	tests/arm_ir_executor_test.cpp \
	tests/file_system_device_test.cpp \
	tests/parser_utils_test.cpp \
	tests/simulator_integration_test.cpp \
	tests/arm_to_ir_converter_test.cpp \
	tests/arm_simulator_ui_test.cpp \
	tests/arm_simulator_test.cpp \
	tests/mock_database_manager.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = run_tests_for_simulator

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
