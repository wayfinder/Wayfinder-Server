/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "Request.h"
#include "RequestParser.h"
#include "XMLInit.h"
#include "CommandlineOptionHandler.h"
#include "StringUtility.h"

#include "NGPMakerGUI.h"

using namespace NGPMaker;

// dumps data about a parameter
ostream& operator << ( ostream& ostr, const NGPMaker::Param& param ) {
   ostr << "Id: " << param.m_id << endl
        << "Type: " << param.m_type << endl
        << "Value: " << param.m_value << endl;
   return ostr;
}

// dumps data about a request
ostream& operator << ( ostream& ostr, const NGPMaker::Request& req ) {
   ostr << "Protocol: " << hex << "0x" << req.m_protocolVersion << dec << endl
        << "type: " << hex << "0x" << req.m_type << dec << endl
        << "id: " << req.m_id << endl
        << "version: " << req.m_version << endl
        << "useGzip: " << req.m_useGzip << endl
        << endl;
   if ( ! req.m_params.empty() ) {
      ostr << "Params: " << endl;
      for ( uint32 i = 0; i < req.m_params.size(); ++i ) {
         ostr << req.m_params[ i ];
      }
   }
   return ostr;
}
/**
 * Command line interface application
 * @param infile The infile that describes request in xml format
 * @param outfile The outbinary
 * @return return value to system
 */
int cliApp( const MC2String& infile,
            const MC2String& outfile );

/**
 * Graphic user interface application with Gtk+
 * @param infile The infile that describes request in xml format
 * @return return value to system
 */
int guiApp( const MC2String& infile );

/// shows usage for this application
void showUsage( const char* appname ) {
   cerr << appname << ": Error on commandline! (Use -h for help) " << endl;
}

/*
 * NGPMaker
 * Can use both CLI and GUI for editing and creating
 * NGP binaries which can be sent to the NavigatorServer.
 *
 */
int main( int argc, char** argv ) {

   CommandlineOptionHandler coh( argc, argv, 1 );
   coh.setTailHelp(" request.xml ( outfile.bin ) \n"
                   " outfile.bin is only needed for non-gui app.");
   coh.setSummary("Parses request descriptions and creates ngp binary.");

   bool useGUI = false;
   coh.addOption( "", "--gui",
                  CommandlineOptionHandler::presentVal,
                  1, &useGUI, "false",
                  "Use GUI interface." );

   if ( ! coh.parse() ) {
      showUsage( argv[ 0 ] );
      return 1;
   }
   

   if ( useGUI ) {
      // Use GUI application

      if ( coh.getTailLength() != 1 ) {
         showUsage( argv[ 0 ] );
         return 1;
      }
      return guiApp( coh.getTail( 0 ) );

   } else {
      // do non-GUI application, needs to arguments

      if ( coh.getTailLength() != 2 ) {
         showUsage( argv[ 0 ] );
         return 1;
      }

      return cliApp( coh.getTail( 0 ), coh.getTail( 1 ) );
   }
} 


int cliApp( const MC2String& infile,
            const MC2String& outfile ) {
   // initialize xml for parsing
   XMLTool::XMLInit xmlInit;

   RequestParser parser;
   if ( ! parser.parse( infile ) ) {
      mc2log << "Failed to parse: " << infile << endl;
      return 1;
   }
   const RequestParser::Requests& reqs = parser.getRequests(); 

   for ( uint32 i = 0; i < reqs.size(); ++i ) {
      cerr << reqs[ i ] << endl;
   }

   if ( ! reqs.empty() ) {
      // use the first request, for now..
      if ( ! NGPMaker::saveNGP( outfile, reqs[ 0 ] ) ) {
         mc2log << "Failed to write to: " << outfile << endl;
         return 2;
      } else {
         mc2log << "Wrote ngp to: " << outfile << endl;
      }
   }

   return 0;
}


int guiApp( const MC2String& infile ) {
   
   char* argv[] = { StringUtility::newStrDup( "NGPMaker" ) };
   int argc = 1;
   mc2dbg << "Starting gui." << endl;
   MakerGUI gui( argc, argv );

   mc2dbg << "Parsing request file \"" 
          << infile << "\"" << endl;

   gui.parseRequestFile( infile );

   gui.show();

   gui.loop();

   delete [] argv[ 0 ];

   return 0;
}
