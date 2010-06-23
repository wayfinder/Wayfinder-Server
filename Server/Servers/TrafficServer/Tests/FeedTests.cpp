/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Parsers
#include "DatexIITrafficParser.h"

// Feeds
#include "FileTrafficFeed.h"

#include "TrafficSituation.h"

#include "DeleteHelpers.h"
#include "CommandlineOptionHandler.h"
#include "Properties.h"
#include "XMLInit.h"

/// Creates a traffic parser from a specific format
TrafficParser* createParser( uint32 format ) {
   switch ( format ) {
   case 0:
      return new DatexIITrafficParser( "SRA_AS" );
   case 1:
      return new DatexIITrafficParser( "SRA_EIS" );
   case 2:
      return new DatexIITrafficParser( "SRA_RWS" );
   case 3:
      return new DatexIITrafficParser( "SRA_TMS" );
   }

   return NULL;
}

int main( int argc, char** argv ) {
   CommandlineOptionHandler coh( argc, argv );
   typedef pair< MC2String, MC2String > OptionPair;

   uint32 format = MAX_UINT32;

   const OptionPair options[] = {
      make_pair( "--datexii_as", "VÃ¤gverket DatexII Accident Service" ),
      make_pair( "--datexii_eis", "VÃ¤gverket DatexII Emergency Info Service" ),
      make_pair( "--datexii_rws", "VÃ¤gverket DatexII Road Work Service" ),
      make_pair( "--datexii_tmx", "VÃ¤gverket DatexII Traffic Message Service" ),
   };

   coh.addSingleChoiceOption( "-f", 
                              options,
                              options + sizeof (options) / sizeof (*options),
                              &format, MAX_UINT32,
                              "Sets the type of data to read." );

   coh.setTailHelp("[files]");

   if ( ! coh.parse() ) {
      mc2log << error << "Failed to parse command line!" << endl;
      return 1;
   }
   
   if ( ! Properties::setPropertyFileName( coh.getPropertyFileName() ) ) {
      mc2log << error << "TrafficServer: No such file or directory: '"
             << coh.getPropertyFileName() << "'" << endl;
      return 2;
   }

   // Initialize the XML system
   XMLTool::XMLInit xmlInit;

   vector<MC2String> files;
   for ( uint32 i = 0; i < coh.getTailLength(); ++i ) {
      files.push_back( coh.getTail( i ) );
   }


   FileTrafficFeed feed( files );
   // we need files!
   if ( feed.eos() ) {
      mc2log << error << "No feed!" << endl;
      return 3;
   }

   auto_ptr<TrafficParser> parser( createParser( format ) );
   
   if ( ! parser.get() ) {
      mc2log << error << "Failed to create parser" << endl;
      return 4;
   }

   while ( ! feed.eos() ) {

      // get data and parse situations

      TrafficFeed::Data data;
      if ( ! feed.getData( data ) ) {
         mc2log << error << "Failed to get feed." << endl;
         break;
      }

      TrafficParser::Situations situations;
      if ( ! parser->parse( data, situations ) ) {
         mc2log << error << "Failed to parse feed." << endl;
         return 5;
      }

      if ( situations.empty() ) {
         mc2log << error << "No situations parsed from feed." << endl;
         return 6;
      }

      mc2log << "Situations parsed: " << situations.size() << endl;

      STLUtility::deleteValues( situations );
   }

   return 0;
}
