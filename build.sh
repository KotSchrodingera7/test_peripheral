#!/usr/bin/sh


if test -d build; then
    echo "Directory exists."
else
    mkdir build
fi


cd build

cmake ../

make -j20


if [ -n "$1" ]; then
    scp expo_view root@$1:/usr/bin
    scp ../qml/*.qml root@$1:/usr/local/share/qml
    scp ../icons/*.png ../icons/*.svg root@$1:/usr/local/share/icons
fi

