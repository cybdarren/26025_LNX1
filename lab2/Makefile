all: lab2.cpp
	$(CXX) $(CXXFLAGS) `pkg-config libegt --cflags --libs` -Wno-psabi -Og lab2.cpp -g -o lab2.bin

clean:
	rm -rf lab2.bin
	