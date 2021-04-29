#!/usr/bin/env bash

if [ ! -f $1 ]; then
  echo -e "\033[1;31mCould not find dependency file '$1'.  Run \"make install\" to fix.\033[0m"
fi
