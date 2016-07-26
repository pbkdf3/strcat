#!/bin/bash

# XXX need a Makefile prior to release
# setup environment
export MAPR_HOME=/opt/mapr

GCC_OPTS="-std=c99 \
 -Wl,--allow-shlib-undefined -I. -I${MAPR_HOME}/include \
-L${MAPR_HOME}/lib -lMapRClient -L${MAPR_HOME}/lib"


# to enable linker to work (??)
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MAPR_HOME}/lib
export LD_RUN_PATH=${LD_RUN_PATH}:${MAPR_HOME}/lib

# compile, link and optionally install ;)
gcc ${GCC_OPTS} strcat.c -o strcat
#cp strcat ~/bin
