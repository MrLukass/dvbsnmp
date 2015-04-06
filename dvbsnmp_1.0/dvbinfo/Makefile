# Makefile for DVBInfo

bindir = /usr/bin

all: dvbinfo 

dvbinfo: dvbinfo.cpp
	g++ dvbinfo.cpp pugixml-1.2/src/pugixml.cpp /usr/lib/libdvbapi.a /usr/lib/libdvbsec.a /usr/lib/libucsi.a /usr/lib/libdvbcfg.a -o dvbinfo -g
clean:
	rm dvbinfo
install:
	install dvbinfo $(DESTDIR)$(bindir)

