#!/usr/bin/env bash

echo "Benching CPP"
\time -f "Program: %C\nTotal time: %E\nUser Mode (s) %U\nKernel Mode (s) %S\nCPU: %P" ./build/release/search > /dev/null

echo "
--------
"

echo "Benching Python"
\time -f "Program: %C\nTotal time: %E\nUser Mode (s) %U\nKernel Mode (s) %S\nCPU: %P" python3 bench.py > /dev/null

