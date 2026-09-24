# Define include headers to install in $(prefix)/include
include_HEADERS = shakeout2.h

# Define executable binaries to install in $(prefix)/bin
bin_PROGRAMS = shakeout2_query

# Define shared/static libraries to install in $(prefix)/lib
lib_LTLIBRARIES = libshakeout2.la

# ------------------------------------------------------------------------------
# Library Definition
# ------------------------------------------------------------------------------

# Sources for the shared/static library
libshakeout2_la_SOURCES = \
	shakeout2.c \
	shakeout2_dataset.c \
	shakeout2_util.c \
	um_netcdf.c \
	cJSON.c

# Include path and flags for compiling the library
libshakeout2_la_CPPFLAGS = -I$(top_srcdir) -I$(prefix)/include -DDYNAMIC_LIBRARY

# Linker flags and libraries for libshakeout2
libshakeout2_la_LDFLAGS = -version-info 1:0:0 -L$(prefix)/lib
libshakeout2_la_LIBADD = -lm $(LIBS)

# ------------------------------------------------------------------------------
# Executables Definitions
# ------------------------------------------------------------------------------

AM_CPPFLAGS = -I$(top_srcdir) -I$(prefix)/include
AM_LDFLAGS = -L$(prefix)/lib

# shakeout2_query links against our Libtool library
shakeout2_query_SOURCES = shakeout2_query.c
shakeout2_query_LDADD = libshakeout2.la -lm $(LIBS)

# ------------------------------------------------------------------------------
# macOS .so compatibility symlink
# ------------------------------------------------------------------------------
install-exec-hook:
	@if test "`uname -s`" = "Darwin"; then \
		cd $(DESTDIR)$(libdir) && \
		rm -f libshakeout2.so && \
		ln -s libshakeout2.dylib libshakeout2.so; \
	fi
