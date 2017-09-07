CFLAGS =
DEFINES =
INCLUDES = 
LIBS =
CC=g++

.cc.o:
	$(CC) -c $(CFLAGS) $(DEFINES) $(INCLUDES) $<

all:: mc_int

gnuplot_i.o: gnuplot_i.hpp
dmc_int.o:	mc_int.cpp

dmc_int: dmc_int.o
	$(CC) -o $@ $(CFLAGS) $(EXAMPLE) $(LIBS)

clean: 
	rm -f mc_int.o mc_int
