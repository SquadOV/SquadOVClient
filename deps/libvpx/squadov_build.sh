mkdir -p squadov_build && cd squadov_build
../configure --target=x86_64-win64-vs16 --prefix=../../../prebuilt/libvpx --enable-static-msvcrt
make -j30
make install