#!/bin/bash
if [ -f ./.fullbuild ]; then
        echo "Running full Linux build";
        make clean;
        ./autogen.sh && CONFIG_SITE=$PWD/depends/x86_64-pc-linux-gnu/share/config.site ./configure --prefix=/;
fi
make -j 24 && strip src/qt/neon-qt
