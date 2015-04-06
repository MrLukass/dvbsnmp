#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

void cleanTableSet (netsnmp_table_data_set *table_set){
    netsnmp_table_row *row;
    while((row =  netsnmp_table_data_set_get_first_row(table_set)) != NULL){
        netsnmp_table_dataset_remove_and_delete_row(table_set, row);
    }
}
