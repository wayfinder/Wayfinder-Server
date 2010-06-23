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
#include "SendEmailPacket.h"
#include "MimeMessage.h"
#include "Properties.h"
#include "XMLServerElements.h"

bool 
XMLParserThread::xmlParseErrorReport( DOMNode* cur, 
                                      DOMNode* out,
                                      DOMDocument* reply,
                                      bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String errorCode;
   MC2String errorMessage;

   // Create error_report_reply element
   DOMElement* error_report_reply = reply->createElement( 
      X( "error_report_reply" ) );
   // Transaction ID
   error_report_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( error_report_reply );

   // Data
   MC2String errMessage;
   MC2String subject( "Error-report: " );

   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "transaction_id" ) ) 
      {
         // Handled above 
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "subject" ) ) 
      {
         subject += tmpStr;
      } else {
         mc2log << warn << "XMLParserThread::xmlParseErrorReport "
                   "unknown attribute for error_report element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Get children
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), 
                                    "error_message" ) ) 
            {
               errMessage = XMLUtility::getChildTextStr( *child );
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseErrorReport "
                         "odd Element in error_report element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break; 
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseErrorReport odd "
                      "node type in error_report element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok ) {
      mc2log << warn << "Errorreport: " << subject << endl
             << errMessage << endl;
      errorCode = "0";
      errorMessage = "OK";
      const char* emailAddress = Properties::getProperty( 
         "XML_ERROR_EMAIL_ADDRESS", 
         "\"Error list\" <list@localhost.localdomain>" );
      MC2String sender( Properties::getProperty( 
                           "DEFAULT_RETURN_EMAIL_ADDRESS", 
                           "please_dont_reply@localhost.localdomain" ) );
      MimeMessage m;
      m.add( new MimePartText( 
                reinterpret_cast< const byte* > ( errMessage.c_str() ),
                errMessage.size(),
                MimePartText::CONTENT_TYPE_TEXT_PLAIN,
                MimePart::CHARSET_UTF_8, "", true ) );

      char* message = m.getMimeMessageBody();
      mc2dbg2 << "Mail: " << endl << errMessage << endl;

      // Send email
      SendEmailRequestPacket* p = new SendEmailRequestPacket();
      const char* optionalHeaderTypes[ 2 ] = 
         { MimeMessage::mimeVersionHeader, 
           MimeMessage::contentTypeHeader };
      const char* optionalHeaderValues[ 2 ] = 
         { m.getMimeVersion(), 
           m.getContentType() };
   
      if ( p->setData( emailAddress, sender.c_str(), subject.c_str(), 
                       message,
                       2, optionalHeaderTypes, optionalHeaderValues ) )
      {
         PacketContainer* pc = putRequest( p, MODULE_TYPE_SMTP );
         if ( pc != NULL && static_cast< ReplyPacket* >( 
              pc->getPacket() )->getStatus() == StringTable::OK )
         {
            mc2log << info << "XS:send error report sent ok." << endl;
         } else {
            mc2log << warn << "XS:send error report "
               "email sending failed.";
            errorCode = "-1";
            errorMessage = "Sending of error report failed.";
            if ( pc == NULL ) {
               mc2log << " Timeout";
               errorCode = "-2";
               errorMessage.append( " Timeout" );
            }
            mc2log << endl;
         }
         delete pc;
      } else {
         mc2log << warn << "XS:send error report "
            "SendEmailRequestPacket::setData failed." << endl;
         errorCode = "-1";
         errorMessage = "Making of error report failed.";
      }

      delete [] message;
   } else {
      // Error handled by sending status below
      ok = true;
   }

   // An error in errorCode, errorMessage
   XMLServerUtility::
      appendStatusNodes( error_report_reply, reply, 
                         indentLevel + 1, false, 
                         errorCode.c_str(), errorMessage.c_str() );

   // Indent 
   if ( indent ) {
      XMLUtility::indentPiece( *error_report_reply, indentLevel );
   }

   return ok;
}

#endif // USE_XML

