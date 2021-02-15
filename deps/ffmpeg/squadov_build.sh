#!/bin/bash
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
    --env="PKG_CONFIG_PATH=../../prebuilt/openh264/lib/pkgconfig:../../prebuilt/opus/lib/pkgconfig:../../prebuilt/libvpx/lib/x64/pkgconfig"

make -j30
make install