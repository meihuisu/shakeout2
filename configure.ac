dnl -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(SHAKEOUT2, 1.1.0, software@scec.org)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign tar-pax no-exeext])

dnl
dnl Dependencies
dnl

dnl
dnl Command line tools
dnl

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_LN_S

dnl --- begin: add libtool and --enable-shared handling ---
# initialize libtool (provides LT_INIT and shared/static helpers)
# If libtool is not available, autoreconf will tell you.
LT_INIT([shared static])

# standard --enable-shared/--disable-shared option
AC_ARG_ENABLE([shared],
  [AS_HELP_STRING([--enable-shared|--disable-shared],
    [build shared libraries (default: enabled)])],
  [case "${enableval}" in
     yes) enable_shared=yes ;;
     no) enable_shared=no ;;
     *) enable_shared=${enableval} ;;
   esac],
  [enable_shared=yes])

# export the variable for Makefile.in / Automake
AC_SUBST([enable_shared])
dnl --- end ---

dnl TAR
AC_PATH_PROG(TAR, tar)
if test -z "$TAR" ; then
  AC_MSG_FAILURE([cannot find 'tar' program.])
  TAR=`echo "Error: tar is not installed." ; false`
fi

dnl netcdf
AC_ARG_ENABLE([netcdf],
    [AC_HELP_STRING([--enable-netcdf],
        [install netCDF library @<:@default=yes@:>@])],
        [if test "$enableval" = yes ; then install_netcdf=yes; else install_netcdf=no; fi],
        [install_netcdf=yes])
AM_CONDITIONAL([INSTALL_NETCDF], [test "$install_netcdf" = yes])
AC_ARG_WITH([netcdf-incdir],
  [AS_HELP_STRING([--with-netcdf-incdir], [directory containing netCDF header files])],
  [with_netcdf_incdir="$withval"],
  [with_netcdf_incdir=no])
AC_SUBST(with_netcdf_incdir)
AC_ARG_WITH([netcdf-libdir],
  [AS_HELP_STRING([--with-netcdf-libdir], [directory containing netCDF library])],
  [with_netcdf_libdir="$withval"],
  [with_netcdf_libdir=no])
AC_SUBST(with_netcdf_libdir)

dnl hdf5
AC_ARG_ENABLE([hdf5],
    [AC_HELP_STRING([--enable-hdf5],
        [install HDF5 library @<:@default=yes@:>@])],
        [if test "$enableval" = yes ; then install_hdf5=yes; else install_hdf5=no; fi],
        [install_hdf5=yes])
AM_CONDITIONAL([INSTALL_HDF5], [test "$install_hdf5" = yes])
AC_ARG_WITH([hdf5-incdir],
  [AS_HELP_STRING([--with-hdf5-incdir], [directory containing HDF5 header files])],
  [with_hdf5_incdir="$withval"],
  [with_hdf5_incdir=no])
AC_SUBST(with_hdf5_incdir)
AC_ARG_WITH([hdf5-libdir],
  [AS_HELP_STRING([--with-hdf5-libdir], [directory containing HDF5 library])],
  [with_hdf5_libdir="$withval"],
  [with_hdf5_libdir=no])
AC_SUBST(with_hdf5_libdir)

dnl
dnl Setup environment so dependencies are used in build
dnl

dnl NETCDF (required)
if test "$install_netcdf" = yes ; then
  NETCDF_INCLUDES=""
  NETCDF_LDFLAGS="-lnetcdf"
else    
  SCEC_NETCDF_HEADER
  SCEC_NETCDF_LIB
fi
if test "$with_netcdf_incdir" != no ; then
  NETCDF_INCLUDES="-I$with_netcdf_incdir"
fi
if test "$with_netcdf_libdir" != no ; then
  NETCDF_LDFLAGS="-L$with_netcdf_libdir -lnetcdf"
fi

dnl HDF5 (required)
if test "$install_hdf5" = yes ; then
  HDF5_INCLUDES=""
  HDF5_LDFLAGS="-lhdf5"
else    
  SCEC_HDF5_HEADER
  SCEC_HDF5_LIB
fi
if test "$with_hdf5_incdir" != no ; then
  HDF5_INCLUDES="-I$with_hdf5_incdir"
fi
if test "$with_hdf5_libdir" != no ; then
  HDF5_LDFLAGS="-L$with_hdf5_libdir -lhdf5"
fi


CFLAGS="$CFLAGS"
CPPFLAGS="-I$prefix/include $NETCDF_INCLUDES $HDF5_INCLUDES $CPPFLAGS"
LDFLAGS="-L$prefix/lib -L$prefix/lib64 $NETCDF_LDFLAGS $HDF5_LDFLAGS $LDFLAGS"


dnl
dnl Verify configuration
dnl

##check optional large data path
##SHAKEOUT2_LARGEDATA_DIR=$CVM_LARGEDATA_DIR/model/shakeout2
if test x"$CVM_LARGEDATA_DIR" != x; then
   # test directory existence
   SHAKEOUT2_LARGEDATA_DIR=$CVM_LARGEDATA_DIR/model/shakeout2
# In docker container building.. this is not accessible  yet ???
   if test x"$CVM_IN_DOCKER" != x; then
     AM_CONDITIONAL([WITH_SHAKEOUT2_LARGEDATA_DIR], true)
     AC_SUBST(SHAKEOUT2_LARGEDATA_DIR)
   else 
     AC_CHECK_FILE([$SHAKEOUT2_LARGEDATA_DIR/model_CVMSI_taper_so2.nc],
                [AM_CONDITIONAL([WITH_SHAKEOUT2_LARGEDATA_DIR], true) AC_SUBST(SHAKEOUT2_LARGEDATA_DIR)],
                [AM_CONDITIONAL([WITH_SHAKEOUT2_LARGEDATA_DIR], false)])
   fi
else
   AM_CONDITIONAL(WITH_SHAKEOUT2_LARGEDATA_DIR, false)
fi

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  data/Makefile
  test/Makefile
  ])

AC_OUTPUT

AM_CONDITIONAL([BUILD_SHARED_LIBRARY], [test "$enable_shared" = yes])
if test "$enable_shared" = yes; then
  AC_MSG_NOTICE([Building shared libraries. Shared libraries enabled.])
else
  AC_MSG_WARN([Building static libraries. Shared libraries are not enabled.])
fi


dnl End of file
