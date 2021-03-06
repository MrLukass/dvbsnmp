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

#include "mgPIDTable.h"

static oid mgPIDTable_oid[] = {1, 3, 6, 1, 4, 1, 2696, 3, 3, 1, 1, 4};
netsnmp_table_data_set *table_set;

/** Initializes the mgPIDTable module */
void init_mgPIDTable(void) {

    /* here we initialize all the tables we're planning on supporting */
    /* create the table structure itself */
    table_set = netsnmp_create_table_data_set("mgPIDTable");

    netsnmp_mib_handler *cache_handler;

    /* comment this out or delete if you don't support creation of new rows */
    table_set->allow_creation = 1;

    /***************************************************
     * Adding indexes
     */
    DEBUGMSGTL(("mgPIDTable", "adding indexes to table mgPIDTable\n"));
    netsnmp_table_set_add_indexes(table_set,
            ASN_INTEGER, /* index: mgPIDServiceNumber */
            ASN_INTEGER, /* index: mgPIDNumber */
            ASN_INTEGER, /* index: mgPIDInputNumber */
            0);

    DEBUGMSGTL(("mgPIDTable", "adding column types to table mgPIDTable\n"));
    netsnmp_table_set_multi_add_default_row(table_set,
            COLUMN_MGPIDINPUTNUMBER, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGPIDSERVICENUMBER, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGPIDNUMBER, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGPIDTYPE, ASN_INTEGER, 0,
            NULL, 0,
            COLUMN_MGPIDCONDACCESS, ASN_INTEGER, 0,
            NULL, 0,
            0);

    /* registering the table with the master agent */
    /* note: if you don't need a subhandler to deal with any aspects
       of the request, change mgPIDTable_handler to "NULL" */
    netsnmp_handler_registration *reginfo = netsnmp_create_handler_registration("mgPIDTable", NULL,
            mgPIDTable_oid,
            OID_LENGTH(mgPIDTable_oid),
            HANDLER_CAN_RWRITE);
    netsnmp_register_table_data_set(reginfo, table_set, NULL);

    DEBUGMSGTL(("mgPIDTable", "Done initalizing mgPIDTable module\n"));

    cache_handler = netsnmp_get_cache_handler(LONGCACHETIMEOUT, /* how long a cache is valid for */
            cache_load, /* a pointer to the cache loading function */
            cache_free, /* a pointer to the cache freeing function */
            mgPIDTable_oid, OID_LENGTH(mgPIDTable_oid)); /* the OID of the registration point */

    netsnmp_inject_handler(reginfo, cache_handler);
}

int cache_load(netsnmp_cache *cache, void *magic) {
    DEBUGMSGTL(("mgPIDTable", "Load Handler\n"));
    xmlDocPtr doc;
    xmlChar *xpath = (xmlChar*) "/dvb/mg/mgSignalCharacteristics/mgSignalCharacteristicsObjects/mgTSStructure/mgPIDTable/mgPIDEntry";
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    xmlNodePtr cur;
    int i;

    netsnmp_table_row *row;

    int mgPIDInputNumber = -1;
    int mgPIDServiceNumber = -1;
    int mgPIDNumber = -1;
    int mgPIDType = -1;
    doc = getXMLDoc();
    if (doc == NULL) {
        DEBUGMSGTL(("mgPIDTable", "Problem with loading file %s, keeping old values in table.\n", INPUTXMLDOCUMENTPATH));
        return SNMP_ERR_NOERROR;
    }

    cleanTableSet(table_set);

    result = getNodesPtr(doc, xpath);

    if (result) {
        nodeset = result->nodesetval;
        for (i = 0; i < nodeset->nodeNr; i++) {
            cur = nodeset->nodeTab[i]->xmlChildrenNode;
            while (cur != NULL) {
                if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgPIDInputNumber"))) {
                    mgPIDInputNumber = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgPIDTable", "mgPIDInputNumber: %d\n", mgPIDInputNumber));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgPIDServiceNumber"))) {
                    mgPIDServiceNumber = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgPIDTable", " mgPIDServiceNumber: %d\n", mgPIDServiceNumber));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgPIDNumber"))) {
                    mgPIDNumber = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgPIDTable", "mgPIDNumber: %d\n", mgPIDNumber));
                } else if ((!xmlStrcmp(cur->name, (const xmlChar *) "mgPIDType"))) {
                    mgPIDType = atoi(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
                    DEBUGMSGTL(("mgPIDTable", "mgPIDType: %d\n", mgPIDType));
                }
                cur = cur->next;
            }
            row = netsnmp_create_table_data_row();
            netsnmp_table_row_add_index(row, ASN_INTEGER, &mgPIDInputNumber, sizeof (mgPIDInputNumber));
            netsnmp_table_row_add_index(row, ASN_INTEGER, &mgPIDServiceNumber, sizeof (mgPIDServiceNumber));
            netsnmp_table_row_add_index(row, ASN_INTEGER, &mgPIDNumber, sizeof (mgPIDNumber));
            netsnmp_set_row_column(row, 4, ASN_INTEGER, &mgPIDType, sizeof (mgPIDType));
            netsnmp_table_dataset_add_row(table_set, row);

            mgPIDInputNumber = -1;
            mgPIDServiceNumber = -1;
            mgPIDNumber = -1;
            mgPIDType = -1;
        }
    } else {
        DEBUGMSGTL(("mgPIDTable", "Data loaded from file %s is empty.\n", INPUTXMLDOCUMENTPATH));
    }
    closeXML(doc, result);
    return SNMP_ERR_NOERROR;

}

void cache_free(netsnmp_cache *cache, void *magic) {
    DEBUGMSGTL(("mgPIDTable", "Free Handler\n"));
}

void deinit_mgPIDTable(void) {
    netsnmp_delete_table_data_set(table_set);
    table_set = NULL;
    unregister_mib(mgPIDTable_oid, OID_LENGTH(mgPIDTable_oid));
}