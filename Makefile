CXX = g++
CXXFLAGS = -g -Wall -std=c++17 -I.  # -I. tells the compiler to look for headers current directory

SRCS_1 = x86_registers.cpp register.cpp register_map.cpp string_utils.cpp
SRCS_2 = parser_utils.cpp memory.cpp register_enums.cpp main.cpp
SRCS_3 = x86_simulator_display.cpp x86_simulator_input.cpp x86_simulator_private_helpers.cpp
SRCS_4 = x86_simulator_state.cpp x86_simulator_core.cpp
SRCS = $(SRCS_1) $(SRCS_2) $(SRCS_3) $(SRCS_4) # Corrected line to combine the lists
OBJS = $(SRCS:.cpp=.o)
TARGET = x86simulator

all: $(TARGET)


# Grouping header files into logical sets
HEADERS_CORE = x86_registers.h register.h register_map.h string_utils.h x86_simulator.h
HEADERS_UTIL = simulator_utils.h parser_utils.h string_utils.h
HEADERS_MEM = memory.h
HEADERS_OPS = register_nums.h operand_types.h operand.h

# Combine all header lists into a single variable for the pattern rule
ALL_HEADERS = $(HEADERS_CORE) $(HEADERS_UTIL) $(HEADERS_MEM) $(HEADERS_OPS)




$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)
%.o: %.cpp $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
