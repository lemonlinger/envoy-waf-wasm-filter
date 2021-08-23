#!/bin/sh

docker run -e uid="$(id -u)" -e gid="$(id -g)" -v $PWD:/work -w /work wasmsdk:v2 /build_wasm.sh
