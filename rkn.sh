#!/bin/bash

RKPATH=$PWD
PRONUM=6

export ARCH=arm
export CROSS_COMPILE=$RKPATH/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-

function build_kernel()
{
#	. build/envsetup.sh;
#	lunch $PRONUM;
	cd $RKPATH/kernel;
#	make distclean;
#	make rk3066_sdk_android-4.4_defconfig;
#	make clean -j12;
	rm -f *.img;
	make kernel.img -j12;
#	cp -vf kernel.img $RKPATH/rockdev/Image-rk3066/;
	cp -vf kernel.img $RKPATH/;
#	cp .config arch/arm/configs/rk3066_sdk_android-4.4_defconfig
	cd $RKPATH;
}


function build_uboot()
{
	rm -fv $RKPATH/uboot.bin;
	cd $RKPATH/bootable/bootloader/u-boot;
#	make distclean;
#	make -j12 clean;
	rm -fv *.bin;
	make rk30xx;
	make -j12;
	cp -fv RK3066Loader_uboot.bin $RKPATH/uboot.bin;
#	cp -fv RK3066Loader_uboot.bin $RKPATH/rockdev/Image-rk3066/;
	cd $RKPATH;
}


if [ $1 = "kernel" ]; then
	build_kernel
fi

if [ $1 = "uboot" ]; then
	build_uboot
fi

cp -fv rkn.sh ../temp/
