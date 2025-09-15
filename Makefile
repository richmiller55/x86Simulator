# Define compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g
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
	CodeGenerator.cpp

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
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)


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