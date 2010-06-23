/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PreLoadMapModule.h"

#include "StringUtility.h"
#include "CommandlineOptionHandler.h"
#include "LoadMapPacket.h"
#include "JobThread.h"

PreLoadMapModule::PreLoadMapModule( moduletype_t modType,
                                    int argc, char** argv ):
   Module( modType, argc, argv, true) // using maps
{
}

PreLoadMapModule::PreLoadMapModule( moduletype_t modType,
                                    CommandlineOptionHandler* handler ):
   Module( modType, handler, true ) // using maps
{ 
}

PreLoadMapModule::~PreLoadMapModule() 
{
}

void PreLoadMapModule::parseCommandLine() throw ( ComponentException ) {  
   char* mapsToPreloadString = NULL;
   CommandlineOptionHandler& coh = *m_cmdlineOptHandler;

   coh.addOption( "-M", "--preloadmaps",
                  CommandlineOptionHandler::stringVal,
                  1, &mapsToPreloadString, "",
                  "Load comma-separated maps before starting reader."
                  " BUG: If one map should be loaded it must be listed"
                  " twice e.g 0,0");

   Module::parseCommandLine();

   vector<uint32> mapsToPreload;

   StringUtility::parseCommaSepInts( mapsToPreload, 
                                     mapsToPreloadString );
   if ( !mapsToPreload.empty() ) {
      createProcessor();
      // Load the maps, depending on which courier setup is used the
      // last map might be in a "loading" state when this loop finish.
      for ( uint32 i=0; i < mapsToPreload.size(); ++i ) {
         m_jobThread->
            processPacket( new LoadMapRequestPacket( mapsToPreload[ i ] ) );
      }

   }
}


