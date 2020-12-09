
CC = gcc
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wshadow -Wpedantic -lpthread

.PHONY: all
all: $(patsubst %.cpp, %.out, $(wildcard *.cpp))

%.out: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.out
