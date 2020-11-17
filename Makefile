CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -Wpedantic
DEPS = cache_block.h mesi_block.h directory.h cache.h interconnect.h
OBJDIR = build
vpath %.h src
vpath %.cpp src
OBJ = $(addprefix $(OBJDIR)/, mesi_block.o cache.o interconnect.o directory.o cachesim.o)

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
