
CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -Wpedantic

.PHONY: all
all:
	$(CXX) $(CXXFLAGS) -lpthread better_spinlock.cpp -o better_spin
	$(CXX) $(CXXFLAGS) -lpthread spinlock.cpp -o spin
	$(CXX) $(CXXFLAGS) -lpthread add.cpp -o add
	$(CXX) $(CXXFLAGS) -lpthread sort.cpp -o sort

.PHONY: clean
clean:
	rm -f better_spin spin add sort
