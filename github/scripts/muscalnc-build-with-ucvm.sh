#!/bin/bash
set -e # Exit on any command failure

if [ -z "$UCVM_INSTALL_PATH" ]; then
  echo "ERROR: UCVM_INSTALL_PATH environment variable is not set."
  exit 1
fi

tmp="$(uname -s)"

if [ "$tmp" = "Darwin" ]; then
  eval "$(/opt/homebrew/bin/brew shellenv 2>/dev/null || /usr/local/bin/brew shellenv 2>/dev/null)"
  brew install automake libtool gcc
  export PATH="/opt/homebrew/opt/libtool/libexec/gnubin:$PATH"

  ## Create a virtual environment to resolve PEP 668 pip errors on macOS
  python3 -m venv .venv
  source .venv/bin/activate

  python3 -m pip install --upgrade pip 
  python3 -m pip install scipy h5py numpy pandas pybind11 netCDF4
else 
  pip install scipy h5py numpy pandas pybind11 netCDF4
fi

## actual build
libtoolize
aclocal -I m4
autoconf
automake --add-missing --force-missing

./configure --prefix=$UCVM_INSTALL_PATH/model/muscalnc --enable-shared --with-hdf5-libdir=$UCVM_INSTALL_PATH/lib/hdf5/lib --with-hdf5-incdir=$UCVM_INSTALL_PATH/lib/hdf5/include --with-netcdf-libdir=$UCVM_INSTALL_PATH/lib/netcdf/lib --with-netcdf-incdir=$UCVM_INSTALL_PATH/lib/netcdf/include

make
make install

