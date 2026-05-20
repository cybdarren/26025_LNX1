all: lab3.cpp
	$(CXX) $(CXXFLAGS) `pkg-config libegt --cflags --libs` -Wno-psabi -Og lab3.cpp -g -o lab3.bin

clean:
	rm -rf lab3.bin
	