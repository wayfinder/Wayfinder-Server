/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Mercator.
 *
 * Do stuff related to Mercator maps.
 */

#include "config.h"
#include "ISABThread.h"
#include "CommandlineOptionHandler.h"
#include "URLFetcherNoSSL.h"
#include "STLStringUtility.h"
#include "URL.h"
#include "XMLUtility.h"
#include "StringConvert.h"
#include "XMLTool.h"
#include "XMLInit.h"
#include "XPathMultiExpression.h"
#include "LangTypes.h"
#include "DebugClock.h"

typedef XMLTool::XPath::MultiExpression::NodeEvaluator Evaluator;

class ZoomLevelGetter :  public Evaluator {
public:
   explicit ZoomLevelGetter( URLFetcherNoSSL& f,
                             const char* host,
                             uint16 hostPort,
                             uint32 pixel_size,
                             uint32 maxZoomLevel,
                             vector<LangTypes::language_t>& langs,
                             const MC2String& other = MC2String() ):
      m_info( other ), m_f( f ), CL_host( host ), CL_hostPort( hostPort ),
      CL_pixel_size( pixel_size ), CL_maxZoomLevel( maxZoomLevel ),
      CL_langs( langs )
      { }

   void operator() ( const DOMNode* node ) {
      mc2log << m_info << " Node \"" << node->getNodeName() << "\""
             << endl;
      using namespace XMLTool;
      //<zoom_level 
      // max_x="720" max_y="720" min_x="-900" min_y="-900" zoom_level_nbr="1"/>
      // max_x
      int32 max_x;
      getAttrib( max_x, "max_x", node );
      // max_y
      int32 max_y;
      getAttrib( max_y, "max_y", node );
      // min_x
      int32 min_x;
      getAttrib( min_x, "min_x", node );
      // min_y
      int32 min_y;
      getAttrib( min_y, "min_y", node );
      // zoom_level_nbr
      int32 zoom_level_nbr;
      getAttrib( zoom_level_nbr, "zoom_level_nbr", node );

      mc2log << "  max_x " << max_x << " max_y " << max_y << " min_x "
           << min_x << " min_y " << min_y << " zoom_level_nbr "
           << zoom_level_nbr << endl;
      if ( uint32(zoom_level_nbr) > CL_maxZoomLevel ) {
         return;
      }
      if ( CL_langs.empty() ) {
         for ( int l = LangTypes::english ;
               l < LangTypes::nbrLanguages ; ++l ) {
            CL_langs.push_back( LangTypes::language_t( l ) );
         }
      }

      for ( int32 x = min_x ; x <= max_x ; x += CL_pixel_size ) {
         for ( int32 y = min_y ; y <= max_y ; y += CL_pixel_size ) {
            for ( size_t i = 0 ; i < CL_langs.size() ; ++i ) {
               const char* langStr = LangTypes::getLanguageAsISO639( 
                  CL_langs[ i ] );
               if ( *langStr == '\0' || 
                    CL_langs[ i ] == LangTypes::invalidLanguage ) {
                  // No need for so unknown language
                  continue;
               }
               //MMap?x=1260&y=2160&zoom=4&lang=en
               char file[ 1024 ];
               sprintf( file, "MMap?x=%d&y=%d&zoom=%d&lang=%s",
                        x, y, zoom_level_nbr, langStr );
               MC2String reply;
               DebugClock t;
               int ures = m_f.get( 
                  reply, URL( MC2String( "http://" ) + CL_host + ":" + 
                              STLStringUtility::uint2str( CL_hostPort )
                              + "/" + file ), 25000 );
               if ( ures != 200 ) {
                  mc2log << "    FAILED to get " << file << " res " << ures 
                         << endl;
               } else {
                  mc2log << "    Got " << file << " OK " << reply.size() 
                         << " bytes in " << t << endl;
               }
            } // For all languages
         } // For all y rows
      } // For all x coloumns
   }

private:
   MC2String m_info;
   URLFetcherNoSSL& m_f; 
   const char* CL_host;
   uint16 CL_hostPort;
   uint32 CL_pixel_size;
   uint32 CL_maxZoomLevel;
   vector<LangTypes::language_t> CL_langs;
};

int main( int argc, char** argv ) {
   ISABThreadInitialize init;
   XMLTool::XMLInit xmlInit;

   // Options
   char* CL_host = NULL;
   uint32 CL_hostPort = 0;
   uint32 CL_maxZoomLevel = MAX_UINT32;
   char* CL_languages = NULL;
   auto_ptr<CommandlineOptionHandler> 
      realCoh( new CommandlineOptionHandler( argc, argv ) );

   CommandlineOptionHandler& coh = *realCoh;
   coh.setSummary( "Does stuff related to mercator" );

   coh.addOption( "-s", "--host",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_host, "\0",
                  "Sets the host to make mercator requests to." );

   coh.addOption( "-t", "--port",
                  CommandlineOptionHandler::uint32Val,
                  1, &CL_hostPort, "0",
                  "Sets the port to make mercator requests to." );

   // Lang!? If needed or langs if specific languages
   coh.addOption( "-l", "--languages",
                  CommandlineOptionHandler::stringVal,
                  1, &CL_languages, "\0",
                  "Sets the languages to make mercator requests for." );

   // Max zoom level!?
  coh.addOption( "-m", "--max_zoom_level",
                  CommandlineOptionHandler::uint32Val,
                  1, &CL_maxZoomLevel, "-1",
                 "Sets the highest zoomlevel to get, say 15 gives 1-15." );

   if ( !coh.parse() ) {
      return 1;
   }
   mc2dbg8 << "CL_host " << CL_host << endl;
   mc2dbg8 << "CL_hostPort " << CL_hostPort << endl;
   mc2dbg8 << "CL_maxZoomLevel " << CL_maxZoomLevel << endl;
   mc2dbg8 << "CL_languages " << CL_languages << endl;

   if ( CL_host == NULL ) {
      mc2log << fatal << "Must have host!" << endl;
      return 1;
   }

   vector<LangTypes::language_t> langs;
   if ( CL_languages != NULL && *CL_languages != '\0' ) {
      vector<MC2String> langStrings = STLStringUtility::explode(
         ",", CL_languages );
      for ( uint32 i = 0 ; i < langStrings.size() ; ++i ) {
         LangTypes::language_t l = LangTypes::getISO639AsLanguage( 
            langStrings[ i ].c_str(), true );
         // LangUtility::getStringAsLanguage( langStrings[ i ] );
         if ( l == LangTypes::invalidLanguage ) {
            mc2log << fatal << "Bad language " << langStrings[ i ] << endl;
            return 1;
         }
         langs.push_back( l );
      }
   }
   
   URLFetcherNoSSL f; 
   MC2String reply;
   int ures = f.get( 
      reply, URL( MC2String( "http://" ) + CL_host + ":" + 
                  STLStringUtility::uint2str( CL_hostPort )
                  + "/ZoomSettings" ), 5000 );
   
   if ( ures == 200 ) {
      mc2dbg8 << "Got ZoomSettings:" << endl << reply << endl;
      using namespace XMLTool;
      using namespace XPath;
      XercesDOMParser parser;
      parser.setValidationScheme( XercesDOMParser::Val_Auto );
      parser.setIncludeIgnorableWhitespace( false );

      MemBufInputSource xmlBuff( (byte*)reply.c_str(), 
                                 reply.size(), "ZoomSettings" );
      parser.parse( xmlBuff );

      const char* rootName = "zoom_levels";
      const DOMNodeList* nodes = parser.getDocument()->getElementsByTagName(
         X( rootName ) );

      if ( nodes->getLength() == 1 ) {
         // Get 180 from zoom_levels/pixel_size="180"
         uint32 pixel_size = 180;
         getAttrib( pixel_size, "pixel_size", nodes->item( 0 ) );

         MultiExpression::NodeDescription desc[] = { 
            { "/zoom_level*", new ZoomLevelGetter( 
               f, CL_host, CL_hostPort, pixel_size, CL_maxZoomLevel,
               langs ) },
         };

         MultiExpression expr( MultiExpression::
                               Description( desc,
                                            desc +
                                            sizeof ( desc ) / 
                                            sizeof ( desc[ 0 ] ) ) );
         // mc2dbg << expr << endl;
         expr.evaluate( nodes->item( 0 ) );
      } else { 
         mc2log << warn << "could not find " << rootName << " node." << endl;
      }
   } else {
      mc2log << warn << "Failed to get ZoomSetting from " << CL_host
             << ":" << CL_hostPort << " res " << ures << endl;
   }

   // Await termination
   init.waitTermination();

   coh.deleteTheStrings();

   return 0;
}
