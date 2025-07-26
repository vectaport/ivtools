cp Makefile.orig Makefile
./configure --build=x86_64-pc-linux-gnu --with-qt-libs=/usr/lib64
make
cp Makefile.orig Makefile
./configure --build=x86_64-pc-linux-gnu --with-qt-libs=/usr/lib64
make
make Makefiles
make
