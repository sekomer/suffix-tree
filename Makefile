# Compiler
CC := g++

# Compiler flags
CFLAGS :=  -Iinclude -std=c++11

# Source files
SRCS := $(wildcard src/*.cpp)

# Object files
OBJS := $(SRCS:.cpp=.o)

# Executable name
TARGET := tree

# Build rule
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Object file rule
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Run rule
run: $(TARGET)
	./$(TARGET)

# Debug rule
debug: $(TARGET)
	gdb ./$(TARGET)