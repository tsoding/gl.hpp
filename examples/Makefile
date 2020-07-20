PKGS=glfw3 gl
CXXFLAGS=-Wall -Wno-missing-braces -I.. -std=c++17 `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)`

tiles: main.cpp ../gl.hpp
	$(CXX) $(CXXFLAGS) -o tiles -ggdb main.cpp $(LIBS)
