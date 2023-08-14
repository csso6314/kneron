# Kneron Host Lib

This project contains C++ examples for using Kneron edge devices

This is a brief introduction. For more detailed documents, please visit <http://doc.kneron.com/docs>.

## Environment Requirements
### Linux
- install libusb
    `sudo apt install libusb-1.0-0-dev`
### Windows(MINGW64/MSYS)
- environment, gcc, etc.
  
    - get [git for windows SDK (MUST BE!)](https://gitforwindows.org/) installed
- install libusb
    `pacman -S mingw-w64-x86_64-libusb`
- install cmake
    `pacman --needed -S mingw-w64-x86_64-cmake`
    
    > make sure you are using `/mingw64/bin/cmake`

## Build
### Linux
```bash
mkdir build && cd build
 # to build opencv_3.4 example: cmake -DBUILD_OPENCV_EX=on ..
cmake ..

# after makefiles are generated, you can choose one of followings to make
make -j4 		# to build libhostkdp and all examples
```

### Windows(MINGW64/MSYS)
```bash
mkdir build && cd build
cmake .. -G"MSYS Makefiles"

# after makefiles are generated, you can choose one of followings to make
make -j4 		# to build libhostkdp and all examples
```

## Runtime DLL Environment
### Windows(MINGW64/MSYS)
- set PATH to add CV DLL location in Windows10
    - Command line example: assume MSYS2 is installed in C:\git-sdk-64\mingw64
```
set PATH=%PATH%;C:\git-sdk-64\mingw64\bin
```
- copy additional DLLs to C:\git-sdk-64\mingw64 directory
```
cp dll\*.dll C:\git-sdk-64\mingw64/bin/
```


## Output Bin Files
``host_lib/build/bin/*``

## USB Device Permissions

### Linux
Add the following to /etc/udev/rules.d/10-local.rules
```
KERNEL=="ttyUSB*",ATTRS{idVendor}=="067b",ATTRS{idProduct}=="2303",MODE="0777",SYMLINK+="kneron_uart"
KERNEL=="ttyUSB*",ATTRS{idVendor}=="1a86",ATTRS{idProduct}=="7523",MODE="0777",SYMLINK+="kneron_pwr"
SUBSYSTEM=="usb",ATTRS{idVendor}=="3231",ATTRS{idProduct}=="0100",MODE="0666"
```
