CXXFLAGS = -Wall -Wextra -std=c++20 -O2
CXXFLAGS += $(shell pkg-config ncurses --cflags)
LDLIBS = $(shell pkg-config ncurses --libs)
OUTFILE = hexit
OBJFILES = hexit.o terminal_window.o
CXX = g++

hexit: $(OBJFILES)
	$(CXX) -o $(OUTFILE) $(OBJFILES) $(LDLIBS)

hexit.o: hexit.cc
	$(CXX) $(CXXFLAGS) -c hexit.cc -o hexit.o

terminal_window.o: terminal_window.cc terminal_window.h data_buffer.hpp
	$(CXX) $(CXXFLAGS) -c terminal_window.cc -o terminal_window.o

clean:
	rm -f $(OUTFILE) $(OBJFILES)
