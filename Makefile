# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -fexceptions -g -Wextra -D_GLIBCXX_USE_CXX11_ABI=1 -I/opt/homebrew/opt/openssl@3/include

# OpenSSL libraries (added extra linking flags)
LIBS = -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -ldl -lz -pthread

# Target executable
TARGET = imapcl 

# Source files
SRCS = main.cpp connect.cpp imap.cpp

# Object files (generated from source files)
OBJS = $(SRCS:.cpp=.o)

# Header files (optional: for dependency generation)
HDRS = connect.h imap.h

# Default target to build the executable
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Rule to compile .cpp files into .o object files
%.o: %.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove the generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets (do not treat them as files)
.PHONY: all clean
