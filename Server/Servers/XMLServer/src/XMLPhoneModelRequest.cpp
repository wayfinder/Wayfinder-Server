/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"


#ifdef USE_XML
#include "CellularPhoneModelsElement.h"
#include "XMLServerElements.h"

using XMLServerUtility::appendStatusNodes;
namespace {
/**
 * Appends a number of phone_manufacturers as children to cur.
 *
 * @param cur Where to put the phone_manufacturer nodes.
 * @param reply The document to make the nodes in.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param models The CellularPhoneModelsElement with data.
 */
void appendPhoneManufacturerList( DOMElement* cur, DOMDocument* reply,
                                  int indentLevel, bool indent, 
                                  CellularPhoneModelsElement* models );

/**
 * Appends a number of phone_models as children to cur.
 *
 * @param cur Where to put the phone_model nodes.
 * @param reply The document to make the nodes in.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param models The CellularPhoneModelsElement with data.
 * @param manufacturer The specific manufacturer.
 * @param hasManufacturer If specific manufacturer should be
 *                        selected.
 */
void appendPhoneModelList( DOMElement* cur, DOMDocument* reply,
                          int indentLevel, bool indent, 
                          CellularPhoneModelsElement* models,
                          const MC2String& manufacturer, bool hasManufacturer );

}

bool 
XMLParserThread::xmlParsePhoneManufacturerRequest( DOMNode* cur, 
                                                   DOMNode* out,
                                                   DOMDocument* reply,
                                                   bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   MC2String errorCode;
   MC2String errorMessage;

   // Create phone_manufacturer_reply element
   DOMElement* phone_manufacturer_reply = 
      reply->createElement( X( "phone_manufacturer_reply" ) );
   // Transaction ID
   phone_manufacturer_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( phone_manufacturer_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), 
         phone_manufacturer_reply );
   }


   // Nothing to read from request


   CellularPhoneModelsElement* models = 
      m_group->getCurrentCellularPhoneModels( this );
   
   if ( models->getNbrManufacturers() > 0 ) {
      // phone_manufacturer_list
      appendPhoneManufacturerList( phone_manufacturer_reply, reply,
                                   indentLevel + 1, indent,
                                   models );
      mc2log << info << "PhoneManufacturers: OK Nbr manufacturers "
             << models->getNbrManufacturers() << endl;
   } else {
      errorCode = "-1";
      errorMessage = "No phone manufacturers found!";
      ok = false;
   }
   m_group->releaseCacheElement( models );

   if ( !ok ) {
      // An error in errorCode, errorMessage
      appendStatusNodes( phone_manufacturer_reply, reply, 
                         indentLevel + 1, indent, 
                         errorCode.c_str(), errorMessage.c_str() );
      // Error handled 
      ok = true;
      mc2log << info << "PhoneManufacturers: Error "
             << errorCode << "," << errorMessage << endl;
   }

   if ( indent ) {
      // Newline and indent before end tag   
      phone_manufacturer_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}


bool 
XMLParserThread::xmlParsePhoneModelRequest( DOMNode* cur, 
                                            DOMNode* out,
                                            DOMDocument* reply,
                                            bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   MC2String errorCode;
   MC2String errorMessage;

   // Specific manufacturer
   MC2String manufacturer;
   bool hasManufacturer = false;


   // Create phone_model_reply element
   DOMElement* phone_model_reply = 
      reply->createElement( X( "phone_model_reply" ) );
   // Transaction ID
   phone_model_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( phone_model_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), 
         phone_model_reply );
   }


   // Get children
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "phone_manufacturer" ) )
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               manufacturer = tmpStr;
               hasManufacturer = true;               
               delete [] tmpStr;
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParsePhoneModelRequest "
                         "odd Element in phone_model_request element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParsePhoneModelRequest odd "
                      "node type in phone_model_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }


   CellularPhoneModelsElement* models = 
      m_group->getCurrentCellularPhoneModels( this );
   
   if ( models->getNbrCellularPhoneModels() > 1 ) {
      // phone_model_list
      appendPhoneModelList( phone_model_reply, reply,
                            indentLevel + 1, indent,
                            models, manufacturer, hasManufacturer );
      // Info printed in appendPhoneModelList
   } else {
      errorCode = "-1";
      errorMessage = "No phone models found!";
      ok = false;
   }
   m_group->releaseCacheElement( models );

   if ( !ok ) {
      // An error in errorCode, errorMessage
      appendStatusNodes( phone_model_reply, reply, 
                         indentLevel + 1, indent, 
                         errorCode.c_str(), errorMessage.c_str() );
      // Error handled 
      ok = true;
      mc2log << info << "PhoneModels: Error "
             << errorCode << "," << errorMessage << endl;
   }

   if ( indent ) {
      // Newline and indent before end tag   
      phone_model_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}

namespace {
void
appendPhoneManufacturerList( 
   DOMElement* cur, DOMDocument* reply,
   int indentLevel, bool indent, CellularPhoneModelsElement* models )
{
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );

   // Create phone_manufacturer_list
   DOMElement* phone_manufacturer_list = 
      reply->createElement( X( "phone_manufacturer_list" ) );
   cur->appendChild( phone_manufacturer_list );
   if ( indent ) {
      // Newline
      cur->insertBefore( 
         reply->createTextNode( X( indentStr.c_str() ) ), 
         phone_manufacturer_list );
   }

   for ( uint32 i = 0 ; i < models->getNbrManufacturers() ; i++ ) {
      // phone_manufacturer
      XMLServerUtility::appendElementWithText( phone_manufacturer_list, reply,
                                               "phone_manufacturer", 
                                               models->getManufacturer()[ i ],
                                               indentLevel + 1, indent );
   }

   // Indent before end tag
   if ( indent ) {
      phone_manufacturer_list->appendChild( reply->createTextNode( 
                                               X( indentStr.c_str() ) ) );
   }
}


void
appendPhoneModelList( 
   DOMElement* cur, DOMDocument* reply,
   int indentLevel, bool indent, CellularPhoneModelsElement* models,
   const MC2String& manufacturer, bool hasManufacturer )
{
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   // Create phone_model_list
   DOMElement* phone_model_list = 
      reply->createElement( X( "phone_model_list" ) );
   cur->appendChild( phone_model_list );
   if ( indent ) {
      // Newline
      cur->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), phone_model_list );
   }
   uint32 nbrAdded = 0;

   for ( uint32 i = 0 ; i < models->getNbrCellularPhoneModels() ; i++ ) {
      CellularPhoneModel* model = models->getModel( i );
      if ( !hasManufacturer || StringUtility::strcasecmp( 
         manufacturer.c_str(), model->getManufacturer() ) == 0 )
      {
         // phone_model
         XMLServerUtility::appendElementWithText( phone_model_list, reply,
                                                  "phone_model", 
                                                  model->getName(),
                                                  indentLevel + 1, indent );
         nbrAdded++;
      }
   }
   mc2log << info << "PhoneModels: OK NbrModels "
          << nbrAdded;
   if ( hasManufacturer ) {
      mc2log << " for manufacturer " << manufacturer;
   }
   mc2log << endl;

   // Indent before end tag
   if ( indent ) {
      phone_model_list->appendChild( reply->createTextNode( 
         XindentStr.XMLStr() ) );
   }
}
}

#endif // USE_XML

