CXX = g++
CXXFLAGS = -Wall -std=c++17 -I.  # -I. tells the compiler to look for headers current directory

SRCS = miniX86.cpp x86_registers.cpp register.cpp register_map.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = x86simulator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp simulator_utils.h x86_registers.h register.h register_map.h 
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
