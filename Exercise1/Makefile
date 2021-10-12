CXX=g++
CXXFLAGS=-std=c++17

SRCS=main.cpp pugixml.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: Exercise1

Exercise1: $(OBJS)
	$(CXX) $(LDFLAGS) -o Exercise1 $(OBJS) $(LDLIBS)