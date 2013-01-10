##
# Makefile - Created by Timothy Morey on 1/7/2013
#
# This is the makefile for the nvn application.
#

CC=mpicc
LD=mpicc

CFLAGS += -MD -MP -g -I$(SDL_INC) -I$(PNETCDF_INC)
LDFLAGS += -g -L$(SDL_LIB) -L$(PNETCDF_LIB)
LDLIBS = -lSDL -lpnetcdf

SRC = $(wildcard *.c)

nvn: $(SRC:%.c=%.o)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(SRC:%.c=%.d)

clean:
	rm -v -f nvn *.d *.o *~ *.gch
