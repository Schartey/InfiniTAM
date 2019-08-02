#!/bin/sh

cmake /app
make

chmod +x ./Apps/InfiniTAM/InfiniTAM

export LIBGL_ALWAYS_INDIRECT=1

./Apps/InfiniTAM/InfiniTAM $1 $2 $3
