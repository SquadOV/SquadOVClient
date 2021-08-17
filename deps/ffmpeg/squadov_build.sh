#!/bin/bash
CURRENT_DIR=$(pwd)/../..
ROOT_DIR=$(realpath "${CURRENT_DIR}")
echo "ROOT DIR ${ROOT_DIR}"

./configure --prefix=../../prebuilt/ffmpeg \
    --enable-shared \
	--disable-static \
    --disable-doc \
    --enable-libopenh264 \
    --enable-amf \
    --enable-nvenc \
	--enable-libvpx \
	--enable-libopus \
    --toolchain=msvc \
    --target-os=win64 \
    --arch=x86_64 \
    --env="PKG_CONFIG_PATH=${ROOT_DIR}/prebuilt/openh264/lib/pkgconfig:${ROOT_DIR}/prebuilt/opus/lib/pkgconfig:${ROOT_DIR}/prebuilt/libvpx/lib/x64/pkgconfig"

make clean
make -j30
make install