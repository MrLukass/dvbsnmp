/*
 * Note: this file originally auto-generated by mib2c using
 *        $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <libxml/xpath.h>

#include "SNMPHelper.h"
#include "XMLHelper.h"
#include "config.h"

#include "mgServiceTable.h"


static oid mgServiceTable_oid[] = {1, 3, 6, 1, 4, 1, 2696, 3, 3, 1, 1, 3};
netsnmp_table_data_set *table_set;

/** Initializes the mgServiceTable module */
void init_mgServiceTable(void) {
    /* create the table structure itself */
    table_set = netsnmp_create_table_data_set("mgServiceTable");

    netsnmp_mib_handler *cache_handler;

    /* comment this out or delete if you don't support creation of new rows */
    table_set->allow_creation = 1;

    /***************************************************
     * Adding indexes
     */
    DEBUGMSGTL(("initialize_table_mgServiceTable",
            "adding indexes to table mgServiceTable\n"));
    netsnmp_table_set_add_indexes(table_set,
            ASN_INTEGER, /* index: mgServiceNumber */
            ASN_INTEGER, /* index: mgServiceInputNumber */
            0);

    DEBUGMSGTL(("initialize_table_mgServiceTable",
            "adding column types to table mgServiceTable\n"));
    netsnmp_table_set_multi_add_default_row(table_set,
            COLUMN_MGSERVICEINPUTNUMBER, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGSERVICENUMBER, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGSERVICETYPE, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGSERVICENAME, ASN_OCTET_STR, 0,
            NULL, 0,
            COLUMN_MGSERVICEPROVIDERNAME, ASN_OCTET_STR, 0,
            NULL, 0,
            COLUMN_MGSERVICEPMTPID, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGSERVICEPCRPID, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGSERVICECONDACCESS, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGSERVICEEITCOMPONENTDESCRIPTOR, ASN_OCTET_STR, 0,
            NULL, 0,
            0);

    /* registering the table with the master agent */
    /* note: if you don't need a subhandler to deal with any aspects
       of the request, change mgServiceTable_handler to "NULL" */
    netsnmp_handler_registration *reginfo = netsnmp_create_handler_registration("mgServiceTable", NULL,
            mgServiceTable_oid,
            OID_LENGTH(mgServiceTable_oid),
            HANDLER_CAN_RWRITE);
    netsnmp_register_table_data_set(reginfo, table_set, NULL);

    DEBUGMSGTL(("mgServiceTable", "Done initalizing mgServiceTable module\n"));

    cache_handler = netsnmp_get_cache_handler(LONGCACHETIMEOUT, /* how long a cache is valid for */
            cache_load, /* a pointer to the cache loading function */
            cache_free, /* a pointer to the cache freeing function */
            mgServiceTable_oid, OID_LENGTH(mgServiceTable_oid)); /* the OID of the registration point */

    netsnmp_inject_handler(reginfo, cache_handler);
}

int cache_load(netsnmp_cache *cache, void *magic) {
    DEBUGMSGTL(("mgServiceTable", "Load Handler\n"));
    xmlDocPtr doc;
    xmlChar *xpath = (xmlChar*) "/dvb/mg/mgSignalCharacteristics/mgSignalCharacteristicsObjects/mgTSStructure/mgServiceTable/mgServiceEntry";
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    xmlNodePtr cur;
    int i;

    netsnmp_table_row *row;

    int mgServiceInputNumber = -1;
    int mgServiceNumber = -1;
    int mgServiceCondAccess = -1;
    int mgServiceType = -1;
    xmlChar * mgServiceName;
    xmlChar * mgServiceProviderName;
    int mgServicePMTPID = -1;
    int mgServicePCRPID = -1;

    doc = getXMLDoc();
    if (doc == NULL) {
        DEBUGMSGTL(("mgServiceTable", "Problem with loading file %s, keeping old values in table.\n", INPUTXMLDOCUMENTPATH));
        return SNMP_ERR_NOERROR;
    }

    cleanTableSet(table_set);

    result = getNodesPtr(doc, xpath);

    if (result) {
        nodeset = result->nodesetval;
        for (i = 0; i < nodeset->nodeNr; i++) {
            cur = nodeset->nodeTab[i]->xmlChildrenNode;
            while (cur != NULL) {
                if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceInputNumber"))) {
                    mgServiceInputNumber = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceTable", "mgServiceInputNumber: %d\n", mgServiceInputNumber));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceNumber"))) {
                    mgServiceNumber = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceTable", " mgServiceNumber: %d\n", mgServiceNumber));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceCondAccess"))) {
                    mgServiceCondAccess = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceTable", "mgServiceCondAccess: %d\n", mgServiceCondAccess));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceType"))) {
                    mgServiceType = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceTable", "mgServiceType: %d\n", mgServiceType));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceName"))) {
                    mgServiceName = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                    DEBUGMSGTL(("mgServiceTable", "mgServiceName: %s\n", mgServiceName));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceProviderName"))) {
                    mgServiceProviderName = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                    DEBUGMSGTL(("mgServiceTable", "mgServiceProviderName: %s\n", mgServiceProviderName));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServicePMTPID"))) {
                    mgServicePMTPID = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceTable", "mgServicePMTPID: %d\n", mgServicePMTPID));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServicePCRPID"))) {
                    mgServicePCRPID = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceTable", "mgServicePCRPID: %d\n", mgServicePCRPID));
                }
                cur = cur->next;
            }
            row = netsnmp_create_table_data_row();
            netsnmp_table_row_add_index(row, ASN_INTEGER, &mgServiceInputNumber, sizeof (mgServiceInputNumber));
            netsnmp_table_row_add_index(row, ASN_INTEGER, &mgServiceNumber, sizeof (mgServiceNumber));
            netsnmp_set_row_column(row, 3, ASN_INTEGER, &mgServiceType, sizeof (mgServiceType));
            netsnmp_set_row_column(row, 4, ASN_OCTET_STR, mgServiceName, strlen(mgServiceName));
            netsnmp_set_row_column(row, 5, ASN_OCTET_STR, mgServiceProviderName, strlen(mgServiceProviderName));
            netsnmp_set_row_column(row, 6, ASN_INTEGER, &mgServicePMTPID, sizeof (mgServicePMTPID));
            netsnmp_set_row_column(row, 7, ASN_INTEGER, &mgServicePCRPID, sizeof (mgServicePCRPID));
            netsnmp_set_row_column(row, 8, ASN_INTEGER, &mgServiceCondAccess, sizeof (mgServiceCondAccess));

            netsnmp_table_dataset_add_row(table_set, row);

            mgServiceInputNumber = -1;
            mgServiceNumber = -1;
            mgServiceCondAccess = -1;
            mgServiceType = -1;
            mgServiceName = 0;
            mgServiceProviderName = 0;
            mgServicePMTPID = -1;
            mgServicePCRPID = -1;
        }
    } else {
        DEBUGMSGTL(("mgServiceTable", "Data loaded from file %s is empty.\n", INPUTXMLDOCUMENTPATH));
    }
    closeXML(doc, result);
    return SNMP_ERR_NOERROR;

}

void cache_free(netsnmp_cache *cache, void *magic) {
    DEBUGMSGTL(("mgServiceTable", "Free Handler\n"));
}

void deinit_mgServiceTable(void) {
    netsnmp_delete_table_data_set(table_set);
    table_set = NULL;
    unregister_mib(mgServiceTable_oid, OID_LENGTH(mgServiceTable_oid));
}


