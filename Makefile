# Attribution: paraphrased from Makefile shared by Paul Zhang on Campuswire.

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Weffc++ -pedantic -g \
		   -fsanitize=address -fsanitize=undefined
OUTFILE = memory_driver

all: debug

debug: *.cpp *.h
	$(CXX) $(CXXFLAGS) *.cpp -o $(OUTFILE)

build: *.cpp *.h
	$(CXX) *.cpp -o $(OUTFILE)

clean:
	rm -f $(OUTFILE)
