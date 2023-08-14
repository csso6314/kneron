# How to use command line DSP build environment

## environment
### windows
  - Xtensa Xplorer must be installed with default path, `C:\user\Xtensa`
  - mingw64 environment
### Linux
  Xtensa tools must be installed under `/usr/local/xtensa/`

note:
> Modify tool path in `toolenv.sh` if the install path is not the same as above

## Steps
1. init tool configuration
   `$xcc> source envtool.sh`
2. run build script to compile project
   `$xcc> bash build.sh [debug|release]`

> The script include 3 parts
> - cmake the project
> - make the project
> - copy built fw_ncpu.bin to bin_gen / Jlink_programmer folders
> - CMAKE checks if libDriver/main/src exists or not,
    if exists, cmake builds all libraries (for RD working environment)
    if not, cmake builds with prebuilt libraries (for release)

