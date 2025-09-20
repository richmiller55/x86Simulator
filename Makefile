# Define compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -mavx
LDFLAGS = 

# Define the target executable name
TARGET = x86simulator

# Define include directories
INCLUDES = -I../libpqxx/include

# Define C++ source files
OBJ = main.o string_utils.o parser_utils.o memory.o x86_simulator_input.o x86_simulator_private_helpers.o x86_simulator_state.o x86_simulator_core.o register_rflags.o register_map.o DatabaseManager.o ui_manager.o decoder.o CodeGenerator.o instruction_describer.o program_decoder.o

TEST_OBJ = tests/test_main.o tests/rflags_test.o tests/memory_test.o tests/register_map_test.o tests/decoder_test.o tests/instruction_describer_test.o tests/operand_parser_test.o program_decoder_test.o
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = x86_decoder_test

# Add gtest flags
GTEST_LIBS = -lgtest -lgtest_main -pthread

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(filter-out main.o,$(OBJS)) $(TEST_OBJS) $(LDFLAGS) $(LIBS) $(GTEST_LIBS)
