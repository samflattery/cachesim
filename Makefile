CC = gcc
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -Wpedantic
DEPS = cache.h
OBJDIR = build
vpath %.h src
vpath %.cpp src
OBJ = $(addprefix $(OBJDIR)/, cache.o cachesim.o)

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
