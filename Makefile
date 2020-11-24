CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -Wpedantic
DEPS = numa_node.h cache_block.h mesi_block.h directory.h cache.h interconnect.h
OBJDIR = build
vpath %.h src
vpath %.cpp src
OBJ = $(addprefix $(OBJDIR)/, numa_node.o mesi_block.o cache.o interconnect.o directory.o cachesim.o)

# Default build rule
.PHONY: all
all: cachesim

cachesim: $(OBJ)
	$(CXX) $(CXXFLAGS) -o cachesim $(OBJ)

$(OBJDIR)/%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o cachesim a.out
