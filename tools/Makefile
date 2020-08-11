CXXFLAGS=-Wall -std=c++17 -ggdb

all: spec

spec: spec.cpp
	$(CXX) $(CXXFLAGS) `pkg-config --cflags libxml-2.0` -o spec spec.cpp `pkg-config --libs libxml-2.0`
