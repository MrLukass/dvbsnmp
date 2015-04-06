/* 
 * File:   moduleHelper.h
 * Author: lukas
 *
 * Created on January 17, 2015, 9:56 AM
 */

#ifndef MODULEHELPER_H
#define	MODULEHELPER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
xmlDocPtr getXMLDoc (void);
xmlXPathObjectPtr getNodesPtr(xmlDocPtr doc, xmlChar *xpath);
void closeXML (xmlDocPtr doc, xmlXPathObjectPtr result);

#ifdef	__cplusplus
}
#endif

#endif	/* MODULEHELPER_H */

