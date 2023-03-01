cd external/jpeg-9a-mac

chmod +x configure
./configure --prefix=$(pwd)

make clean

make
chmod +x install-sh
make install

