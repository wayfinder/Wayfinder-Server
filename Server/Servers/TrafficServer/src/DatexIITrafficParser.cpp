/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DatexIITrafficParser.h"

#include "XPathMultiExpression.h"
#include "XMLTool.h"
#include "XMLInit.h"
#include "XMLUtility.h"
#include "ParserThreadGroup.h"

#include "STLUtility.h"

#include "TrafficSituation.h"

#include "DatexIIStructs.h"

#include <sax/HandlerBase.hpp>

#include <memory>

#define DIIPT "[DIIPT:" << __LINE__ << "] "

using namespace DatexIIStructs;

struct DatexIITrafficParser::Impl {
   explicit Impl( const MC2String& provider ):
      m_provider( provider ) { }
   bool parseXMLFile( const char* xmlFile );
   DatexIITrafficParser::Situations m_trafficSituations;
   MC2String m_provider;
};

DatexIITrafficParser::DatexIITrafficParser( const MC2String& provider ):
   TrafficParser( provider ),
   m_impl( new Impl( provider ) ) {
   
}

DatexIITrafficParser::~DatexIITrafficParser() {
   delete m_impl;
}

bool DatexIITrafficParser::Impl::parseXMLFile( const char* xmlFile ) {

   mc2log << info << DIIPT << "Parsing XML-file." << endl;
   MC2String country;
   MC2String messageSender;

   MultiExpression::NodeDescription desc[] = {
      {"exchange/supplierIdentification/country",
       makeAssigner( country ) },
      {"exchange/supplierIdentification/nationalIdentifier", 
       makeAssigner( messageSender ) },
      {"payloadPublication/situation*",
       new SituationExpression( m_trafficSituations, m_provider ) }
   };

   uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] ) ;
   MultiExpression supplierInfo( MultiExpression::
                                 Description( desc, desc + descSize ) );

   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );

   MemBufInputSource xmlBuff( (byte*)xmlFile, 
                              strlen( xmlFile ), "xmlRequest" );

   try {
      parser.parse( xmlBuff );
   } catch (const XMLException& e) {
      MC2String message = XMLUtility::transcodefrom( e.getMessage() );
      mc2log << error << DIIPT << "XML Exception message is: " 
             << message << endl;
   } catch (const DOMException& e) {
      MC2String message = XMLUtility::transcodefrom( e.msg );
      mc2log << error << DIIPT << "DOM Exception message is: " 
             << message << endl;
   } catch (const SAXParseException& e) {
      MC2String message = XMLUtility::transcodefrom( e.getMessage() );
      mc2log << DIIPT << "SAXParseException message is: \n" 
             << message << "\n"
             << e.getMessage() << ", line " << e.getLineNumber() 
             << ", column " << e.getColumnNumber() << endl;
   }

   mc2dbg << "looking for root node " << endl;
   const char* rootName = "d2LogicalModel";
   const DOMNode* root = XMLTool::findNodeConst( parser.getDocument(),
                                                 rootName );
   mc2dbg << "found root node " << endl;

   /* This will evalute a multiexpression built up in this way:
    *  * suplierInfo
    *  |
    *  |-> * SituationExpression
    *      |
    *      |-> * AccidentExpression : SituationRecordExpression
    *      |   |-> * AlertCMethod4Expression
    *      |   |-> * AlertCMethod2Expression
    *      |
    *      |-> * NetworkManagementExpression : SituationRecordExpression
    *      |   |-> * AlertCMethod4Expression
    *      |   |-> * AlertCMethod2Expression
    *      |
    *      |-> * MaintenanceWorksExpression : SituationRecordExpression
    *      |   |-> * AlertCMethod4Expression
    *      |   |-> * AlertCMethod2Expression
    *      |
    *      |-> * ConstructionWorksExpression : SituationRecordExpression
    *      |   |-> * AlertCMethod4Expression
    *      |   |-> * AlertCMethod2Expression
    *      |
    *      |-> * AbnormalTrafficExpression : SituationRecordExpression
    *      |   |-> * AlertCMethod4Expression
    *      |   |-> * AlertCMethod2Expression
    *      |
    *      |-> * Activities : SituationRecordExpression
    *      |   |-> * AlertCMethod4Expression
    *      |   |-> * AlertCMethod2Expression
    *      |
    *      |-> * AnimalPresenceObstruction : SituationRecordExpression
    *      |   |-> * AlertCMethod4Expression
    *      |   |-> * AlertCMethod2Expression
    *      |
    *      |-> * GeneralObstruction : SituationRecordExpression
    *      |   |-> * AlertCMethod4Expression
    *      |   |-> * AlertCMethod2Expression
    *      |
    *      |-> * NonWeatherRelatedRoadConditions : SituationRecordExpression
    *      |   |-> * AlertCMethod4Expression
    *      |   |-> * AlertCMethod2Expression
    *      |
    *      |-> * VehicleObstruction : SituationRecordExpression
    *          |-> * AlertCMethod4Expression
    *          |-> * AlertCMethod2Expression
    */

   // Check for the root node
   if ( root == NULL ) {
      mc2log << warn << DIIPT << "could not find root node: " 
             << rootName << endl;
      return false;
   } else { 
      supplierInfo.evaluate( root );
   }

   // Check if there is a country and a supplier info in the data
   if ( country.empty() || messageSender.empty() ) {
      mc2log << warn << DIIPT << " not a valid DatexII format."
             << " Supplier identification data incorrect." << endl;
      return false;
   }

   mc2log << info << DIIPT << "Reference Name: " << messageSender
          << " Country: " << country << endl;

   return true;
}

bool DatexIITrafficParser::
parse( const MC2String& data, Situations& situations ) {

   if ( ! m_impl->parseXMLFile( data.c_str() ) ) {
      return false;
   }

   mc2log << DIIPT << "File parsed OK" << endl;
   mc2dbg << DIIPT << "Number of traffic situations before removing: "
          << m_impl->m_trafficSituations.size() << endl;

   uint32 timeNow = time( NULL );

   // Delete traffic situations with end time that has expired
   for ( vector< TrafficSituation* >::iterator 
            it = m_impl->m_trafficSituations.begin();
         it != m_impl->m_trafficSituations.end(); ) {
      const vector< TrafficSituationElement* >& tseVect =
         (*it)->getSituationElements();
      // Check if the expiry time is expired, if so delete the traffic
      // situation. If not just skip it.
      if ( timeNow > tseVect.front()->getExpiryTime() ) {
         mc2dbg4 << DIIPT << "End time passed, removing: "
                 << (*it)->getSituationReference() << endl;
         delete *it;
         it = m_impl->m_trafficSituations.erase( it );
      } else {
         ++it;
      }
   }

   mc2log << DIIPT << m_impl->m_trafficSituations.size() << " traffic situations"
          << " left to handle." << endl;

   situations.swap( m_impl->m_trafficSituations );

   STLUtility::deleteValues( m_impl->m_trafficSituations );
   return true;
}
