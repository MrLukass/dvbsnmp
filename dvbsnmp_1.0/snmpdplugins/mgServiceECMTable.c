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

#include "mgServiceECMTable.h"

static oid mgServiceECMTable_oid[] = {1, 3, 6, 1, 4, 1, 2696, 3, 3, 1, 1, 6};
netsnmp_table_data_set *table_set;

/** Initializes the mgServiceECMTable module */
void init_mgServiceECMTable(void) {
    /* here we initialize all the tables we're planning on supporting */
    /* create the table structure itself */
    table_set = netsnmp_create_table_data_set("mgServiceECMTable");

    netsnmp_mib_handler *cache_handler;

    /* comment this out or delete if you don't support creation of new rows */
    table_set->allow_creation = 1;

    /***************************************************
     * Adding indexes
     */
    DEBUGMSGTL(("mgServiceECMTable", "adding indexes to table mgServiceECMTable\n"));
    netsnmp_table_set_add_indexes(table_set,
            ASN_INTEGER, /* index: mgServiceECMInputNumber */
            ASN_INTEGER, /* index: mgServiceECMServiceNumber */
            0);

    DEBUGMSGTL(("mgServiceECMTable", "adding column types to table mgServiceECMTable\n"));
    netsnmp_table_set_multi_add_default_row(table_set,
            COLUMN_MGSERVICEECMINPUTNUMBER, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGSERVICEECMSERVICENUMBER, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGSERVICEECMCAPID, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGSERVICEECMCASYSTEMID, ASN_INTEGER, 0,
            NULL, 0,
            0);

    /* registering the table with the master agent */
    /* note: if you don't need a subhandler to deal with any aspects
       of the request, change mgServiceECMTable_handler to "NULL" */
    netsnmp_handler_registration *reginfo = netsnmp_create_handler_registration("mgServiceECMTable", NULL,
            mgServiceECMTable_oid,
            OID_LENGTH(mgServiceECMTable_oid),
            HANDLER_CAN_RWRITE);
    netsnmp_register_table_data_set(reginfo, table_set, NULL);

    DEBUGMSGTL(("mgServiceECMTable", "Done initalizing mgServiceECMTable module\n"));

    cache_handler = netsnmp_get_cache_handler(LONGCACHETIMEOUT, /* how long a cache is valid for */
            cache_load, /* a pointer to the cache loading function */
            cache_free, /* a pointer to the cache freeing function */
            mgServiceECMTable_oid, OID_LENGTH(mgServiceECMTable_oid)); /* the OID of the registration point */

    netsnmp_inject_handler(reginfo, cache_handler);
}

int cache_load(netsnmp_cache *cache, void *magic) {
    DEBUGMSGTL(("mgServiceECMTable", "Load Handler\n"));
    xmlDocPtr doc;
    xmlChar *xpath = (xmlChar*) "/dvb/mg/mgSignalCharacteristics/mgSignalCharacteristicsObjects/mgTSStructure/mgServiceECMTable/mgServiceECMEntry";
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    xmlNodePtr cur;
    int i;

    netsnmp_table_row *row;

    int mgServiceECMInputNumber = -1;
    int mgServiceECMServiceNumber = -1;
    int mgServiceECMCaPID = -1;
    int mgServiceECMCASystemID = -1;

    doc = getXMLDoc();
    if (doc == NULL) {
        DEBUGMSGTL(("mgServiceECMTable", "Problem with loading file %s, keeping old values in table.\n", INPUTXMLDOCUMENTPATH));
        return SNMP_ERR_NOERROR;
    }

    cleanTableSet(table_set);

    result = getNodesPtr(doc, xpath);

    if (result) {
        nodeset = result->nodesetval;
        for (i = 0; i < nodeset->nodeNr; i++) {
            cur = nodeset->nodeTab[i]->xmlChildrenNode;
            while (cur != NULL) {
                if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceECMInputNumber"))) {
                    mgServiceECMInputNumber = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceECMTable", "mgServiceECMInputNumber: %d\n", mgServiceECMInputNumber));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceECMServiceNumber"))) {
                    mgServiceECMServiceNumber = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceECMTable", " mgServiceECMServiceNumber: %d\n", mgServiceECMServiceNumber));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceECMCaPID"))) {
                    mgServiceECMCaPID = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceECMTable", "mgServiceECMCaPID: %d\n", mgServiceECMCaPID));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgServiceECMCASystemID"))) {
                    mgServiceECMCASystemID = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgServiceECMTable", "mgServiceECMCASystemID: %d\n", mgServiceECMCASystemID));
                }
                cur = cur->next;
            }
            row = netsnmp_create_table_data_row();
            netsnmp_table_row_add_index(row, ASN_INTEGER, &mgServiceECMInputNumber, sizeof (mgServiceECMInputNumber));
            netsnmp_table_row_add_index(row, ASN_INTEGER, &mgServiceECMServiceNumber, sizeof (mgServiceECMServiceNumber));
            netsnmp_set_row_column(row, 3, ASN_INTEGER, &mgServiceECMCaPID, sizeof (mgServiceECMCaPID));
            netsnmp_set_row_column(row, 4, ASN_INTEGER, &mgServiceECMCASystemID, sizeof (mgServiceECMCASystemID));
            netsnmp_table_dataset_add_row(table_set, row);

            mgServiceECMInputNumber = -1;
            mgServiceECMServiceNumber = -1;
            mgServiceECMCaPID = -1;
            mgServiceECMCASystemID = -1;
        }

    } else {
        DEBUGMSGTL(("mgServiceECMTable", "Data loaded from file %s is empty.\n", INPUTXMLDOCUMENTPATH));
    }
    closeXML(doc, result);
    return SNMP_ERR_NOERROR;

}

void cache_free(netsnmp_cache *cache, void *magic) {
    DEBUGMSGTL(("mgServiceECMTable", "Free Handler\n"));
}

void deinit_mgServiceECMTable(void) {
    netsnmp_delete_table_data_set(table_set);
    table_set = NULL;
    unregister_mib(mgServiceECMTable_oid, OID_LENGTH(mgServiceECMTable_oid));
}

