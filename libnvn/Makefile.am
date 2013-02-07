#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libnvn.la

#######################################
# Build information for each library

# Sources for libnvn
libnvn_la_SOURCES = \
	color-ramp.c \
	communication-queue.c \
	DataGrid.cpp \
	gl-window.cpp \
	layer.cpp \
	Loader.cpp \
	model.cpp \
	nvn.cpp \
	variant.c

# Linker options libnvn
libnvn_la_LDFLAGS = -lpnetcdf -lGL -lGLU -lpthread -lX11 -lxml2

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libnvn_la_CPPFLAGS = -I$(top_srcdir)/include

