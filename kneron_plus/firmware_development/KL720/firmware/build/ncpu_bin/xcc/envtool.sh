#!/bin/bash
os=""
if [[ -z ${MSYSTEM} ]]; then
    os=linux
    export XTENSA_INST=/usr/local/xtensa/
    export XTENSA_CORE=vp6_asic_2
else
    os=win32
    export XTENSA_INST=/c/usr/xtensa/XtDevTools/install
    export XTENSA_CORE=vp6_asic
fi
export XTENSA_ROOT=$XTENSA_INST/builds/RI-2019.2-$os/$XTENSA_CORE
export XTENSA_SYSTEM=$XTENSA_ROOT/config
export XTENSA_TOOLS_ROOT=$XTENSA_INST/tools/RI-2019.2-$os/XtensaTools
export LM_LICENSE_FILE=27000@192.168.200.3:27000@192.168.200.9:6000@172.17.0.205
export PATH=$PATH:$XTENSA_TOOLS_ROOT/bin

