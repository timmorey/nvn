##
# Makefile - Created by Timothy Morey on 1/7/2013
#
# This is the makefile for the nvn application.
#

CC=mpicc
CXX=mpicc
LD=mpicc

CFLAGS += -MD -MP -g -I$(PNETCDF_INC)
CPPFLAGS += -g -I$(PNETCDF_INC)
LDFLAGS += -g -L$(PNETCDF_LIB)
LDLIBS = -lpnetcdf -lGL -lGLU -lpthread -lX11

CSRC = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
OBJS = $(patsubst %.c,%.o,$(CSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

nvn: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(CPPSRC:%.cpp=%.d)

-include $(CSRC:%.c=%.d)

clean:
	rm -v -f nvn *.d *.o *~ *.gch
