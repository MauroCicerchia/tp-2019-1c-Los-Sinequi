#!/bin/bash
cd

if test ! -d /home/utnso/so-commons-library; then
        git clone https://github.com/sisoputnfrba/so-commons-library.git
        cd so-commons-library/
        sudo make install
fi

cd

if test ! -d /home/utnso/lissandra-checkpoint/; then
	mkdir /home/utnso/lissandra-checkpoint/
fi

if test ! -d /home/utnso/lissandra-checkpoint/Metadata/; then
    mkdir /home/utnso/lissandra-checkpoint/Metadata/
fi

if test ! -d /home/utnso/lissandra-checkpoint/Metadata/metadata.bin; then
    touch /home/utnso/lissandra-checkpoint/Metadata/metadata.bin
fi

if test ! -d /home/utnso/lissandra-checkpoint/Tables/; then
    mkdir /home/utnso/lissandra-checkpoint/Tables/
fi

if test ! -d /home/utnso/lissandra-checkpoint/Blocks/; then
    mkdir /home/utnso/lissandra-checkpoint/Blocks/
fi

if test ! -d /home/utnso/workspace; then
    mkdir /home/utnso/workspace
fi

mv /home/utnso/tp-2019-1c-Los-Sinequi/  /home/utnso/workspace

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi/sharedLib/Debug
    make clean
    make all

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi/FileSystem/Debug
    make clean
    make all

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi/Kernel/Debug
    make clean
    make all

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi/Memory/Debug
    make clean
    make all

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi
