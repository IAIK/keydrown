all: keydrown2

keydrown2: inject.c
	gcc inject.c -std=gnu99 -o keydrown2 -fno-pic -mcmodel=large `pkg-config --cflags --libs gtk+-3.0` -Wno-deprecated-declarations
	
install: keydrown2
	cp keydrown2 /usr/bin/keydrown2
	
uninstall:
	rm /usr/bin/keydrown2
	
deb: keydrown2
	-if test `whoami` != "root"; then echo "\n\nYou need to run this target using fakeroot: fakeroot -u make deb\n"; exit 1; fi
	mkdir -pv keydrown-2/usr/bin
	mkdir -pv keydrown-2/usr/share/doc/keydrown-2/
	cp keydrown2 keydrown-2/usr/bin
	strip keydrown-2/usr/bin/keydrown2
	mkdir -p keydrown-2/DEBIAN
	sed "s/%VERSION%/0.1/" docs/debian-control > keydrown-2/DEBIAN/control
	echo "initial version" > keydrown-2/usr/share/doc/keydrown-2/changelog
	echo "" > keydrown-2/usr/share/doc/keydrown-2/copyright
	gzip -c -9 keydrown-2/usr/share/doc/keydrown-2/changelog > keydrown-2/usr/share/doc/keydrown-2/changelog.gz
	rm keydrown-2/usr/share/doc/keydrown-2/changelog
	chmod 0644 keydrown-2/usr/share/doc/keydrown-2/copyright
	chmod 0644 keydrown-2/usr/share/doc/keydrown-2/changelog.gz
	chmod -R 0755 keydrown-2/usr
	chown -R root:root keydrown-2/
	dpkg-deb --build keydrown-2
	rm -rf keydrown-2
	lintian keydrown-2.deb
	