#include <stdio.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "pugixml-1.2/src/pugixml.hpp"
#include <fstream>
#include <vector>
#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <time.h>
#include <stdint.h>
#include <list>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/file.h>

#include <libucsi/mpeg/descriptor.h>
#include <libucsi/mpeg/section.h>
#include <libucsi/dvb/descriptor.h>
#include <libucsi/dvb/section.h>
#include <libucsi/transport_packet.h>
#include <libucsi/section_buf.h>
#include <libucsi/dvb/types.h>
#include <libdvbapi/dvbdemux.h>
#include <libdvbapi/dvbfe.h>
#include <libdvbcfg/dvbcfg_zapchannel.h>
#include <libdvbsec/dvbsec_api.h>
#include <libdvbsec/dvbsec_cfg.h>
#include <libucsi/dvb/nit_section.h>

#include "config.h"

#define DEMUXDEVICE "/dev/dvb/adapter%d/demux%d"
#define TSSECTIONSIZE 4096
#define TSBUFSIZE   (1 * 4096)

#define PAT_PID 0
#define CAT_PID 1
#define NIT_PID 16
#define SDT_PID 17


int processTables(int defd, int inputNumber, pugi::xml_document *doc);
int processNITTable(int defd, int inputNumber, pugi::xml_document *doc);
int processSDTTable(int defd, int inputNumber, pugi::xml_document *doc);
int processPATTable(int defd, int inputNumber, pugi::xml_document *doc);
int processPMTTable(int defd, int inputNumber, pugi::xml_document *doc, std::list<int> *PMTPids);
int processCATTable(int defd, int inputNumber, pugi::xml_document *doc);
char* removeControlCharsFromBeginingOfString(unsigned char *text, int len);
void createEmptyOutputXMLDocument(pugi::xml_document *doc);
int loadXMLDocument(pugi::xml_document *doc);
int openDemux(char * dedev);
void closeDemux(int defd);
int readPid(int defd, __u8 * data, int dataSize, int pid);
void iprintf(int indent, char *fmt, ...);
int saveXMLDocument(pugi::xml_document* doc);
int process();
void printHelp();

enum DeliverySystemType {
    UKNOWN = 1, CABLE = 2, SATELITE = 3, TERRESTERIAL = 4
};

int loadXMLDocument(pugi::xml_document *doc) {
    createEmptyOutputXMLDocument(doc);
}

void createEmptyOutputXMLDocument(pugi::xml_document *doc) {
    pugi::xml_node nodeMgTSStructure = doc->append_child("dvb").append_child("mg").append_child("mgSignalCharacteristics").append_child("mgSignalCharacteristicsObjects").append_child("mgTSStructure");
    nodeMgTSStructure.append_child("mgTSTable");
    nodeMgTSStructure.append_child("mgServiceTable");
    nodeMgTSStructure.append_child("mgPIDTable");
    nodeMgTSStructure.append_child("mgEMMTable");
    nodeMgTSStructure.append_child("mgServiceECMTable");
    nodeMgTSStructure.append_child("mgPIDECMTable");
    nodeMgTSStructure.append_child("mgNITDeliverySystemTable");
}

int processTables(int defd, int inputNumber, pugi::xml_document *doc) {
    int ret = 1;
    if(processSDTTable(defd, inputNumber, doc)==0)
        ret = 0;
    if(processPATTable(defd, inputNumber, doc)==0)
        ret = 0;
    if(processNITTable(defd, inputNumber, doc)==0)
        ret = 0;
    if(processCATTable(defd, inputNumber, doc)==0)
        ret = 0;
    return ret;
}

char* removeControlCharsFromBeginingOfString(unsigned char *text, int len) {
    int newLen = 0;
    char *newText = new char[len+1];
    if (len <= 1) {
        newText[0] = 0;
        return newText;
    }
    int prohibitedCharsCount;
    for (prohibitedCharsCount = 0; prohibitedCharsCount < len; prohibitedCharsCount++) {
        if (text[prohibitedCharsCount] >= 0 && text[prohibitedCharsCount] <= 32||text[prohibitedCharsCount] >= 127 && text[prohibitedCharsCount] <= 49824)
            continue;
        break;
    }
    newLen = len - prohibitedCharsCount;
    for (int j = 0; j < newLen; prohibitedCharsCount++, j++) {
        newText[j] = text[prohibitedCharsCount];
    }
    newText[newLen] = 0;
    return newText;
}

int processNITTable(int defd, int inputNumber, pugi::xml_document *doc) {

    pugi::xml_node nodeMgTSTable = doc->child("dvb").child("mg").child("mgSignalCharacteristics").child("mgSignalCharacteristicsObjects").child("mgTSStructure").child("mgTSTable");
    pugi::xml_node nodeMgNITDeliverySystemTable = doc->child("dvb").child("mg").child("mgSignalCharacteristics").child("mgSignalCharacteristicsObjects").child("mgTSStructure").child("mgNITDeliverySystemTable");

    pugi::xml_node nodeMgTSTableActualRow, nodeMgNITDeliverySystemTableActualRow;
    nodeMgTSTableActualRow = nodeMgTSTable.last_child();

    char pchar[30];

    struct section *section;
    struct section_ext *section_ext = 0;

    __u8 data[TSSECTIONSIZE];

    int len = readPid(defd, data, sizeof (data), NIT_PID);
    if ((section = section_codec(data, len)) == 0) {
        fprintf(stderr, "NIT table: can't get section\n");
        return 0;
    }
    if (section->table_id == stag_dvb_network_information_actual) {
        struct dvb_nit_section *nit;
        struct descriptor *curd;
        struct dvb_nit_section_part2 *part2;
        struct dvb_nit_transport *cur_transport;

        if ((section_ext = section_ext_decode(section, 1)) == 0) {
            fprintf(stderr, "NIT table: can't get section_ext\n");
            return 0;
        }
        printf("SCT Decode NIT (pid:0x%04x) (table:0x%02x)\n", NIT_PID, section->table_id);
        if ((nit = dvb_nit_section_codec(section_ext)) == 0) {
            fprintf(stderr, "SCT XXXX NIT section decode error\n");
            return 0;
        }

        printf("SCT network_id:0x%04x\n", dvb_nit_section_network_id(nit));
        sprintf(pchar, "%d", dvb_nit_section_network_id(nit));
        nodeMgTSTableActualRow.append_child("mgTSNetworkID").append_child(pugi::node_pcdata).set_value(pchar);

        dvb_nit_section_descriptors_for_each(nit, curd) {
            if((curd->tag) == dtag_dvb_network_name){
                struct dvb_network_name_descriptor *dx;
                dx = dvb_network_name_descriptor_codec(curd);

                if (dx == NULL) {
                    fprintf(stderr, "DSC XXXX dvb_network_name_descriptor decode error\n");
                    return 0;
                }

                    char * text = removeControlCharsFromBeginingOfString(dvb_network_name_descriptor_name(dx), dvb_network_name_descriptor_name_length(dx));
                    nodeMgTSTableActualRow.append_child("mgTSNetworkName").append_child(pugi::node_pcdata).set_value(text);
                    iprintf(1, "DSC name: %s\n", text);
                    delete[] text;
                }
        }
        part2 = dvb_nit_section_part2(nit);
        
        int originalTransportSTreamID = atoi(nodeMgTSTable.last_child().child("mgTSId").child_value());

        dvb_nit_section_transports_for_each(nit, part2, cur_transport) {


            printf("\tSCT transport_stream_id:0x%04x original_network_id:0x%04x\n", cur_transport->transport_stream_id, cur_transport->original_network_id);

            if(originalTransportSTreamID == cur_transport->transport_stream_id){
                sprintf(pchar, "%d", cur_transport->original_network_id);
                nodeMgTSTableActualRow.append_child("mgTSOriginalNetworkID").append_child(pugi::node_pcdata).set_value(pchar);
            }

            dvb_nit_transport_descriptors_for_each(cur_transport, curd) {
                if (curd->tag == dtag_dvb_terrestial_delivery_system) {
                    struct dvb_terrestrial_delivery_descriptor *dx;

                    iprintf(2, "DSC Decode dvb_terrestrial_delivery_descriptor\n");
                    dx = dvb_terrestrial_delivery_descriptor_codec(curd);
                    if (dx == 0) {
                        fprintf(stderr, "DSC XXXX dvb_terrestrial_delivery_descriptor decode error\n");
                        return 0;
                    }
                    
                    if(originalTransportSTreamID == cur_transport->transport_stream_id){
                        nodeMgNITDeliverySystemTableActualRow = nodeMgNITDeliverySystemTable.append_child("mgNITDeliverySystemEntry");
                        
                        sprintf(pchar, "%d", inputNumber);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSInputNumber").append_child(pugi::node_pcdata).set_value(pchar);
                        
                        sprintf(pchar, "%d", TERRESTERIAL);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSSystemType").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->centre_frequency / 1000);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSFrequency").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSFecOuter").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSCableModulation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSSymbolRate").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSFecInner").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSOrbitalPosition").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSWestEastFlag").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSPolarization").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSSatelliteModulation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->bandwidth);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSBandwidth").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->constellation);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSConstellation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->hierarchy_information);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSHierarchyInformation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->code_rate_hp_stream);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSCodeRateHPStream").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->code_rate_lp_stream);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSCodeRateLPStream").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->guard_interval);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSGuardInterval").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->transmission_mode);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSTransmissionMode").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->other_frequency_flag);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSOtherFrequencyFlag").append_child(pugi::node_pcdata).set_value(pchar);
                    }

                    iprintf(2, "DSC centre_frequency:%i bandwidth:%i priority:%i "
                            "time_slicing_indicator:%i mpe_fec_indicator:%i constellation:%i "
                            "hierarchy_information:%i code_rate_hp_stream:%i "
                            "code_rate_lp_stream:%i guard_interval:%i transmission_mode:%i "
                            "other_frequency_flag:%i\n",
                            dx->centre_frequency, dx->bandwidth, dx->priority,
                            dx->time_slicing_indicator, dx->mpe_fec_indicator,
                            dx->constellation,
                            dx->hierarchy_information, dx->code_rate_hp_stream,
                            dx->code_rate_lp_stream, dx->guard_interval,
                            dx->transmission_mode, dx->other_frequency_flag);

                } else if (curd->tag == dtag_dvb_satellite_delivery_system) {
                    struct dvb_satellite_delivery_descriptor *dx;

                    iprintf(2, "DSC Decode dvb_satellite_delivery_descriptor\n");
                    dx = dvb_satellite_delivery_descriptor_codec(curd);
                    if (dx == 0) {
                        fprintf(stderr, "DSC XXXX dvb_satellite_delivery_descriptor decode error\n");
                        return 0;
                    }
                    
                    if(originalTransportSTreamID == cur_transport->transport_stream_id){
                        nodeMgNITDeliverySystemTableActualRow = nodeMgNITDeliverySystemTable.append_child("mgNITDeliverySystemEntry");
                        
                        sprintf(pchar, "%d", inputNumber);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSInputNumber").append_child(pugi::node_pcdata).set_value(pchar);
                        
                        sprintf(pchar, "%d", SATELITE);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSSystemType").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->frequency / 1000);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSFrequency").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSFecOuter").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSCableModulation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->symbol_rate);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSSymbolRate").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->fec_inner);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSFecInner").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->orbital_position);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSOrbitalPosition").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->west_east_flag);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSWestEastFlag").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->polarization);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSPolarization").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->modulation_type);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSSatelliteModulation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSBandwidth").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSConstellation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSHierarchyInformation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSCodeRateHPStream").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSCodeRateLPStream").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSGuardInterval").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSTransmissionMode").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSOtherFrequencyFlag").append_child(pugi::node_pcdata).set_value(pchar);
                    }

                    iprintf(2, "DSC frequency:%i orbital_position:%i west_east:%i polarization:%i roll_off:%i modulation_system:%i modulation_type: %i symbol_rate:%i fec_inner:%i\n",
                            dx->frequency,
                            dx->orbital_position,
                            dx->west_east_flag,
                            dx->polarization,
                            dx->roll_off,
                            dx->modulation_system,
                            dx->modulation_type,
                            dx->symbol_rate,
                            dx->fec_inner);
                    break;
                } else if (curd->tag == dtag_dvb_cable_delivery_system) {
                    struct dvb_cable_delivery_descriptor *dx;

                    iprintf(2, "DSC Decode dvb_cable_delivery_descriptor\n");
                    dx = dvb_cable_delivery_descriptor_codec(curd);
                    if (dx == 0) {
                        fprintf(stderr, "DSC XXXX dvb_cable_delivery_descriptor decode error\n");
                        return 0;
                    }
                    
                    if(originalTransportSTreamID == cur_transport->transport_stream_id){
                        nodeMgNITDeliverySystemTableActualRow = nodeMgNITDeliverySystemTable.append_child("mgNITDeliverySystemEntry");
                        
                        sprintf(pchar, "%d", inputNumber);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSInputNumber").append_child(pugi::node_pcdata).set_value(pchar);
                        
                        sprintf(pchar, "%d", CABLE);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSSystemType").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->frequency / 1000);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSFrequency").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->fec_outer);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSFecOuter").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->modulation);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSCableModulation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->symbol_rate);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSSymbolRate").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", dx->fec_inner);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSFecInner").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSOrbitalPosition").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSWestEastFlag").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSPolarization").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSSatelliteModulation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSBandwidth").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSConstellation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSHierarchyInformation").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSCodeRateHPStream").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSCodeRateLPStream").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSGuardInterval").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSTransmissionMode").append_child(pugi::node_pcdata).set_value(pchar);

                        sprintf(pchar, "%d", 0);
                        nodeMgNITDeliverySystemTableActualRow.append_child("mgNITDSOtherFrequencyFlag").append_child(pugi::node_pcdata).set_value(pchar);
                    }

                    iprintf(2, "DSC frequency:%i fec_outer:%i modulation:%i symbol_rate:%i fec_inner:%i\n",
                            dx->frequency, dx->fec_outer, dx->modulation,
                            dx->symbol_rate, dx->fec_inner);

                }
                else if (curd->tag == dtag_dvb_frequency_list) {
                    struct dvb_frequency_list_descriptor *dx;
                    uint32_t *freqs;
                    int count;
                    int i;

                    iprintf(2, "DSC Decode dvb_frequency_list_descriptor\n");
                    dx = dvb_frequency_list_descriptor_codec(curd);
                    if (dx == 0) {
                        fprintf(stderr, "DSC XXXX dvb_frequency_list_descriptor decode error\n");
                        return 0;
                    }
                    iprintf(0, "DSC coding_type=%i\n", dx->coding_type);

                    freqs = dvb_frequency_list_descriptor_centre_frequencies(dx);
                    count = dvb_frequency_list_descriptor_centre_frequencies_count(dx);
                    for (i = 0; i < count; i++) {
                        iprintf(2 + 1, "DSC %i\n", freqs[i]);
                    }
                }

            }
        }
    }
    return 1;
}

int processSDTTable(int defd, int inputNumber, pugi::xml_document *doc) {

    pugi::xml_node nodeMgServiceTableActualRow;
    pugi::xml_node nodeMgServiceTable = doc->child("dvb").child("mg").child("mgSignalCharacteristics").child("mgSignalCharacteristicsObjects").child("mgTSStructure").child("mgServiceTable");


    char pchar[20];

    struct section *section;
    struct section_ext *section_ext = 0;

    __u8 data[TSSECTIONSIZE];

    int len = readPid(defd, data, sizeof (data), SDT_PID);

    if ((section = section_codec(data, len)) == 0) {
        fprintf(stderr, "SDT table: can't get section\n");
        return 0;
    }

    if (section->table_id == stag_dvb_service_description_actual) {
        struct dvb_sdt_section *sdt;
        struct dvb_sdt_service *cur_service;
        struct descriptor *curd;

        if ((section_ext = section_ext_decode(section, 1)) == 0) {
            fprintf(stderr, "SDT table: can't get section_ext\n");
            return 0;
        }
        printf("SCT Decode SDT (pid:0x%04x) (table:0x%02x)\n", SDT_PID, section->table_id);
        if ((sdt = dvb_sdt_section_codec(section_ext)) == 0) {
            fprintf(stderr, "XXXX SDT section decode error\n");
            return 0;
        }

        printf("SCT transport_stream_id:0x%04x original_network_id:0x%04x\n", dvb_sdt_section_transport_stream_id(sdt), sdt->original_network_id);

        dvb_sdt_section_services_for_each(sdt, cur_service) {
            printf("\tSCT service_id:0x%04x eit_schedule_flag:%i eit_present_following_flag:%i running_status:%i free_ca_mode:%i\n",
                    cur_service->service_id,
                    cur_service->eit_schedule_flag,
                    cur_service->eit_present_following_flag,
                    cur_service->running_status,
                    cur_service->free_ca_mode);

            nodeMgServiceTableActualRow = nodeMgServiceTable.append_child("mgServiceEntry");

            sprintf(pchar, "%d", inputNumber);
            nodeMgServiceTableActualRow.append_child("mgServiceInputNumber").append_child(pugi::node_pcdata).set_value(pchar);

            sprintf(pchar, "%d", cur_service->service_id);
            nodeMgServiceTableActualRow.append_child("mgServiceNumber").append_child(pugi::node_pcdata).set_value(pchar);

            sprintf(pchar, "%d", cur_service->free_ca_mode);
            nodeMgServiceTableActualRow.append_child("mgServiceCondAccess").append_child(pugi::node_pcdata).set_value(pchar);

            dvb_sdt_service_descriptors_for_each(cur_service, curd) {
                struct dvb_service_descriptor *dx;
                struct dvb_service_descriptor_part2 *part2;

                iprintf(1, "DSC Decode dvb_service_descriptor\n");
                dx = dvb_service_descriptor_codec(curd);
                if (dx == 0) {
                    fprintf(stderr, "DSC XXXX dvb_service_descriptor decode error\n");
                    return 0;
                }
                part2 = dvb_service_descriptor_part2(dx);

                sprintf(pchar, "%d", dx->service_type);
                nodeMgServiceTableActualRow.append_child("mgServiceType").append_child(pugi::node_pcdata).set_value(pchar);

                char * serviceName = removeControlCharsFromBeginingOfString(dvb_service_descriptor_service_name(part2), part2->service_name_length);
                nodeMgServiceTableActualRow.append_child("mgServiceName").append_child(pugi::node_pcdata).set_value(serviceName);

                char * providerName = removeControlCharsFromBeginingOfString(dvb_service_descriptor_service_provider_name(dx), dx->service_provider_name_length);
                nodeMgServiceTableActualRow.append_child("mgServiceProviderName").append_child(pugi::node_pcdata).set_value(providerName);

                iprintf(1, "DSC service_type:%02x provider_name:%s service_name:%s\n",
                        dx->service_type,
                        providerName,
                        serviceName);
                delete[] serviceName;
                delete[] providerName;
            }
        }
    }
    return 1;
}

int processPATTable(int defd, int inputNumber, pugi::xml_document *doc) {

    pugi::xml_node nodeMgTSTable = doc->child("dvb").child("mg").child("mgSignalCharacteristics").child("mgSignalCharacteristicsObjects").child("mgTSStructure").child("mgTSTable");
    pugi::xml_node nodeMgTSTableActualRow;


    char pchar[20];
    std::list<int> PMTPids;

    struct section *section;
    struct section_ext *section_ext = 0;

    __u8 data[TSSECTIONSIZE];

    int len = readPid(defd, data, sizeof (data), PAT_PID);

    if ((section = section_codec(data, len)) == 0) {
        fprintf(stderr, "PAT table: can't get section\n");
        return 0;
    }

    if (section->table_id == stag_mpeg_program_association) {
        struct mpeg_pat_section *pat;
        struct mpeg_pat_program *cur;

        if ((section_ext = section_ext_decode(section, 1)) == 0) {
            fprintf(stderr, "PAT table: can't get section_ext\n");
            return 0;
        }

        printf("SCT Decode PAT (pid:0x%04x) (table:0x%02x)\n", PAT_PID, section->table_id);


        if ((pat = mpeg_pat_section_codec(section_ext)) == 0) {
            fprintf(stderr, "SCT XXXX PAT section decode error\n");
            return 0;
        }

        printf("SCT transport_stream_id:0x%04x\n", mpeg_pat_section_transport_stream_id(pat));
        
        nodeMgTSTableActualRow = nodeMgTSTable.append_child("mgTSEntry");
        
        sprintf(pchar, "%d", inputNumber);
        nodeMgTSTableActualRow.append_child("mgTSInputNumber").append_child(pugi::node_pcdata).set_value(pchar);
        
        sprintf(pchar, "%d", mpeg_pat_section_transport_stream_id(pat));
        nodeMgTSTableActualRow.append_child("mgTSId").append_child(pugi::node_pcdata).set_value(pchar);

        mpeg_pat_section_programs_for_each(pat, cur) {
            printf("\tSCT program_number:0x%04x pid:0x%04x\n", cur->program_number, cur->pid);
            if (cur->pid != NIT_PID) {
                PMTPids.push_back(cur->pid);
            }
        }

    }
    if(processPMTTable(defd, inputNumber, doc, &PMTPids)==0)
        return 0;
    return 1;
}

int processPMTTable(int defd, int inputNumber, pugi::xml_document *doc, std::list<int> *PMTPids) {

    pugi::xml_node nodeMgPIDTable = doc->child("dvb").child("mg").child("mgSignalCharacteristics").child("mgSignalCharacteristicsObjects").child("mgTSStructure").child("mgPIDTable");
    pugi::xml_node nodeMgPIDECMTable = doc->child("dvb").child("mg").child("mgSignalCharacteristics").child("mgSignalCharacteristicsObjects").child("mgTSStructure").child("mgPIDECMTable");
    pugi::xml_node nodeMgServiceECMTable = doc->child("dvb").child("mg").child("mgSignalCharacteristics").child("mgSignalCharacteristicsObjects").child("mgTSStructure").child("mgServiceECMTable");

    
    pugi::xml_node nodeMgPIDTableActualRow, nodeMgServiceTableActualRow, nodeMgPIDECMTableActualRow, nodeMgServiceECMTableActualRow;

    char pchar[100];
    sprintf(pchar,"/dvb/mg/mgSignalCharacteristics/mgSignalCharacteristicsObjects/mgTSStructure/mgServiceTable/mgServiceEntry[mgServiceInputNumber=%d]",inputNumber);
    pugi::xpath_node_set mgServiceEntries = doc->select_nodes(pchar);

    bool countOK = true;
    int position = mgServiceEntries.size() - PMTPids->size();
    if(position != 0){
        fprintf(stderr, "Difference between service count get from SDT and PAT table.\n");
        fprintf(stderr, "PAT table count: %d \n",PMTPids->size());
        fprintf(stderr, "SDT table count: %d \n",mgServiceEntries.size());
        fprintf(stderr, "All raws in mgServiceTable collected from demux%d will be removed.\n",inputNumber-1);
        for(int position = 0; position < mgServiceEntries.size(); position++){
            nodeMgServiceECMTable.remove_child(mgServiceEntries[position].node());
        }
        countOK = false;
    }
    for (std::list<int>::iterator it = PMTPids->begin(); it != PMTPids->end(); it++) {
        
        __u8 data[TSSECTIONSIZE];
        struct section *section;
        struct section_ext *section_ext2 = 0;

        int len = readPid(defd, data, sizeof (data), *it);
        if ((section = section_codec(data, len)) == 0) {
            fprintf(stderr, "PMT table: can't get section\n");
            return 0;
        }
        struct mpeg_pmt_section *pmt;
        struct descriptor *curd;
        struct mpeg_pmt_stream *cur_stream;

        if ((section_ext2 = section_ext_decode(section, 1)) == 0) {
            return 0;
        }
        printf("SCT Decode PMT (pid:0x%04x) (table:0x%02x)\n", *it, section->table_id);
        if ((pmt = mpeg_pmt_section_codec(section_ext2)) == 0) {
            fprintf(stderr, "SCT XXXX PMT section decode error\n");
            return 0;
        }
        
        if(countOK){
            nodeMgServiceTableActualRow = mgServiceEntries[position++].node();

            sprintf(pchar, "%d", *it);
            nodeMgServiceTableActualRow.append_child("mgServicePMTPID").append_child(pugi::node_pcdata).set_value(pchar);

            sprintf(pchar, "%d", pmt->pcr_pid);
            nodeMgServiceTableActualRow.append_child("mgServicePCRPID").append_child(pugi::node_pcdata).set_value(pchar);

            printf("SCT program_number:0x%04x pcr_pid:0x%02x\n", mpeg_pmt_section_program_number(pmt), pmt->pcr_pid);
        }

        mpeg_pmt_section_descriptors_for_each(pmt, curd) {
            if(curd->tag == dtag_mpeg_ca)
            {
                struct mpeg_ca_descriptor *dx;

                iprintf(2, "DSC Decode mpeg_ca_descriptor\n");
                dx = mpeg_ca_descriptor_codec(curd);
                if (dx == 0) {
                    fprintf(stderr, "DSC XXXX mpeg_ca_descriptor decode error\n");
                    return 0;
                }

                nodeMgServiceECMTableActualRow = nodeMgServiceECMTable.append_child("mgServiceECMEntry");

                sprintf(pchar, "%d", inputNumber);
                nodeMgServiceECMTableActualRow.append_child("mgServiceECMInputNumber").append_child(pugi::node_pcdata).set_value(pchar);

                sprintf(pchar, "%d", mpeg_pmt_section_program_number(pmt));
                nodeMgServiceECMTableActualRow.append_child("mgServiceECMServiceNumber").append_child(pugi::node_pcdata).set_value(pchar);
                
                sprintf(pchar, "%d", dx->ca_pid + 1);
                nodeMgServiceECMTableActualRow.append_child("mgServiceECMCaPID").append_child(pugi::node_pcdata).set_value(pchar);

                sprintf(pchar, "%d", dx->ca_system_id);
                nodeMgServiceECMTableActualRow.append_child("mgServiceECMCASystemID").append_child(pugi::node_pcdata).set_value(pchar);

                iprintf(3, "DSC ca_system_id:0x%04x ca_pid:0x%04x\n",
                        dx->ca_system_id,
                        dx->ca_pid);
            }
            else{
                //parse_descriptor(curd, 1, 1);
            }
        }

        mpeg_pmt_section_streams_for_each(pmt, cur_stream) {
            printf("\tSCT stream_type:0x%02x pid:0x%04x\n", cur_stream->stream_type, cur_stream->pid);

            nodeMgPIDTableActualRow = nodeMgPIDTable.append_child("mgPIDEntry");

            sprintf(pchar, "%d", inputNumber);
            nodeMgPIDTableActualRow.append_child("mgPIDInputNumber").append_child(pugi::node_pcdata).set_value(pchar);

            sprintf(pchar, "%d", mpeg_pmt_section_program_number(pmt));
            nodeMgPIDTableActualRow.append_child("mgPIDServiceNumber").append_child(pugi::node_pcdata).set_value(pchar);

            sprintf(pchar, "%d", cur_stream->pid + 1);
            nodeMgPIDTableActualRow.append_child("mgPIDNumber").append_child(pugi::node_pcdata).set_value(pchar);

            sprintf(pchar, "%d", cur_stream->stream_type);
            nodeMgPIDTableActualRow.append_child("mgPIDType").append_child(pugi::node_pcdata).set_value(pchar);

            mpeg_pmt_stream_descriptors_for_each(cur_stream, curd) {
                if(curd->tag == dtag_mpeg_ca)
                {
                    struct mpeg_ca_descriptor *dx;

                    iprintf(2, "DSC Decode mpeg_ca_descriptor\n");
                    dx = mpeg_ca_descriptor_codec(curd);
                    if (dx == 0) {
                        fprintf(stderr, "DSC XXXX mpeg_ca_descriptor decode error\n");
                        return 0;
                    }

                    nodeMgPIDECMTableActualRow = nodeMgPIDECMTable.append_child("MgPIDECMEntry");

                    sprintf(pchar, "%d", inputNumber);
                    nodeMgPIDECMTableActualRow.append_child("mgPIDECMInputNumber").append_child(pugi::node_pcdata).set_value(pchar);

                    sprintf(pchar, "%d", mpeg_pmt_section_program_number(pmt));
                    nodeMgPIDECMTableActualRow.append_child("mgPIDECMServiceNumber").append_child(pugi::node_pcdata).set_value(pchar);
                    
                    sprintf(pchar, "%d", cur_stream->pid + 1);
                    nodeMgPIDECMTableActualRow.append_child("mgPIDECMPID").append_child(pugi::node_pcdata).set_value(pchar);
                    
                    sprintf(pchar, "%d", dx->ca_pid + 1);
                    nodeMgPIDECMTableActualRow.append_child("mgPIDECMCaPID").append_child(pugi::node_pcdata).set_value(pchar);

                    sprintf(pchar, "%d", dx->ca_system_id);
                    nodeMgPIDECMTableActualRow.append_child("mgPIDECMCASystemID").append_child(pugi::node_pcdata).set_value(pchar);

                    iprintf(3, "DSC ca_system_id:0x%04x ca_pid:0x%04x\n",
                            dx->ca_system_id,
                            dx->ca_pid);
                }
                else
                {
                    //parse_descriptor(curd, 2, 1);
                }
            }
        }
    }
    return 1;
}

int processCATTable(int defd, int inputNumber, pugi::xml_document* doc){
    
    pugi::xml_node nodeMgEMMTable = doc->child("dvb").child("mg").child("mgSignalCharacteristics").child("mgSignalCharacteristicsObjects").child("mgTSStructure").child("mgEMMTable");
    pugi::xml_node nodeMgEMMTableActualRow;
    char pchar[20];
    
    struct section *section;
    struct section_ext *section_ext = 0;

    __u8 data[TSSECTIONSIZE];
    
    int len = readPid(defd, data, sizeof (data), CAT_PID);

    if ((section = section_codec(data, len)) == 0) {
        fprintf(stderr, "CAT table: can't get section\n");
        return 0;
    }

    struct mpeg_cat_section *cat;
    struct descriptor *curd;

    if ((section_ext = section_ext_decode(section, 1)) == 0) {
        fprintf(stderr, "CAT table: can't get section_ext\n");
        return 0;
    }
    printf("SCT Decode CAT (pid:0x%04x) (table:0x%02x)\n", CAT_PID, section->table_id);
    if ((cat = mpeg_cat_section_codec(section_ext)) == 0) {
        fprintf(stderr, "SCT XXXX CAT section decode error\n");
        return 0;
    }

    mpeg_cat_section_descriptors_for_each(cat, curd) {
        if(curd->tag == dtag_mpeg_ca)
        {
            struct mpeg_ca_descriptor *dx;

            iprintf(2, "DSC Decode mpeg_ca_descriptor\n");
            dx = mpeg_ca_descriptor_codec(curd);
            if (dx == 0) {
                fprintf(stderr, "DSC XXXX mpeg_ca_descriptor decode error\n");
                return 0;
            }
            
            nodeMgEMMTableActualRow = nodeMgEMMTable.append_child("mgEMMEntry");
            
            sprintf(pchar, "%d", inputNumber);
            nodeMgEMMTableActualRow.append_child("mgEMMInputNumber").append_child(pugi::node_pcdata).set_value(pchar);
            
            sprintf(pchar, "%d", dx->ca_pid + 1);
            nodeMgEMMTableActualRow.append_child("mgEMMCaPID").append_child(pugi::node_pcdata).set_value(pchar);
            
            sprintf(pchar, "%d", dx->ca_system_id);
            nodeMgEMMTableActualRow.append_child("mgEMMCASystemID").append_child(pugi::node_pcdata).set_value(pchar);
            
            iprintf(3, "DSC ca_system_id:0x%04x ca_pid:0x%04x\n",
                    dx->ca_system_id,
                    dx->ca_pid);
        }
    }
}

int process(){
    int ret = 1;
    int demux = 0;
    int adapter = 0;
    int inputNumber = 1; //identificator of demux, according MIB file it started from 1 to N
    char dedev[128]; //contains actual path to demux device
    pugi::xml_document *doc = new pugi::xml_document;
    
    loadXMLDocument(doc);

    while (inputNumber <= MAXDEVICECOUNT) {
        snprintf(dedev, sizeof (dedev), DEMUXDEVICE, adapter, demux);
        printf("Try to open device: %s\n", dedev);
        if (access(dedev, F_OK) != -1) {
            printf("Success, device exist!!! \n");
            printf("Try to get connection.\n");
            int defd;
            if((defd = openDemux(dedev)) != 0){
                printf("Connected to demux device.\n");
                if(processTables(defd, inputNumber, doc) == 0){
                    ret = 0;
                }
                closeDemux(defd);
            }else {
                printf("Connection to demux failed.\n");
            }
        } else {
            printf("Fail, device doesn't exist!!! \n");
        }
        adapter++;
        inputNumber++;
    }
    
    saveXMLDocument(doc);
    delete doc;
    return ret;
}

int saveXMLDocument(pugi::xml_document* doc){
    int fd;
    
    umask(0000);
    if((fd = open(OUTPUTXMLDOCUMENTPATH, O_RDWR|O_CREAT, 01666)) < 0){
        perror("Opening output file failed");
        return 0;
    }
    if(flock(fd, LOCK_EX) != 0){
        perror("Locking output file failed");
        return 0;
    }
    doc->save_file(OUTPUTXMLDOCUMENTPATH);
    flock(fd, LOCK_UN);
    close(fd);
    return 1;
}

int main(int argc, char **argv) {
    bool debug = false;
    if(argc > 2){
        printf("Too many input arguments!!\n");
        return 0;
    }
    else if(argc == 2){
        if((std::string)argv[1] == "-d"){
            debug = true;
            printf("DVBInfo is running debug mode.\n");
        }
        else if((std::string)argv[1] == "-h"){
            printHelp();
            return 1;
        }
        else{
            printf("Unknown input argument!!\n");
            return 0;
        }
    }
    else{
        printf("DVBInfo is running normal mode.\n");
        fflush(stdout);
        freopen("/dev/null","w", stdout );
    }
    process();
    if(!debug){
        fflush(stdout);
        freopen("/dev/tty","w", stdout );
    }
    printf("DVBInfo finished.\n");
    return 1;
}

int openDemux(char * dedev) {
    int defd;

    if ((defd = open(dedev, O_RDWR | O_LARGEFILE)) < 0) {
        perror("Opening demux failed");
        return 0;
    }
    return defd;
}

void closeDemux(int defd) {
    close(defd);
}

int readPid(int defd, __u8 * data, int dataSize, int pid) {
    long dmxBufferSize = TSBUFSIZE;

    if (ioctl(defd, DMX_SET_BUFFER_SIZE, dmxBufferSize) < 0) {
        perror("Set demux filter failed");
        return 0;
    }
    struct dmx_sct_filter_params sctFilterParams;
    memset(&sctFilterParams, 0, sizeof (struct dmx_sct_filter_params));
    sctFilterParams.pid = pid;
    sctFilterParams.timeout = 30000; //30s
    sctFilterParams.flags = DMX_IMMEDIATE_START | DMX_CHECK_CRC | DMX_ONESHOT;

    if (ioctl(defd, DMX_SET_FILTER, &sctFilterParams) < 0) {
        perror("Set demux filter failed");
        return 0;
    }

    int len = read(defd, data, dataSize);
    if(len <= 0){
        perror("Read demux failed");
        return 0;
    }

    return len;
}

void iprintf(int indent, char *fmt, ...){
    va_list ap;

    while(indent--) {
            printf("\t");
    }

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}
void printHelp(){
    printf("DVBinfo\n"
            "Application for collecting information about DVB devices.\n\n"
            "Version:\t%s\n"
            "Author:\t\tLukas Zajac\n"
            "Email:\t\tzajac.ov@gmail.com\n\n"
            "OPTIONS:\n"
            "\t-h\t\tdisplay this help message\n"
            "\t-d\t\tstart DVBinfo in debug mode\n",VERSION);
}
