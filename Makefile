CXX = g++
CFLAGS = -Wall -Werror
CFLAGS += -g
#CFLAGS += -O3
LIBS    =

CORE = bdfe
OBJS = main.o ossd_i2c.o pi2c.o bdf.o rterm.o
HFILES = Makefile pi2c.h ossd_i2c.h rterm.h font88.h font816.h
CFILES = ossd_i2c.c bdf.c rterm.c main.c 

all: $(CORE)

$(CORE): $(OBJS) $(CFILES) $(HFILES)
	$(CXX) $(CFLAGS) -o $(CORE) $(OBJS) $(LIBS)

clean:
	rm -f $(CORE)
	rm -f *.o

%.o: %.c $(HFILES)
	$(CXX) -c $(CFLAGS) $< -o $@


