CXX = g++
CXXFLAGS = -g -Wall -std=c++17 -I.  # -I. tells the compiler to look for headers current directory

SRCS = miniX86.cpp x86_registers.cpp register.cpp register_map.cpp string_utils.cpp parser_utils.cpp memory.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = x86simulator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp simulator_utils.h x86_registers.h register.h register_map.h string_utils.h parser_utils.h x86_simulator memory.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
