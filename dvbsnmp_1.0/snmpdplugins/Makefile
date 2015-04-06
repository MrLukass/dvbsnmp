# Makefile for SnmpPlugins

CC=gcc
CFLAGS= -I. `net-snmp-config --cflags`
DLFLAGS=-fPIC -shared
BUILDLIBS=`net-snmp-config --libs` `xml2-config --libs` XMLHelper.o SNMPHelper.o
OUTPUTDIR=dvbmodules

snmpdir = /usr/lib/snmpd

all: init XMLHelper.o SNMPHelper.o mgTSTable.so mgDCHTable.so mgServiceTable.so mgPIDTable.so mgNITDeliverySystemTable.so mgEMMTable.so mgServiceECMTable.so mgPIDECMTable.so
		
init:
	mkdir -p $(OUTPUTDIR)
	
XMLHelper.o: XMLHelper.c
	$(CC) $(DLFLAGS) -c -o XMLHelper.o XMLHelper.c `xml2-config --libs`

SNMPHelper.o: SNMPHelper.c
	$(CC) $(DLFLAGS) -c -o SNMPHelper.o SNMPHelper.c `net-snmp-config --libs`
	
mgDCHTable.so: mgDCHTable.c
	$(CC) $(CFLAGS) $(DLFLAGS)  -c -o mgDCHTable.o mgDCHTable.c 
	$(CC) $(CFLAGS) $(DLFLAGS) -o $(OUTPUTDIR)/mgDCHTable.so mgDCHTable.o SNMPHelper.o `net-snmp-config --libs`
	
mgTSTable.so: mgTSTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o mgTSTable.o mgTSTable.c 
	$(CC) $(CFLAGS) $(DLFLAGS) -o $(OUTPUTDIR)/mgTSTable.so mgTSTable.o $(BUILDLIBS)

mgServiceTable.so: mgServiceTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o mgServiceTable.o mgServiceTable.c 
	$(CC) $(CFLAGS) $(DLFLAGS) -o $(OUTPUTDIR)/mgServiceTable.so mgServiceTable.o $(BUILDLIBS)
	
mgPIDTable.so: mgPIDTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o mgPIDTable.o mgPIDTable.c 
	$(CC) $(CFLAGS) $(DLFLAGS) -o $(OUTPUTDIR)/mgPIDTable.so mgPIDTable.o $(BUILDLIBS)
	
mgNITDeliverySystemTable.so: mgNITDeliverySystemTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o mgNITDeliverySystemTable.o mgNITDeliverySystemTable.c 
	$(CC) $(CFLAGS) $(DLFLAGS) -o $(OUTPUTDIR)/mgNITDeliverySystemTable.so mgNITDeliverySystemTable.o $(BUILDLIBS)
	
mgEMMTable.so: mgEMMTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o mgEMMTable.o mgEMMTable.c 
	$(CC) $(CFLAGS) $(DLFLAGS) -o $(OUTPUTDIR)/mgEMMTable.so mgEMMTable.o $(BUILDLIBS)
	
mgServiceECMTable.so: mgServiceECMTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o mgServiceECMTable.o mgServiceECMTable.c 
	$(CC) $(CFLAGS) $(DLFLAGS) -o $(OUTPUTDIR)/mgServiceECMTable.so mgServiceECMTable.o $(BUILDLIBS)
	
mgPIDECMTable.so: mgPIDECMTable.c
	$(CC) $(CFLAGS) $(DLFLAGS) -c -o mgPIDECMTable.o mgPIDECMTable.c 
	$(CC) $(CFLAGS) $(DLFLAGS) -o $(OUTPUTDIR)/mgPIDECMTable.so mgPIDECMTable.o $(BUILDLIBS)

clean:
	rm -rf *o $(OUTPUTDIR)

install:
	cp -r $(OUTPUTDIR) $(DESTDIR)$(snmpdir)
