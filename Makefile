# Makefile for Doomnite iOS Game
# Builds static library from C++ sources

CXX = clang++ -std=c++17
CXXFLAGS = -Wall -Wno-deprecated-declarations -arch arm64 -isysroot $(xcrun --show-sdk-path --sdk iphoneos)

SOURCES = doomnite_final.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: libdoomnite.a

libdoomnite.a: $(OBJECTS)
libtool -static -o $@ $^
@echo "=== LIBRARY BUILT! ==="

%.o: %.cpp
$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
rm -f $(OBJECTS) libdoomnite.a

.PHONY: all clean
