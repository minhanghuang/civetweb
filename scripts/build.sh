#!/bin/bash

function main() {
  echo "cyclone build."
  mkdir -p build && cd build
  cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug ..
  make -j4
}

main "$@"

