/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MIMEMESSAGE_H
#define MIMEMESSAGE_H

// Includes
#include "config.h"
#include "MimePart.h"
#include <vector>


/**
 * Contains and handles a multipart mime message.
 *
 */
class MimeMessage {
   public:
      /**
       * Constructs a new empty MimeMessage.
       * 
       * @param contentType The main content's type, eg. "text/html".
       */
      MimeMessage( const char* contentType = "text/html" );

      
      /**
       * Deletes all resources in this MimeMessage.
       */
      ~MimeMessage();

      
      /**
       * Add part to mail.
       *
       * @param part The part to add, is stored and deleted by
       *             this MimeMessage.
       */
      void add( MimePart* part );
 

      /**
       * Set the plaintext version of this message.
       * This is only shown if the mail client doesn't handles multipart
       * mime messages. The default plaintext tells that the message is
       * a multipart mime message.
       * @param plainText The plainText version of the message.
       * @param copyPlainText If the plainText should be copied or not,
       *                      default false.
       */
      void setPlaintextMessage( const char* plainText,
                                bool copyPlainText = false );


      /**
       * Get the plaintext version of this message.
       *
       * @return The plaintext version of this message.
       */
      const char* getPlaintextMessage() const;


      /**
       * Sets the main content's type, eg. "text/html".
       *
       * @param contentType The main content's type.
       */
      void setMainContentType( const char* contentType );


      /**
       * The content type of the entire message.
       * @return A string with the content type.
       */
      const char* getContentType() const;

      
      /**
       * The mimeversion of the message.
       */
      const char* getMimeVersion() const;


      /**
       * Creates a new buffer with the current content of the message.
       *
       * @param contentLocation If to add Content-Location or Content-ID,
       *                        default Content-ID.
       * @return A new buffer with the message body.
       */
      char* getMimeMessageBody( bool contentLocation = false ) const;


      /**
       * The number of MimeParts.
       * @return The number of MimeParts in this message.
       */
      inline uint32 getNbrMimeParts() const;


      /**
       * Get a MimePart.
       * @return The MimePart at index i.
       */
      inline const MimePart* getMimePart( uint32 i ) const;
      
      /**
       * The mime boundary text.
       * @return The unique boundary text for this message.
       */
      const char* getBoundaryText() const;


      /**
       * Tries to make a MimePart of right type with right content type
       * from a file name.
       *
       * @param fileName The files name. The extension of the file name
       *                 is used to make content-type.
       * @param buff The buffer with the data to add.
       * @param buffLength The length of buff.
       * @param contentLocation The URI of the content. If empty string 
       *                        then no location is set for the part.
       * @param copyLocation If the contentLocation string should be 
       *                     copied or just keep a pointer to it.
       * @param defaultCharacterSet The character set to default to.
       */
      static MimePart* createMimePartForFile( 
         const char* fileName,
         byte* buff, uint32 buffLength,
         const char* contentLocation, bool copyLocation,
         MimePartText::characterSet
         defaultCharacterSet = MimePartText::CHARSET_ISO_8859_1 );

      
      /**
       * "Content-Type" string.
       */
      static const char* contentTypeHeader;


      /**
       * "Mime-Version" string.
       */
      static const char* mimeVersionHeader;


   private:
      typedef vector<MimePart*> MimePartVector;

      /**
       * The parts of the message.
       */
      MimePartVector m_parts;

      
      /**
       * The plain text version;
       */
      char* m_plainText;

      
      /**
       * If the m_plainText is a copy.
       */
      bool m_copyPlainText;

      
      /**
       * The mime boundary text.
       */
      char* m_boundaryText;


      /**
       * The content type, with the boundary text.
       */
      char* m_contentTypeText;


      /**
       * The start ContentID.
       */
      char* m_startCID;

      
      /**
       * The mime boundary text start.
       */
      static const char* m_boundaryTextStart;


      /**
       * The content type string format for sprintf.
       */
      static const char* m_contentTypeFmt;


      /**
       * The default plaintext message.
       */
      static const char* m_defaultPlainText;


      /**
       * The start ContentID start.
       */
      static const char* m_contentIDStart;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline uint32 
MimeMessage::getNbrMimeParts() const {
   return m_parts.size();
}


inline const MimePart* 
MimeMessage::getMimePart( uint32 i ) const {
   return m_parts[ i ];
}


#endif // MIMEMESSAGE_H 

