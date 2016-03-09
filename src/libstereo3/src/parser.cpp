#include <string.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "parser.h"
#include "mono.h"

#define TAG_Left "Left"
#define TAG_Right "Right"
#define TAG_R "R"
#define TAG_T "T"

#define UND_MAXLINE 4096

/** Structure that store 2 double values */
typedef struct doublePoint {
  double x;	      /**< first double*/
  double y;	      /**< second double*/
} doublePoint;

/**Structure that store the Calibration Information*/
typedef struct calibInfo {
  doublePoint f;
  doublePoint p;
  double k[4];
} calibInfo;

/**
 * Search the children of a node of XMLTree for a specific tag and return its contents
 * @param node the parent node
 * @param tag the tag to be searched
 * @param content pointer to the string where to store the content
 * @param content_size size that could be stored in the content
 * @return a pointer to content or NULL on error or the element is not found
 */

xmlNode* XmlFindTag(const xmlNode* node, const char* tag )
{
	xmlNode* aux,*first;
	if(!node || !tag)
		return NULL;
	if(!(first=xmlFirstElementChild((xmlNodePtr)node)))
		return NULL;
	aux=first;
	while(aux)
	{
		if(!(strcmp(tag,(char*)aux->name)))
			return aux;
		aux=aux->next;
	}
	return NULL;
}

/**
 * Search the children of a node of XMLTree for a specific tag and return a pointer to the founded node
 * @param node parent node
 * @param tag the tag to be searched
 * @return the node founded or NULL on error or if the respective tag was not in the children of node
 */

char* XmlGetTagContent(const xmlNode* node, const char* tag, char* content,const size_t content_size)
{
	xmlNode* aux;
	char* cont;
	if(!node || !tag || !content)
		return NULL;
	if(!(aux=XmlFindTag(node,tag)))
		return NULL;
	if(!(cont=(char*)xmlNodeGetContent(aux)))
		return NULL;
	strncpy(content,cont,content_size -1);
	return content;
}

/**
 *  Constructor initializes xerces-C libraries.
 *  The XML tags and attributes which we seek are defined.
 *  The xerces-C DOM parser infrastructure is initialized.
 */

GetConfig::GetConfig()
{
   m_Camera = new Stereo();
}

/**
 *  Class destructor frees memory used to hold the XML tag and 
 *  attribute definitions. It als terminates use of the xerces-C
 *  framework.
 */

GetConfig::~GetConfig()
{
   delete m_Camera;
}


/**
 * Extract the Calibration Information from a xml file and sets a structure with it
 * @param filename the XML file
 * @param tag the tag under the calibration are stored. Valid values are Left and Right (for each specified cameras)
 * @param info the address of the structure where the parameter will be stored
 * @return 0 on success -1 on error
 */

int GetIntrinsics(const char* filename, const char* tag, calibInfo* info)
{
	xmlDocPtr doc;
	xmlNode *root_ele, *cam, *calib;
	char value[UND_MAXLINE],*cont;
	doublePoint* dval;
	int i=0;
	if(!(doc=xmlReadFile(filename, NULL, 0)))
		return -1;
	root_ele=xmlDocGetRootElement(doc);
	if(!(cam=XmlFindTag(root_ele,tag)))
	{
		xmlFreeDoc(doc);
		return -1;
	}
	for(calib=xmlFirstElementChild(cam);calib;calib=calib->next)
	{
		if(!(strcmp("text",(char*)calib->name)))
			continue;
		if(!(strcmp((char*)calib->name,"f"))|| !(strcmp((char*)calib->name,"p")))
		{
			if(!strcmp((char*)calib->name,"f"))
				dval=&info->f;
			else
				dval=&info->p;
			if(!((char*)XmlGetTagContent(calib,"x",value,UND_MAXLINE)))
			{
				xmlFreeDoc(doc);
				return -1;
			}
			dval->x=(double)atof(value);
			if(!(XmlGetTagContent(calib,"y",value,UND_MAXLINE)))
			{
				xmlFreeDoc(doc);
				return -1;
			}
			dval->y=(double)atof(value);
		}
		else
		{
			if(i<4)
			{
				if(!(cont=(char*)xmlNodeGetContent(calib)))
				{
					xmlFreeDoc(doc);
					return -1;
				}
				strncpy(value,cont,UND_MAXLINE);
				info->k[i]=(double)atof(value);
			}
			i++;
		}
	}
	xmlFreeDoc(doc);
	return 0;
}

int GetExtrinsics(const char* filename, const char* tag, double* params, int size)
{
	xmlDocPtr doc;
	xmlNode *root_ele, *cam, *calib;
	char value[UND_MAXLINE],*cont;
	doublePoint* dval;
	int i=0;
	if(!(doc=xmlReadFile(filename, NULL, 0)))
		return -1;
	root_ele=xmlDocGetRootElement(doc);
	if(!(cam=XmlFindTag(root_ele,tag)))
	{
		xmlFreeDoc(doc);
		return -1;
	}
	for(calib=xmlFirstElementChild(cam);calib;calib=calib->next)
	{
		if(!(strcmp("text",(char*)calib->name)))
			continue;
		else
		{
			if(i<size)
			{
				if(!(cont=(char*)xmlNodeGetContent(calib)))
				{
					xmlFreeDoc(doc);
					return -1;
				}
				strncpy(value,cont,UND_MAXLINE);
				params[i]=(double)atof(value);
			}
			i++;
		}
	}
	xmlFreeDoc(doc);
	return 0;
}

/**
 *  This function:
 *  - Reads and extracts the pertinent information from the XML config file.
 *
 *  @param in configFile The text string name of the HLA configuration file.
 */

void GetConfig::readConfigFile(const char* fileName)
{
	double R[9], T[3];
	calibInfo CAM_Calib;
	if( GetIntrinsics(fileName, TAG_Left, &CAM_Calib) == 0)
	{
		m_Camera->getLeft()->setIntrinsics(CAM_Calib.f.x, CAM_Calib.f.y, CAM_Calib.p.x, CAM_Calib.p.y);
		m_Camera->getLeft()->setDistortion(CAM_Calib.k[0], CAM_Calib.k[1], CAM_Calib.k[2], CAM_Calib.k[3]);
	}
	if( GetIntrinsics(fileName, TAG_Right, &CAM_Calib) == 0)
	{
		m_Camera->getRight()->setIntrinsics(CAM_Calib.f.x, CAM_Calib.f.y, CAM_Calib.p.x, CAM_Calib.p.y);
		m_Camera->getRight()->setDistortion(CAM_Calib.k[0], CAM_Calib.k[1], CAM_Calib.k[2], CAM_Calib.k[3]);
	}
	if( GetExtrinsics(fileName, TAG_R, R, 9) == 0)
		m_Camera->setRotation(R[0], R[1], R[2], R[3], R[4], R[5], R[6], R[7], R[8]);
	if( GetExtrinsics(fileName, TAG_T, T, 3) == 0)
		m_Camera->setTranslation(T[0], T[1], T[2]);

}

