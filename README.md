> [!CAUTION]
> **THIS REPOSITORY IS FOR NetCDF VERSION**
>
>

# The SHAKEOUT2 Velocity Model (based on cvmsi/taper)

<a href="https://github.com/sceccode/shakeout2.git"><img src="https://github.com/sceccode/shakeout2/wiki/images/shakeout2_logo.png"></a>

[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
![GitHub repo size](https://img.shields.io/github/repo-size/sceccode/shakeout2)
[![shakeout2-ucvm-ci Actions Status](https://github.com/SCECcode/shakeout2/workflows/shakeout2-ucvm-ci/badge.svg)](https://github.com/SCECcode/shakeout2/actions)

blah blah

## Installation

This package is intended to be installed as part of the UCVM framework,
version 25.7 or higher. 

## Contact the authors

If you would like to contact the authors regarding this software,
please e-mail software@scec.org. Note this e-mail address should
be used for questions regarding the software itself (e.g. how
do I link the library properly?). Questions regarding the model's
science (e.g. on what paper is the SHAKEOUT2 based?) should be directed
to the model's authors, located in the AUTHORS file.

## To build in standalone mode

To install this package on your computer, please run the following commands:

<pre>
  libtoolize --copy --force
  aclocal -I m4
  autoconf
  automake --add-missing --force-missing
  ./configure --prefix=/dir/to/install
  make
  make install
</pre>

<pre>
example:

./configure --prefix=$UCVM_INSTALL_PATH --enable-shared CPPFLAGS='-I$UCVM_INSTALL_PATH/lib/hdf5/include -I$UCVM_INSTALL_PATH/lib/netcdf/include' LDFLAGS='-L$UCVM_INSTALL_PATH/lib/hdf5/lib -L$UCVM_INSTALL_PATH/lib/netcdf/lib -Wl,-rpath,$UCVM_INSTALL_PATH/lib/hdf5/lib -Wl,-rpath,$UCVM_INSTALL_PATH/lib/netcdf/lib' LIBS='-lhdf5 -lnetcdf'
</pre>

## Note

### shakeout2_query
