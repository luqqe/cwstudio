mkdir binaries
tput setaf 2
make clean
./configure CFLAGS='-g -O2 -static-libgcc' CXXFLAGS='-g -O2 -static-libgcc -static-libstdc++' --host=x86_64-w64-mingw32 --with-wx-config=/usr/x86_64-w64-mingw32/bin/wx-config
make
for file in cwcli cwcurses cwwx
do
	cp src/${file}.exe binaries/${file}-win64.exe
	x86_64-w64-mingw32-strip binaries/${file}-win64.exe
done

tput setaf 1

make clean
./configure CFLAGS='-g -O2 -static-libgcc' CXXFLAGS='-g -O2 -static-libgcc -static-libstdc++' --host=i686-w64-mingw32 --with-wx-config=/usr/i686-w64-mingw32/bin/wx-config
make
for file in cwcli cwcurses cwwx
do
	cp src/${file}.exe binaries/${file}-win32.exe
	i686-w64-mingw32-strip binaries/${file}-win32.exe
done
make clean
make CFLAGS="-g -O2 -static-libgcc" CXXFLAGS="-g -O2 -static-libgcc -static-libstdc++ -DWIN9X"
for file in cwcli cwcurses cwwx
do
	cp src/${file}.exe binaries/${file}-win32-legacy.exe
	i686-w64-mingw32-strip binaries/${file}-win32-legacy.exe
done

tput setaf 4

make clean
./configure --host=i586-pc-msdosdjgpp
make
for file in cwcli cwcurses
do
	cp src/${file}.exe binaries/${file}-dos32.exe
	i586-pc-msdosdjgpp-strip binaries/${file}-dos32.exe
done

tput sgr0

make clean
