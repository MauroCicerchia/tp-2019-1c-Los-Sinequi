#!/bin/bash
cd

#------------<COMMONS>----------------------

if test -d /home/utnso/so-commons-library; then
    cd /home/utnso/so-commons-library/
    sudo make uninstall
    cd ..
    sudo rm -r ./so-commons-library
fi

cd

git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library/
sudo make install

cd

#------------</COMMONS>----------------------

# ------------<BASE>---------------------------------
# directorio para prueba de Base
if test -d /home/utnso/lfs-base/; then
	sudo rm -r /home/utnso/lfs-base
fi

cd
mkdir -p /home/utnso/lfs-base
cd /home/utnso/lfs-base/
mkdir Metadata
mkdir Blocks
mkdir Tables

cd Metadata/
echo -e "BLOCK_SIZE=64\nBLOCKS=4096\nMAGIC_NUMBER=LISSANDRA" >> Metadata.bin

cd 

# Copio carpeta tp

if test -d /home/utnso/workspace/tp-2019-1c-Los-Sinequi/; then
	sudo rm -r /home/utnso/workspace/tp-2019-1c-Los-Sinequi
fi

cd /home/utnso/tp-safe-zone/

cp -r tp-2019-1c-Los-Sinequi/ /home/utnso/workspace/tp-2019-1c-Los-Sinequi

# Build sharedLib

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi/sharedLib/Debug/

make all

# Creo Memorias

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi/Memory/Debug/

make all

cd ../../

cp Memory/ Memory2

# Config Memoria

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi/Memory/

if test -e .config; then
	rm .config
fi

echo -e "PUERTO=\nPUERTO_FS=\nIP_SEEDS=[]\nPUERTO_SEEDS=[]\nRETARDO_MEM=0\nRETARDO_FS=0\nTAM_MEM=1280\nRETARDO_JOURNAL=60000\nRETARDO_GOSSIPING=10000\nMEMORYNUMBER=1\n" >> .config

cd ../Memory2/

if test -e .config; then
	rm .config
fi

echo -e "PUERTO=\nPUERTO_FS=\nIP_SEEDS=[]\nPUERTO_SEEDS=[]\nRETARDO_MEM=0\nRETARDO_FS=0\nTAM_MEM=1280\nRETARDO_JOURNAL=60000\nRETARDO_GOSSIPING=10000\nMEMORYNUMBER=2\n" >> .config

# Config FS

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi/FileSystem/Debug/

make all

cd ../Config/

if test -e .config; then
	rm .config
fi

echo -e "PUERTO_ESCUCHA=\nIP=\nPUNTO_MONTAJE=/home/utnso/lfs-base/\nRETARDO=0\nTAMAÑO_VALUE=60\nTIEMPO_DUMP=20000\n" >> .config

# Config Kernel

cd /home/utnso/workspace/tp-2019-1c-Los-Sinequi/Kernel/Debug/

make all

cd ../

if test -e .config; then
	rm .config
fi

echo -e "MEM_IP=\nMEM_PORT=\nQUANTUM=3\nMULT_DEGREE=1\nMD_REFRESH_RATE=15000\nEXEC_DELAY=100\nGOSSIP_DELAY=10000\n" >> .config

# -------------</BASE>---------------------------------

cd

