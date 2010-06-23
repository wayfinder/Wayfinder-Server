/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MimeMessage.h"
#include "StringUtility.h"
#include "MC2String.h"
#include "HttpFileHandler.h"


MimeMessage::MimeMessage( const char* contentType ) 
      : m_plainText( NULL ),
        m_copyPlainText( false )
{
   uint32 startLen = strlen( m_boundaryTextStart );
   m_boundaryText = new char[ startLen + 10 + 1 ];
   strcpy( m_boundaryText, m_boundaryTextStart );
   // TODO: Better unique identifier
   StringUtility::randStr( m_boundaryText + startLen, 10 );
   m_boundaryText[ startLen + 10 ] = '\0';
   uint32 startContentIDLen = strlen( m_contentIDStart );
   m_startCID = new char[ startContentIDLen + 4 + 1 ];
   strcpy( m_startCID, m_contentIDStart );
   StringUtility::randStr( m_startCID + startContentIDLen, 3 );
   m_startCID[ startContentIDLen + 3 ] = '>';
   m_startCID[ startContentIDLen + 4 ] = '\0';
   m_contentTypeText = NULL;

   setMainContentType( contentType );
}


MimeMessage::~MimeMessage() {
   for ( uint32 i = 0 ; i < m_parts.size() ; i++ ) {
      delete m_parts[ i ];
   }
   if ( m_copyPlainText ) {
      delete [] m_plainText;
   }
   delete [] m_boundaryText;
   delete [] m_contentTypeText;
   delete [] m_startCID;
}


void
MimeMessage::add( MimePart* part ) {
   m_parts.push_back( part );
}


void 
MimeMessage::setPlaintextMessage( const char* plainText,
                                  bool copyPlainText )
{
   m_copyPlainText = copyPlainText;
   if ( m_copyPlainText ) {
      m_plainText = StringUtility::newStrDup( plainText );
   } else {
      m_plainText = const_cast<char*>( plainText );
   }
}


const char* 
MimeMessage::getPlaintextMessage() const {
   if ( m_plainText != NULL ) {
      return m_plainText;
   } else {
      return m_defaultPlainText;
   }
}


void
MimeMessage::setMainContentType( const char* contentType ) {
   delete [] m_contentTypeText;
   m_contentTypeText = new char[ strlen( m_contentTypeFmt ) +
                                 strlen( m_boundaryText ) +
                                 strlen( contentType ) + 
                                 strlen( m_startCID ) + 1 ];
   sprintf( m_contentTypeText, m_contentTypeFmt, m_boundaryText/*,
            contentType, m_startCID*/ );
}


const char*
MimeMessage::getContentType() const {
   return m_contentTypeText;
}

const char*
MimeMessage::getMimeVersion() const {
   return "1.0";
}


char* 
MimeMessage::getMimeMessageBody( bool contentLocation ) const {
   uint32 messageLength = 0;
   uint32 eolLength = 2;
   const char eolStr[3] = "\r\n";
   uint32 startBoundaryStrLength = 2;
   const char startBoundaryStr[3] = "--";
   const char* boundaryTextStr = getBoundaryText();
   uint32 boundaryTextLength = strlen( boundaryTextStr );
   uint32 boundaryLength = boundaryTextLength + startBoundaryStrLength;
   const char* contentLocationStr = "Content-Location: ";
   const char* contentIDStr = "Content-ID: ";
   uint32 contentIDStrLength = strlen( contentIDStr );
   uint32 contentLocationStrLength = strlen( contentLocationStr );
   const char* contentTypeStr = "Content-Type: ";
   uint32 contentTypeStrLength = strlen( contentTypeStr );
   const char* contentTransferEncodingStr = "Content-Transfer-Encoding: ";
   uint32 contentTransferEncodingStrLength = 
      strlen( contentTransferEncodingStr );
   char tmpStr[512];

   //** Count length of message
   
   // The plaintext 
   const char* plainText = m_plainText;
   if ( plainText == NULL ) {
      plainText = m_defaultPlainText;
   }
   if ( plainText != NULL ) {
      messageLength += strlen( plainText );
      // Make empty line at body end
      if ( plainText[ strlen( plainText ) - 1 ] != '\n' )
      {
         messageLength += eolLength;
      }
      // Line break;
      messageLength += eolLength;
   }

   if ( m_parts.size() > 0 ) {
      // Mime parts
      for ( uint32 i = 0; i < m_parts.size() ; i++ ) {
         // Boundary before mime
         messageLength += boundaryLength;
         messageLength += eolLength;

         if ( ! contentLocation ) {
            // Content-ID
            messageLength += contentIDStrLength;
            if ( i == 0 && false ) {
               messageLength += strlen( m_startCID );
            } else {
               messageLength += strlen( m_parts[ i ]->getContentLocation() )
                  + 2; // "<map0.gif>"
            }
            messageLength += eolLength;
         } else {
            // Content-Location
            if ( m_parts[ i ]->getContentLocation()[0] != '\0' ) {
               messageLength += contentLocationStrLength;
               messageLength += strlen( m_parts[ i ]->getContentLocation() );
               messageLength += eolLength;
            }
         }

         // Content-Type
         messageLength += contentTypeStrLength;
         messageLength += strlen( m_parts[ i ]->getContentType() );
         messageLength += eolLength;

         // Contant-Transfer-Encoding
         messageLength += contentTransferEncodingStrLength;
         messageLength += strlen( m_parts[ i ]->getTransferEncoding() );
         messageLength += eolLength;

         // Extra Headers
         for ( uint32 j = 0 ; j < m_parts[ i ]->getHeaderfield().size() ;
               ++j )
         {
            messageLength += m_parts[ i ]->getHeaderfield()[ j ].size();
            messageLength += eolLength;
         }

         // End Part header
         messageLength += eolLength;

         // Part body
         messageLength += m_parts[ i ]->getContentLength();

         // Make empty line at body end
         if ( m_parts[ i ]->getContent()[ 
              m_parts[ i ]->getContentLength() - 1 ] != '\n' )
         {
            messageLength += eolLength;
         }
         messageLength += eolLength;
      }
      messageLength += boundaryLength;
      // Last boundary has an startBoundaryStr immediately after Boundary
      messageLength += startBoundaryStrLength;
      messageLength += eolLength;
   }


   //** Make message
   char* messageBody = new char[ messageLength + 1 ];
   uint32 messagePos = 0;

   // Initialize messageBody
   messageBody[ 0 ] = '\0';

   // The plaintext
   if ( plainText != NULL ) {
      strcpy( messageBody + messagePos, plainText );
      messagePos += strlen( plainText );
      if ( plainText[ strlen( plainText ) - 1 ] != '\n' )
      {
         strcpy( messageBody + messagePos, eolStr );
         messagePos += eolLength;
      }
      // Line break;
      strcpy( messageBody + messagePos, eolStr );
      messagePos += eolLength;
   } 

   if ( m_parts.size() > 0 ) {
      // Mime parts
      for ( uint32 i = 0; i < m_parts.size() ; i++ ) {
         // Boundary before mime
         strcpy( messageBody + messagePos, startBoundaryStr );
         messagePos += startBoundaryStrLength;
         strcpy( messageBody + messagePos, boundaryTextStr );
         messagePos += boundaryTextLength;
         strcpy( messageBody + messagePos, eolStr );
         messagePos += eolLength;

         if ( !contentLocation ) {
            // Content-ID
            strcpy( messageBody + messagePos, contentIDStr );
            messagePos += contentIDStrLength;
            if ( i == 0 && false ) {
               // Start content id
               strcpy( messageBody + messagePos, m_startCID );
               messagePos += strlen( m_startCID );
            } else {
               //sprintf( tmpStr, "<%04d>", i );
               sprintf( tmpStr, "<%s>", m_parts[ i ]->getContentLocation() );
               strcpy( messageBody + messagePos, tmpStr );
               messagePos += strlen( tmpStr );
            }
            strcpy( messageBody + messagePos, eolStr );
            messagePos += eolLength;
         } else {
            // Content-Location
            if ( m_parts[ i ]->getContentLocation()[0] != '\0' ) {
               strcpy( messageBody + messagePos, contentLocationStr );
               messagePos += contentLocationStrLength;
               strcpy( messageBody + messagePos, 
                       m_parts[ i ]->getContentLocation() );
               messagePos += strlen( m_parts[ i ]->getContentLocation() );
               strcpy( messageBody + messagePos, eolStr );
               messagePos += eolLength;
            }
         }

         // Content-Type
         strcpy( messageBody + messagePos, contentTypeStr );
         messagePos += contentTypeStrLength;
         strcpy( messageBody + messagePos, 
                 m_parts[ i ]->getContentType() );
         messagePos += strlen( m_parts[ i ]->getContentType() );
         strcpy( messageBody + messagePos, eolStr );
         messagePos += eolLength;

         // Contant-Transfer-Encoding
         strcpy( messageBody + messagePos, contentTransferEncodingStr );
         messagePos += contentTransferEncodingStrLength;
         strcpy( messageBody + messagePos, 
                 m_parts[ i ]->getTransferEncoding() );
         messagePos += strlen( m_parts[ i ]->getTransferEncoding() );
         strcpy( messageBody + messagePos, eolStr );
         messagePos += eolLength;

         // Extra Headers
         for ( uint32 j = 0 ; j < m_parts[ i ]->getHeaderfield().size() ;
               ++j )
         {
            strcpy( messageBody + messagePos, 
                    m_parts[ i ]->getHeaderfield()[ j ].c_str() );
            messagePos += m_parts[ i ]->getHeaderfield()[ j ].size();
            strcpy( messageBody + messagePos, eolStr );
            messagePos += eolLength;
         }

         // End Part Header
         strcpy( messageBody + messagePos, eolStr );
         messagePos += eolLength;

         // Part body
         strcpy( messageBody + messagePos, m_parts[ i ]->getContent() );
         messagePos += m_parts[ i ]->getContentLength();

         // Make empty line at body end
         if ( m_parts[ i ]->getContent()[ 
              m_parts[ i ]->getContentLength() - 1 ] != '\n' )
         {
            strcpy( messageBody + messagePos, eolStr );
            messagePos += eolLength;
         }
         //strcpy( messageBody + messagePos, eolStr );
         //messagePos += eolLength;
      }
      strcpy( messageBody + messagePos, startBoundaryStr );
      messagePos += startBoundaryStrLength;
      strcpy( messageBody + messagePos, boundaryTextStr );
      messagePos += boundaryTextLength;
      
      // Last boundary has an startBoundaryStr immediately after Boundary
      strcpy( messageBody + messagePos, startBoundaryStr );
      messagePos += startBoundaryStrLength;
      strcpy( messageBody + messagePos, eolStr );
      messagePos += eolLength;
   }

   return messageBody;
}


const char* 
MimeMessage::getBoundaryText() const {
   return m_boundaryText;
}


MimePart* 
MimeMessage::createMimePartForFile( 
   const char* fileName,
   byte* buff, uint32 buffLength,
   const char* contentLocation, bool copyLocation,
   MimePartText::characterSet defaultCharacterSet )
{
   MimePart* res = NULL;
   
   MC2String ext( fileName );
   MC2String::size_type findPos = ext.rfind('.'); // Last .
   if ( findPos != MC2String::npos ) {
      ext = ext.substr( findPos + 1, ext.rfind( '?' ) );
      char* tmp = StringUtility::newStrDup(
         StringUtility::copyLower( ext ).c_str() );
      const char* fileType = HttpFileHandler::getFileType( tmp, NULL );
      delete [] tmp;

      if ( strncmp( fileType, "text/", 5 ) == 0 ) {
         // text
         res = new MimePartText( buff, buffLength,
                                 MimePartText::getContentTypeForMimeType(
                                    fileType ),
                                 defaultCharacterSet,
                                 contentLocation, copyLocation );
      } else if ( strncmp( fileType, "image/", 6 ) == 0 ) {
         // Image
         res = new MimePartImage( buff, buffLength,
                                  MimePartImage::getContentTypeForMimeType(
                                    fileType ),
                                  contentLocation, copyLocation );
      } else {
         // application
         res = new MimePartApplication( 
            buff, buffLength,
            MimePartApplication::getContentTypeForMimeType( fileType ),
            contentLocation, copyLocation );
      }
   } else {
      res = new MimePartApplication( 
         buff, buffLength,
         MimePartApplication::CONTENT_TYPE_APPLICATION_OCTETSTREAM,
         contentLocation, copyLocation ); 
   }

   return res;
}


const char* 
MimeMessage::contentTypeHeader = "Content-Type";


const char* 
MimeMessage::mimeVersionHeader = "MIME-Version";


const char* 
MimeMessage::m_boundaryTextStart = "boundary-isabmail_";


const char* 
MimeMessage::m_contentTypeFmt = 
"multipart/related; boundary=\"%s\"";//; type=%s; start=%s";


const char* 
MimeMessage::m_defaultPlainText = 
"This is a multi-part message in MIME format.";


const char* 
MimeMessage::m_contentIDStart =
"<start_";
