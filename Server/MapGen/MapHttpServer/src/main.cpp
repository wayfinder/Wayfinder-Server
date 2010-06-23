/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapHttpServer.h"
#include "ISABThread.h"
#include "CommandlineOptionHandler.h"
#include "Properties.h"
#include "Convert.h"
#include "PropertyHelper.h"
#include "StringUtility.h"

#include <cstdlib>
#include <iostream>
using namespace std;

int main( int argc, char **argv ) try {

   CommandlineOptionHandler coh( argc, argv );

   uint32 preferredPort = 0;
   bool convertAll = false;
   char* mapsToConvertString = NULL;
   char* mapIDsToConvertString = NULL;

   coh.addOption( "-t", "--port",
                  CommandlineOptionHandler::uint32Val,
                  1, &preferredPort, "0",
                  "Set the preferred port number. This value, "
                  "if present, overrides the default one in the "
                  "property file." );

   coh.addOption( "-s", "--convertMapSets",
                  CommandlineOptionHandler::stringVal,
                  1, &mapsToConvertString, "",
                  "Comma separated mapSets to convert." );

   coh.addOption( "-a", "--convertAll",
                  CommandlineOptionHandler::presentVal,
                  1, &convertAll, "F",
                  "Load index.db and convert all maps inside it." );

   coh.addOption( "-M", "--convertMap",
                  CommandlineOptionHandler::stringVal,
                  1, &mapIDsToConvertString, "",
                  "The map IDs to make maps for." );

   if( !coh.parse() ){
      cerr << "MapHttpServer: Error on commandline! (-h for help)" << endl;
      exit(1);
   }

   std::vector<uint32> mapsToConvert;
   StringUtility::parseCommaSepInts( mapsToConvert, mapsToConvertString );

   mc2dbg << "mapIDsToConvertString " << mapIDsToConvertString << endl;
   std::vector<uint32> mapIDsToConvert;
   StringUtility::parseCommaSepInts( mapIDsToConvert, mapIDsToConvertString );
   mc2dbg << "mapsToConvert " << mapIDsToConvert.size() << endl;

   if ( !mapIDsToConvert.empty() ) {
      Convert::convertSet( mapsToConvert, mapIDsToConvert );
      return 0;
   }

   if ( convertAll  ) {
      Convert::convertAll();
      return 0;
   }
   
   // if we did not set a listen port in the command line 
   // then we must have property MAP_SERV_PORT
   if ( !convertAll && preferredPort == 0 ) {
      cout << "No port specified, using default." << endl;
      preferredPort = PropertyHelper::get<int>("MAP_SERV_PORT");
   }
   
   PropertyHelper::get<MC2String>("MAP_SERV_SOURCE_PATH");
   PropertyHelper::get<MC2String>("MAP_SERV_DEST_PATH");

   JTCInitialize init;
   MapHttpServer server( preferredPort, convertAll );
   server.run(); 

} catch (MC2String err) {

   cerr << "Error: " << err <<endl;

   return EXIT_FAILURE;
} catch ( const PropertyException& e ) {
   cerr << "Error: " << e.what() << endl;
   return EXIT_FAILURE;
} catch ( ... ) {
   cerr << "Unknown exception!" << endl;
   return EXIT_FAILURE;
}
