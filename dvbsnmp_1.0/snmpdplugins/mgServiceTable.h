/*
 * Note: this file originally auto-generated by mib2c using
 *  $
 */
#ifndef MGSERVICETABLE_H
#define MGSERVICETABLE_H

/* function declarations */
void init_mgServiceTable(void);
void deinit_mgServiceTable(void);
NetsnmpCacheFree cache_free;
NetsnmpCacheLoad cache_load;

/* column number definitions for table mgServiceTable */
       #define COLUMN_MGSERVICEINPUTNUMBER		1
       #define COLUMN_MGSERVICENUMBER		2
       #define COLUMN_MGSERVICETYPE		3
       #define COLUMN_MGSERVICENAME		4
       #define COLUMN_MGSERVICEPROVIDERNAME		5
       #define COLUMN_MGSERVICEPMTPID		6
       #define COLUMN_MGSERVICEPCRPID		7
       #define COLUMN_MGSERVICECONDACCESS		8
       #define COLUMN_MGSERVICEEITCOMPONENTDESCRIPTOR		9
#endif /* MGSERVICETABLE_H */
