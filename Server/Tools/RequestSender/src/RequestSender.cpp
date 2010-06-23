/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <xercesc/util/PlatformUtils.hpp>

#include "CommandlineOptionHandler.h"
#include "Properties.h"
#include "URLFetcherNoSSL.h"
#include "URL.h"
#include "TempFile.h"

#include "XMLUIParser.h"
#include "SlotHandler.h"
#include "EntryUtil.h"
#include "Replacer.h"

#include <sstream>
#include <fstream>

void sendFile( const MC2String& server, const MC2String& port,
               const MC2String& filename, const MC2String& resultFile =
               MC2String() );

using namespace MC2Gtk;

class RequestSender {
public:
   RequestSender( const char* replaceFile,
              const char* xmlFilename ):
      m_replacer( replaceFile, xmlFilename )  {
      
      m_slots.connect( G_OBJECT( XMLUI::UIParser::getWidget( "send_button" ) ),
                       "clicked", Slot( *this, &RequestSender::search ) );

   }

   void search() try {
      TempFile destFile( "request", "/tmp" );

      m_replacer.replace( destFile.getTempFilename() );
      sendFile( MC2Gtk::XMLUI::getEntryValue( "server" ), 
                MC2Gtk::XMLUI::getEntryValue( "port" ),
                destFile.getTempFilename() );
      
   } catch ( const FileUtils::FileException& e ) {
      mc2log << error << e.what() << endl;
   } catch ( const XMLUI::Exception& e ) {
      mc2log << error 
             << "Seems to be a problem with user interface xml" << endl;
      mc2log << "Check the user interface xml file. " << endl;
      mc2log << e.what() << endl;
   }

   void run() {
      gtk_main();
   }

private:
   SlotHandler m_slots;
   MC2Gtk::Replacer m_replacer;
}; 

void showUsage( const char* program ) { 
   cout << "Usage: " << program << " requestTemplateFilename guiFilename" 
        << endl;
}
int main( int argc, char **argv ) { 
   try {
      XMLPlatformUtils::Initialize();
   } catch(const XMLException &toCatch) {
      cerr << "Error.\n"
           << "  Exception message:"
           << toCatch.getMessage() << endl;
      return 1;
   }
   gtk_init( &argc, &argv );

   CommandlineOptionHandler coh( argc, argv, 2 );
   coh.setTailHelp(" requestfile guifile");
   coh.setSummary("Sends request to xml server. Requires at least a button"
                  " with id=\"send_button\" in the gui file");

   if ( ! coh.parse() ) {
      cerr << argv[ 0 ] << ": Error on commandline! (Use -h for help) " <<endl;
      return -1;
   }

   if ( ! Properties::setPropertyFileName( coh.getPropertyFileName() ) ) {
      cerr << "No such file or directory: '"
           << coh.getPropertyFileName() << "'" << endl;

   }

   try {

      RequestSender sender( coh.getTail( 0 ), coh.getTail( 1 ) );
      sender.run();

   } catch ( const MC2Gtk::XMLUI::Exception& e ) {
      mc2log << fatal << "Error while creating gui: " << e.what() << endl;
   } catch ( const std::exception& e ) {
      mc2log << fatal << "Error: " << e.what() << endl;
   } catch ( ... ) {
      mc2log << fatal << "Unknown error!" << endl;
   }

   XMLPlatformUtils::Terminate();
   return 0;

}

void sendFile( const MC2String& server, const MC2String& port,
               const MC2String& filename, const MC2String& resultFile ) {
   MC2String url("http://");
   url += server + ":" + port + "/";
   
   URLFetcherNoSSL fetcher;
   MC2String answer;
   ifstream infile( filename.c_str() );
   if ( ! infile ) {
      return;
   }
   stringstream str;
   str << infile.rdbuf();
   uint32 result = fetcher.post( answer, URL(url), MC2String( str.str() ) );
   mc2dbg << "got result: " << result << " with url: " << url << endl;
   if ( result < 0 ) {
      return;
   }
   if ( ! resultFile.empty() ) {
      ofstream outfile( resultFile.c_str() );
      outfile << answer;
   } else {
      ofstream outfile( "/dev/stdout" );
      outfile << answer;
   }
}
