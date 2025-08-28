CXX = g++
CXXFLAGS = -g -Wall -std=c++17 -I.

# List all source files, excluding the linker flag
SRCS = x86_registers.cpp register.cpp register_map.cpp string_utils.cpp parser_utils.cpp memory.cpp register_enums.cpp main.cpp x86_simulator_display.cpp x86_simulator_input.cpp x86_simulator_private_helpers.cpp x86_simulator_state.cpp x86_simulator_core.cpp register_rflags.cpp DatabaseManager.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = x86simulator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) -lncursesw -lpqxx -lpq

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
