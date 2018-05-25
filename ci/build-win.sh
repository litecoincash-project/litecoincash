#!/bin/bash
if [ -f ./.fullbuild ]; then
        echo "Running full Windows build";
        make clean;
        ./autogen.sh && CONFIG_SITE=$PWD/depends/x86_64-w64-mingw32/share/config.site ./configure --prefix=/;
fi
make -j 24 && strip src/qt/litecoincash-qt.exe
