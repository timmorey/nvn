#######################################
# The list of executables we are building seperated by spaces
# the 'bin_' indicates that these build products will be installed
# in the $(bindir) directory. For example /usr/bin
bin_PROGRAMS=nvn

#######################################
# Build information for each executable. The variable name is derived
# by use the name of the executable with each non alpha-numeric character is
# replaced by '_'. So a.out becomes a_out and the appropriate suffex added.
# '_SOURCES' for example.

# Sources for the a.out 
nvn_SOURCES= nvn.cpp

# Linker options for a.out
nvn_LDFLAGS = $(top_srcdir)/libnvn/libnvn.la

# Compiler options for a.out
nvn_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/libnvn
