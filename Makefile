CXXFLAGS = -Wall -Wextra -std=c++17 -O2
CXXFLAGS += $(shell pkg-config ncurses --cflags)
LDLIBS = $(shell pkg-config ncurses --libs)
OUTFILE = hexit
OBJFILE = hexit.o
CXX = g++

hexit: hexit.o
	$(CXX) -o $(OUTFILE) $(OBJFILE) $(LDLIBS)

hexit.o: hexit.cc
	$(CXX) $(CXXFLAGS) -c hexit.cc -o $(OBJFILE)

clean:
	rm $(OUTFILE) $(OBJFILE)
