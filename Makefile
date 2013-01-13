##
# Makefile - Created by Timothy Morey on 1/7/2013
#
# This is the makefile for the nvn application.
#

CC=mpicc
CXX=mpicc
LD=mpicc

INCLUDES = -I$(PNETCDF_INC) -I/usr/include/libxml2
CFLAGS += -g $(INCLUDES)
CPPFLAGS += -g $(INCLUDES)
LDFLAGS += -g -L$(PNETCDF_LIB)
LDLIBS = -lpnetcdf -lGL -lGLU -lpthread -lX11 -lxml2

CSRC = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
OBJS = $(patsubst %.c,%.o,$(CSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

nvn: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(CPPSRC:%.cpp=%.d)

-include $(CSRC:%.c=%.d)

clean:
	rm -v -f nvn *.d *.o *~ *.gch
