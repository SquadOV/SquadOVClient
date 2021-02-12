#!/bin/bash
./configure --prefix=../../prebuilt/ffmpeg \
    --enable-shared \
    --disable-doc \
    --enable-libopenh264 \
    --enable-amf \
    --enable-nvenc \
    --toolchain=msvc \
    --target-os=win64 \
    --arch=x86_64 \
    --env="PKG_CONFIG_PATH=../../prebuilt/openh264/lib/pkgconfig"

make -j30
make install