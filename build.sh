#!/usr/bin/sh


if test -d build; then
    echo "Directory exists."
else
    mkdir build
fi


cd build

cmake ../

make -j20


if [ -z "$1" ]
  then
    scp tester root@192.168.4.33:/usr/local/bin
    scp ../*.qml root@192.168.4.33:/usr/local/share
fi

