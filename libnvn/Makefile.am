#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libnvn.la

#######################################
# Build information for each library

# Sources for libnvn
libnvn_la_SOURCES = \
  CartesianCRS.cpp \
	color-ramp.c \
	communication-queue.c \
	CRS.cpp \
	DataGrid.cpp \
	GLWindow.cpp \
	GLX.cpp \
	GridCRS.cpp \
	GridTransform.cpp \
	Loader.cpp \
	Model.cpp \
	nvn.cpp \
	Plot2DLayer.cpp \
	ReferenceFrameLayer.cpp \
	ScreenCRS.cpp \
	ShadedSurfaceLayer.cpp \
	variant.c \
	ViewTransform.cpp 

# Linker options libnvn
libnvn_la_LDFLAGS = -lpnetcdf -lGL -lGLU -lpthread -lX11 -lxml2

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libnvn_la_CPPFLAGS = -I$(top_srcdir)/include

