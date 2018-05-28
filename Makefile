CPP = g++ -std=c++11
CPPFLAGS = -g -O0 -Wall `pkg-config gtkmm-3.0 --cflags`
COMPC = $(CPP) $(CPPFLAGS) $^ -c
COMPG = $(CPP) $(CPPFLAGS) $^ `pkg-config gtkmm-3.0 --libs` -o $@

complex_drawer : complex_drawer.o parser.o complex_drawer_gtk.o
	$(COMPG) 

parser : parser.o
	$(COMPC) 
