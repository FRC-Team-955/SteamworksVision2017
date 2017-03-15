#!/bin/bash
mkdir ../release
cd ../release
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
