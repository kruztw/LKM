#!/bin/bash

set -x

if [ "$1" == "init" ];then
    rm rootfs -rf || true
    mkdir -p rootfs
    cp *cpio* rootfs
    cd rootfs
    gzip -d *cpio* || true
    cpio -idmv < *cpio*
elif [ "$1" == "run" ];then
    if [ -f "./ans.c" ];then
        gcc ans.c -o ans -no-pie -ggdb --static -lpthread -masm=intel || exit
        cp ans rootfs/
    fi

    if [ -f "simple.ko" ];then
        cp ./simple.ko rootfs/
    fi

    cd rootfs
    find . | cpio -o --format=newc > ../local.cpio
    cd ..
    gzip local.cpio -f
    ./run.sh
fi

