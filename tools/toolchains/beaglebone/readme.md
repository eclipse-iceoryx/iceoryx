Step 1: dd image for beagle bone black

$ wget https://rcn-ee.com/rootfs/debian-armhf/2022-05-10/debian-11.3-iot-armhf-2022-05-10.tar.xz

$ tar xvf debian-11.3-iot-armhf-2022-05-10.tar.xz
$ cd debian-11.3-iot-armhf-2022-05-10/

Insert your sdcard to linux machine and check device
$lsblk
/dev/sdc

$ sudo ./setup_sdcard.sh --mmc /dev/sdc  --dtb beaglebone

Step 2: tar root file system to build + install cross compile
install cross compile
sudo apt-get install -y gcc-arm-linux-gnueabihf
sudo apt-get install -y libncurses-dev
sudo apt-get install -y libqt4-dev pkg-config
sudo apt-get install -y u-boot-tools
sudo apt-get install -y device-tree-compiler
sudo apt-get install g++-arm-linux-gnueabihf
Or download cross comiple you want 
https://releases.linaro.org/components/toolchain/binaries/latest-5/arm-linux-gnueabihf/

$wget https://releases.linaro.org/components/toolchain/binaries/latest-5/arm-linux-gnueabihf/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf.tar.xz


Note: Sometime if packages are not existed in rootfs, let try to install by chroot
Ex:
export rootbb=/home/kenny/work/image/debian-11.3-iot-armhf-2022-05-10/rootfs/
sudo chroot ${rootbb} /bin/bash -c "apt-get update" || exit 1
sudo chroot ${rootbb} /bin/bash -c "apt-get -y install libacl1-dev" || exit 1

Step 3: clone iox + create cmake toolchain file and build

$ cd iceoryx
Optional: rm -rf build

$ cmake -Bbuild -Hiceoryx_meta -DCMAKE_TOOLCHAIN_FILE=/home/kenny/work/source/iceoryx4bb/tools/toolchains/beaglebone
!! tip To build all iceoryx components add -DBUILD_ALL=ON to the CMake command. For Windows it is currently recommended to use the cmake -Bbuild -Hiceoryx_meta -DBUILD_TEST=ON -DINTROSPECTION=OFF -DBINDING_C=ON -DEXAMPLES=ON instead

$ cmake --build build

https://github.com/eclipse-iceoryx/iceoryx/blob/master/doc/website/getting-started/installation.md

link:
Cross compile
https://releases.linaro.org/components/toolchain/binaries/latest-5/arm-linux-gnueabihf/

Or 
https://stackoverflow.com/questions/36446721/arm-linux-gnueabi-g-command-not-found

Image + ros:
https://subscription.packtpub.com/book/hardware-and-creative/9781786463654/1/ch01lvl1sec12/installing-ros-in-beaglebone-black

Link image:
https://rcn-ee.com/rootfs/
https://rcn-ee.com/rootfs/debian-armhf/2022-05-10/debian-11.3-iot-armhf-2022-05-10.tar.xz

Iceoryx: 
https://github.com/eclipse-iceoryx/iceoryx