PKGS=glfw3 gl
CXXFLAGS=-Wall -I.. `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)`

tiles: main.cpp
	$(CXX) $(CXXFLAGS) -o tiles -ggdb main.cpp $(LIBS)
