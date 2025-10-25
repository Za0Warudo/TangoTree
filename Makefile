CXX = g++
CXXFLAGS = -Wall -std=c++20 -g -MMD -MP
TARGETS = RedBlackTree TangoTree
SRCS = RedBlackTree.cpp TangoTree.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
all: $(TARGETS)

RedBlackTree: RedBlackTree.o
	$(CXX) $< -o $@

TangoTree: TangoTree.o
	$(CXX) $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean all

clean:
	rm -f $(OBJS) $(TARGETS) $(DEPS)

-include $(DEPS)