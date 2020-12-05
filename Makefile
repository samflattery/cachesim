CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -Wpedantic
DEPS = latencies.h protocols.h numa_node.h cache_block.h moesi_block.h mesi_block.h msi_block.h directory.h cache.h interconnect.h
OBJDIR = build
vpath %.h src
vpath %.cpp src
OBJ = $(addprefix $(OBJDIR)/, numa_node.o moesi_block.o mesi_block.o msi_block.o cache.o interconnect.o directory.o cachesim.o)

# Default build rule
.PHONY: all
all: cachesim

.PHONY: programs
programs:
	(cd programs && make)

cachesim: $(OBJ)
	$(CXX) $(CXXFLAGS) -o cachesim $(OBJ)

$(OBJDIR)/%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o cachesim a.out
	(cd programs && make clean)
