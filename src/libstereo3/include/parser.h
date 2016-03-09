#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP
/**
 *  @file
 *  Class "GetConfig" provides the functions to read the XML data.
 *  @version 1.0
 */

#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "stereo.h"

class GetConfig
{
public:
   GetConfig();
  ~GetConfig();
   void readConfigFile(const char* fileName);
   Stereo* getCamera(){ return m_Camera; }
   
private:
   Stereo* m_Camera;

};
#endif
