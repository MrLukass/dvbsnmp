#include <fcntl.h>
#include <errno.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "config.h"
#include "XMLHelper.h"

xmlDocPtr getXMLDoc (void) {
	
    int fd = open(INPUTXMLDOCUMENTPATH, O_RDONLY);
    if(flock(fd, LOCK_SH | LOCK_NB) != 0){
        perror("Cant get lock");
        return NULL;
    }
    xmlDocPtr doc;
    doc = xmlParseFile(INPUTXMLDOCUMENTPATH);
    flock(fd, LOCK_UN);
    close(fd);
    if (doc == NULL ) {
            fprintf(stderr,"Document not parsed successfully. \n");
            return NULL;
    }
    return doc;
}

xmlXPathObjectPtr getNodesPtr(xmlDocPtr doc, xmlChar *xpath) {

	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;

	context = xmlXPathNewContext(doc);
	if (context == NULL) {
		printf("Error in xmlXPathNewContext\n");
		return NULL;
	}
	result = xmlXPathEvalExpression(xpath, context);
	xmlXPathFreeContext(context);
	if (result == NULL) {
		printf("Error in xmlXPathEvalExpression\n");
		return NULL;
	}
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
		xmlXPathFreeObject(result);
                printf("No result\n");
		return NULL;
	}
	return result;   
}

void closeXML (xmlDocPtr doc, xmlXPathObjectPtr result){
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);
    xmlCleanupParser();
}
