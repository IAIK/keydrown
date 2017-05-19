all: libgtk_keydrown.so

libgtk_keydrown.so: gtkentry.c gtktextview.c
	gcc -c -Wall -Werror -fpic gtkentry.c -o gtkentry.o `pkg-config --cflags gtk+-3.0`
	gcc -c -Wall -Werror -fpic gtktextview.c -o gtktextview.o `pkg-config --cflags gtk+-3.0`
	gcc -shared -o libgtk_keydrown.so gtkentry.o gtktextview.o
	
install: libgtk_keydrown.so
	cp libgtk_keydrown.so /usr/lib/libgtk_keydrown.so
	cp keydrown3 /usr/bin/keydrown3
	
uninstall:
	rm /usr/lib/libgtk_keydrown.so
	rm /usr/bin/keydrown3
	
	
deb: libgtk_keydrown.so
	-if test `whoami` != "root"; then echo "\n\nYou need to run this target using fakeroot: fakeroot -u make deb\n"; exit 1; fi
	mkdir -pv libgtk-keydrown/usr/bin
	mkdir -pv libgtk-keydrown/usr/lib
	mkdir -pv libgtk-keydrown/usr/share/doc/libgtk-keydrown/
	cp keydrown3 libgtk-keydrown/usr/bin
	cp libgtk_keydrown.so libgtk-keydrown/usr/lib
	strip libgtk-keydrown/usr/lib/libgtk_keydrown.so
	mkdir -p libgtk-keydrown/DEBIAN
	sed "s/%VERSION%/0.1/" docs/debian-control > libgtk-keydrown/DEBIAN/control
	echo "initial version" > libgtk-keydrown/usr/share/doc/libgtk-keydrown/changelog
	echo "" > libgtk-keydrown/usr/share/doc/libgtk-keydrown/copyright
	gzip -c -9 libgtk-keydrown/usr/share/doc/libgtk-keydrown/changelog > libgtk-keydrown/usr/share/doc/libgtk-keydrown/changelog.gz
	rm libgtk-keydrown/usr/share/doc/libgtk-keydrown/changelog
	chmod 0644 libgtk-keydrown/usr/share/doc/libgtk-keydrown/copyright
	chmod 0644 libgtk-keydrown/usr/share/doc/libgtk-keydrown/changelog.gz
	chmod -R 0755 libgtk-keydrown/usr
	chown -R root:root libgtk-keydrown/
	dpkg-deb --build libgtk-keydrown
	rm -rf libgtk-keydrown
	lintian libgtk-keydrown.deb
	
clean:
	rm -f *.o *.so
	