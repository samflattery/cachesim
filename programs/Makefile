
CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -Wpedantic

.PHONY: all
all: $(patsubst %.cpp, %.o, $(wildcard *.cpp))

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o
