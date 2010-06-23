/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Includes
#include "config.h"

#include "HttpFunctionHandler.h"

#include "HttpMapFunctions.h"
#include "HttpNavigatorFunctions.h"
#include "HttpRouteFunctions.h"
#include "HttpTMapFunctions.h"
#include "HttpProxyFunctions.h"
#include "HttpSimplePoiFunctions.h"

#include "HttpBody.h"
#include "HttpHeader.h"

#include "ExpandItemID.h"
#include "ExpandStringItem.h"
#include "ServerProximityPackets.h"


//////////////////////////////////////////////////////////////
// HttpFunctionHandler
//////////////////////////////////////////////////////////////


void
HttpFunctionHandler::addMethod( const MC2String& name, 
                                HttpFunctionNote* n )
{
   m_funcMap.insert( make_pair( name, n ) ); 
}


HttpFunctionHandler::HttpFunctionHandler() {
   // Create the variables for the functions
   variableContainer = new HttpVariableContainer();

   // Insert the function into the map
   // m_funcMap["HtmlName"] = new HttpFunctionNote(htmlFunc, minParams);
   addMethod( "Test", new HttpFunctionNote( 
                 HttpFunctionHandler::htmlTest, 0 ) );
   addMethod( "MakeImage", new HttpFunctionNote(
      HttpMapFunctions::htmlMakeImage, 0 ) );
   addMethod( "NavigatorRoute", new HttpFunctionNote(
      HttpNavigatorFunctions::htmlNavigatorRoute, 0 ) );
   addMethod( "ShowStoredRoute", new HttpFunctionNote(
      HttpRouteFunctions::htmlShowStoredRoute, 0 ) );
   addMethod( "ShowLocalMap", new HttpFunctionNote(
      HttpMapFunctions::htmlShowLocalMap, 0 ) );
   addMethod( "MakeTMapTile", new HttpFunctionNote(
      HttpTMapFunctions::htmlMakeTMapTile, 0 ) );

   // Proxy functions for caching
   addMethod( "Proxy", new HttpFunctionNote(
      HttpProxyFunctions::htmlProxy, 1 ) );

   // Simple poi maps for java and AJAX
   addMethod( "SimplePoiDesc", new HttpFunctionNote(
      HttpSimplePoiFunctions::htmlMakeSimplePoiDesc, 0 ) );
   addMethod( "SimplePoiMap", new HttpFunctionNote(
      HttpSimplePoiFunctions::htmlMakeSimplePoiMap, 0 ) );
   addMethod( "gmap", new HttpFunctionNote(
      HttpMapFunctions::gmap, 0 ) );
   addMethod( "mmap", new HttpFunctionNote(
      HttpMapFunctions::mmap, 0 ) );
   addMethod( "zoomSettings", new HttpFunctionNote(
      HttpMapFunctions::zoomSettings, 0 ) );
}


HttpFunctionHandler::~HttpFunctionHandler() {
  for ( FunctionsMap::iterator funcI = m_funcMap.begin() ;
        funcI != m_funcMap.end() ; ++funcI ) {
    delete funcI->second;
  }
  DEBUG4(cerr << "Deleteting variableContainer" << endl;);
  delete variableContainer;
}


HttpFunctionNote*
HttpFunctionHandler::getFunctionNoteOf(const MC2String& func) {
   FunctionsMap::iterator funcI = m_funcMap.find(func);
   if ( funcI == m_funcMap.end() ) {
      return NULL;
   } else {
      return funcI->second;
   }
}


HttpVariableContainer*
HttpFunctionHandler::getVariableContainer() {
   return variableContainer;
}


bool
HttpFunctionHandler::htmlTest( stringVector* params, 
                                 int paramc, 
                                 stringMap* paramsMap,
                                 HttpHeader* inHead, 
                                 HttpHeader* outHead, 
                                 HttpBody* inBody, 
                                 HttpBody* outBody,
                                 HttpParserThread* myThread, 
                                 HttpVariableContainer* myVar )
{
   // test
   DEBUG4(cerr << "htmlTest nbrparams: " << paramc << " = " << 
	  (params->size()) << endl;);
   for (int i = 0 ; i < paramc ; i ++) {
      //      DEBUG8(cerr << *((*params)[i]) << endl;);
      outBody->addString(*((*params)[i]));
   }
   
   return true; // Ok to parse rest of page
}


//////////////////////////////////////////////////////////////
// HttpFunctionNote
//////////////////////////////////////////////////////////////


HttpFunctionNote::HttpFunctionNote(
   HttpFunctionHandler::htmlFunction* theFunction, 
   uint32 minArguments ) 
{
   m_Function = theFunction;
   m_minArgc = minArguments;
}


HttpFunctionHandler::htmlFunction*
HttpFunctionNote::getFunction() {
   return m_Function;
}


uint32
HttpFunctionNote::getMinArguments() {
   return m_minArgc;
}


//////////////////////////////////////////////////////////////
// HttpVariableContainer
//////////////////////////////////////////////////////////////

HttpVariableContainer::HttpVariableContainer() {

   currentLanguage = StringTable::ENGLISH;

   https = false;
}


HttpVariableContainer::~HttpVariableContainer() {
}
