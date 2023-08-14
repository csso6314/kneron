#!/bin/bash
function fail {
    printf '%s\n' "$1"
    exit "${2-1}"
}

build_type=$1
if [[ -z $build_type ]];then
    build_type=Debug
elif [[ $build_type == "debug" ]];then
    build_type=Debug
elif [[ $build_type == "release" ]];then
    build_type=Release
else
    fail "[ERROR] wrong argument! Usage: bulid.sh [debug|release]"
fi

echo Start to compile ...

if [ -d "./build" ];then
    rm -rf ./build
fi
mkdir build
cd build
if [[ -z ${MSYSTEM} ]]; then
    cmake ../../../../platform/kl720/ncpu -DCMAKE_TOOLCHAIN_FILE=../xt-toolchain-for-720.cmake -DCMAKE_BUILD_TYPE=$build_type
    status=$?
else
    cmake ../../../../platform/kl720/ncpu -DCMAKE_TOOLCHAIN_FILE=../xt-toolchain-for-720.cmake -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE=$build_type
    status=$?
fi

[ $status -eq 0 ] && echo "[INFO] cmake done" || fail "[ERROR] cmake failed"

make -j
status=$?
[ $status -eq 0 ] && echo "[INFO] compile done" || fail "[ERROR] compile failed"


echo copying fw_ncpu.bin to res/firmware/KL720
cp -f ./ncpu_main/fw_ncpu.bin ../../../../../../../res/firmware/KL720/

echo [Done]

