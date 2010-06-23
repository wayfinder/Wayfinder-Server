/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CellIDLookUp.h"
#include "TimeUtility.h"
#include "URL.h"
#include "URLFetcher.h"
#include "HttpHeader.h"
#include "HttpStringMap.h"
#include "HttpParserThreadUtility.h"
#include "Properties.h"

#include "URLParams.h"
#include "WGS84Coordinate.h"
#include "DebugClock.h"
#include "ParserThreadGroup.h"

#include "boost/lexical_cast.hpp"

#include "XPathMultiExpression.h"
#include "XPathAssigner.h"
#include "XMLTool.h"
#include <sax/SAXParseException.hpp>
#include "PropertyHelper.h"
#include "STLStringUtility.h"

#include <map>
#include <memory>


namespace CellIDLookUp {

typedef MC2String ParameterName;
typedef MC2String Value;
typedef map< ParameterName, Value > CellIDRequestData;

void addRequestData( CellIDRequestData& data,
                     const MC2String& name, 
                     const MC2String& value ) {
   if ( ! value.empty() ) {
      data[ name ] = value;
   }
}

MC2String toDecNumber( const MC2String& str ) {
   uint32 nbr = STLStringUtility::strtoul( str );
   return  boost::lexical_cast< MC2String >( nbr );
}

void getRequestData( const CellIDRequest& req, CellIDRequestData& data ) {
   addRequestData( data, "mcc", toDecNumber( req.m_mcc ) );
   addRequestData( data, "mnc", toDecNumber( req.m_mnc ) );
   //TODO: Find a better way to check if the cell id is 16bit or 32bits
   if ( req.m_networkType == "GPRS" ) { // || req.m_cellID.size() <= 4 ) {
      addRequestData( data, "lac", toDecNumber( req.m_lac ) );
   }
   addRequestData( data, "cellid", toDecNumber( req.m_cellID ) );
}

MC2String addURLEncodedParameters( const CellIDRequestData& data ) {
   URLParams params;
   // for convinience
   typedef CellIDRequestData::const_iterator MapIt;
   for ( MapIt it = data.begin(); it != data.end(); ++it ) {
      params.add( it->first.c_str(), it->second );
   }

   return params;
}

bool parseAnswer( const MC2String& reply, CellIDReply& rep )
try {
/*
<rsp stat="ok">
   <cell range="6000" lac="0" lat="53.1349470453681" nbSamples="51" lon="25.2746703877572" cellId="29513" mcc="250" mnc="99"/>
</rsp>

<rsp stat="ok">
   <cell range="50000" lac="" lat="0.0" nbSamples="0" lon="0.0" cellId="" mcc="" mnc=""/>
</rsp>

<rsp stat="fail">
   <err info=" cell not found" code="1"/>
</rsp>
*/
   bool ok = false;
   if ( ! reply.empty() ) {
      MC2String stat;
      uint32 range;
      float64 lat;
      float64 lon;
      uint32 nbrSamples = 0;
      MC2String err;
      uint32 errCode = 0;

      using namespace XMLTool;
      using namespace XPath;
      using XMLTool::XPath::MultiExpression;

      MultiExpression::NodeDescription desc[] = {
         { "rsp/@stat", makeAssigner( stat ) },
         { "rsp/cell/@range", makeAssigner( range ) },
         { "rsp/cell/@lat", makeAssigner( lat ) },
         { "rsp/cell/@lon", makeAssigner( lon ) },
         { "rsp/cell/@nbSamples", makeAssigner( nbrSamples ) },
         { "rsp/err/@info", makeAssigner( err ) },
         { "rsp/err/@code", makeAssigner( errCode ) }
      };
      uint32 size = sizeof ( desc ) / sizeof ( desc[ 0 ] );

      MultiExpression expression( MultiExpression::
                                  Description( desc, desc + size ) );


      XercesDOMParser xmlParser;
      MemBufInputSource xmlBuff( (byte*)(reply.data()), reply.size(), 
                                 "CellIDReply" );
      xmlParser.parse( xmlBuff );
      if ( ! xmlParser.getDocument() ) {
         mc2dbg << warn << "[CellIDLookUp] Failed to parse document." << endl;
      } else {
         expression.evaluate( xmlParser.getDocument() );      

         if ( stat == "ok" && nbrSamples > 0 ) {
            rep.m_coord = MC2Coordinate( WGS84Coordinate( lat, lon ) );
            rep.m_outerRadius = range;
            ok = true;
         } else {
            rep.m_errorStr = MC2String( "Status: " ) + stat + " Info: " + err +
               " Code: " + boost::lexical_cast< MC2String >( errCode );
            mc2log << warn << "[CellIDLookUp] Not ok reply: " 
                   << rep.m_errorStr << endl;
         }
      }
   }

   return ok;
} catch ( const MC2Exception& e ) {
   mc2log << error
          << "[CellIDLookUp]: Exception when parsing reply: " << e
          << endl;
   return false;
} catch ( const XMLException& e ) {
   mc2log << error
          << "[CellIDLookUp]: an XMLerror occured "
          << "during parsing of cellid reply: "
          << e.getMessage() << " line "
          << e.getSrcLine() << endl;
   return false;
} catch ( const SAXParseException& e) {
   mc2log << error
          << "[CellIDLookUp]: an SAXerror occured "
          << "during parsing of cellid reply:: "
          << e.getMessage() << ", "
          << "line " << e.getLineNumber() << ", column "
          << e.getColumnNumber() << endl;
   return false;
}

bool cellIDLookUp( const CellIDRequest& req, 
                   CellIDReply& rep, 
                   ParserThreadGroup* group ) {
   if ( ! PropertyHelper::get( "ENABLE_OPENCELLID_INTEGRATION", false ) ) {
      // Not enabled
      mc2log << info << "[CellIDLookUp] OpenCellID inetration not enabled, "
             << "no cell id lookup done." << endl;
      return false;
   }
   const char* apiKey = NULL;
   apiKey = Properties::getProperty( "OPENCELLID_KEY" );
   if ( apiKey == NULL || *apiKey == '\0' ) {
      // Not enabled
      mc2log << info << "[CellIDLookUp] No OpenCellID API key in properties, "
             << "no cell id lookup done." << endl;
      return false;
   }

   // A map for holding request parameters.
   CellIDRequestData data;
   getRequestData( req, data );
   // Add API key to request
   addRequestData( data, "key", apiKey );

   // Create the parameters
   MC2String params = addURLEncodedParameters( data );

   // for convinience
   typedef CellIDRequestData::const_iterator MapIt;

   MC2String reply;
   uint32 commTimeOut = 5000; // ms

   MC2String url = "http://www.opencellid.org/cell/get";
   // Add the parameters to the url
   url += params;

   URLFetcher fetcher;
   // For id in print-outs
   MC2String idStr(" ID:");
   DebugClock clock;
   int ures = 0;
   idStr += boost::lexical_cast< MC2String >( rand() );
   bool logSocketInformation = true;
   bool debugPrints = true;

   if ( ! logSocketInformation ) {
      ures = fetcher.post( reply, 
                           URL( url ),
                           "",
                           commTimeOut );

   } else {
      ures = fetcher.postWithLogging( reply,
                                      URL( url ),
                                      "",
                                      commTimeOut,
                                      NULL/*inHeaders*/,
                                      "CellIDLookUp::cellIDLookUp" + idStr );
   }

   if ( debugPrints ) {
      mc2log << "[CellIDLookUp]  cellIDLookUp request took " << clock 
             << " for " << idStr << endl;
      mc2log << "[CellIDLookUp]  Parameters: " << params << endl;
      mc2log << "[CellIDLookUp]  Reply: " << reply << endl;
   }

   if ( reply.empty() ) {
      // no reply from server.
      mc2log << warn << "[CellIDLookUp] Reply empty for " << idStr
             << " HTTP status code " << MC2CITE( ures ) << endl;
      rep.m_errorStr = "Empty reply, res = \"" + 
         boost::lexical_cast< MC2String >( ures ) + "\", in " + 
         boost::lexical_cast< MC2String >( clock.getTime() ) + " ms";
      return false;
   }

   // Parse the answer.
   stringMap paramsMap;
   return parseAnswer( reply, rep );
}


} // End namespace CellIDLookUp
