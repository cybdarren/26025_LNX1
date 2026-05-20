all: lab4.cpp
	$(CXX) $(CXXFLAGS) `pkg-config libegt --cflags --libs` -Wno-psabi -Og lab4.cpp -g -o lab4.bin

clean:
	rm -rf lab4.bin
	