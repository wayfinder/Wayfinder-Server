/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MimePart_H
#define MimePart_H

// Includes
#include "config.h"
#include "ImageDrawConfig.h"
#include "MC2String.h"

class MimePartText;


/**
 * A part of a multipart mime message. Abstract super class.
 *
 */
class MimePart {
   public:
      /**
       * Types of main content.
       */
      enum mainContentType {
         /// text/*
         MAIN_CONTENT_TYPE_TEXT,
         
         /// image/*
         MAIN_CONTENT_TYPE_IMAGE,

         /// application/*
         MAIN_CONTENT_TYPE_APPLICATION,
 
         /// End of contentType, not a content type.
         MAIN_CONTENT_TYPE_NBR
      };


      /**
       * Type of transfer encoding.
       */
      enum transferEncoding {
         /// Base64 encoding blows about 33%.
         TRANSFER_ENCODING_BASE64,

         /// End of transferEncoding, not a transfer encoding.
         TRANSFER_ENCODING_END
      };


      /**
       * Type of character set.
       */
      enum characterSet {
         /// ISO-8859-1
         CHARSET_ISO_8859_1 = 0,

         /// UTF-8
         CHARSET_UTF_8,

         /// End of characterSet, not a character set.
         CHARSET_END
      };

      
      /**
       * Constructs a new MimePart.
       * @param buff The buffer with the data to add.
       * @param buffLength The length of buff.
       * @param mainType The main type of the content.
       * @param contentLocation The URI of the content. If empty string 
       *                        then no location is set for the part.
       * @param copyLocation If the contentLocation string should be 
       *                     copied or just keep a pointer to it.
       * @param encoding The transfer encoding to use on the data, default
       *                 base64 encoding is used.
       */
      MimePart( const byte* buff, uint32 buffLength,
                mainContentType mainType, 
                const char* contentLocation, bool copyLocation,
                transferEncoding encoding = TRANSFER_ENCODING_BASE64 );


      /**
       * Copy constructor, creates a new copy of other.
       */
      MimePart( const MimePart& other );

      
      /**
       * Deletes all resources in this MimePart.
       */
      virtual ~MimePart();

      
      /**
       * The location of the MimePart.
       * @return The location of the content.
       */
      const char* getContentLocation() const;

      
      /**
       * The content, transfer encoded.
       * @return The content to use in the MimeMessage.
       */
      const char* getContent() const;


      /**
       * The length of the content,  transfer encoded.
       * @return The length of the content.
       */
      uint32 getContentLength() const;

      
      /**
       * The main type of the content.
       * @return Main type of the content.
       */
      mainContentType getMainContentType() const;
      
      
      /**
       * The content type as string.
       * Subclasses must implement this function.
       */
      virtual const char* getContentType() const = 0;

      
      /**
       * The transferencoding as string.
       */
      const char* getTransferEncoding() const;


      /**
       * Get the content without transfer encoding.
       *
       * @param size Set to the size of the buffer.
       * @return A new buffer with the content, caller must delete the
       *         buffer.
       */
      byte* getUnencodedContent( uint32& size ) const;


      /**
       * Transferencodes a buffer.
       * @param buff The buffer to encode.
       * @param buffLength The length of buff.
       * @param encoding The type of transferEncoding to use.
       * @param outLength Set the the length of the returned bufer
       * @return A new char buffer with the encoded content of buff.
       */
      static char* transferEncode( const byte* buff, uint32 buffLength,
                                   transferEncoding encoding,
                                   uint32& outLength );


      /**
       * Unencodes a transferencoded buffer.
       * @param buff The buffer to decode.
       * @param buffLength The length of buff.
       * @param outLength Set the the length of the returned bufer
       * @return A new byte buffer with the encoded content of buff.
       */
      static byte* transferDecode( const char* buff, uint32 buffLength,
                                   transferEncoding encoding,
                                   uint32& outLength );


      /**
       * Add an extra header field to the part.
       */
      void addHeaderfield( const char* name, const char* value );


      /**
       * Get the extra headefields vector.
       */
      const vector<MC2String>& getHeaderfield() const;


      /**
       * Clones the object.
       */
      virtual MimePart* getClone() const = 0;


      /**
       * The transferEncoding as strings.
       */
      static const char* m_transferEncodingStr[];


      /**
       * The characterSet as strings.
       */
      static const char* m_characterSetStr[];

   private:
      
      /**
       * The transferencoded content.
       */
      char* m_buff;


      /**
       * The lenght of the transferencoded content.
       */
      uint32 m_buffLength;


      /**
       * The main type of content.
       */
      mainContentType m_type;

      
      /**
       * The content location.
       */
      char* m_contentLocation;

      
      /**
       * If content location is a copy that should be deleted.
       */
      bool m_copyLocation;


      /**
       * The transferencoding used.
       */
      transferEncoding m_transferEncoding;


      /**
       * The extra header fields.
       */
      vector<MC2String> m_extraHeaders;
};


/**
 * A text part of a multipart mime message.
 *
 */
class MimePartText : public MimePart {
   public:
      /**
       * The type of text.
       */
      enum contentType {
         /// text/html
         CONTENT_TYPE_TEXT_HTML = 0,
         /// text/xml
         CONTENT_TYPE_TEXT_XML,
         /// text/plain
         CONTENT_TYPE_TEXT_PLAIN,
         /// text/css (Cascading Style Sheets)
         CONTENT_TYPE_TEXT_CCS,
         // text/vnd.wap.wml
         CONTENT_TYPE_TEXT_WML,
         /// End of contentType, not a content type.
         CONTENT_TYPE_NBR
      };         


      /**
       * @param buff The buffer with the data to add.
       * @param buffLength The length of buff.
       * @param type The text type of the content.
       * @param charset The charcter set for the content, only used
       *                for content types that are text.
       * @param contentLocation The URI of the content. If empty string 
       *                        then no location is set for the part.
       * @param copyLocation If the contentLocation string should be 
       *                     copied or just keep a pointer to it.
       * @param encoding The transfer encoding to use on the data, default
       *                 base64 encoding is used.
       */
      MimePartText( const byte* buff, uint32 buffLength,
                    contentType type, 
                    characterSet charset,
                    const char* contentLocation, bool copyLocation,
                    transferEncoding encoding = TRANSFER_ENCODING_BASE64 );


      /**
       * Copy constructor, creates a new copy of other.
       */
      MimePartText( const MimePartText& other );


      /**
       * Deletes all resources in this MimePartText.
       */
      virtual ~MimePartText();
   
      
      /**
       * The content type as string.
       */
      virtual const char* getContentType() const;


      /**
       * The content type for a mimetype.
       * @param mimeType The mimetype.
       * @return The content-type for the mimetype.
       */
      static contentType getContentTypeForMimeType( const char* mimeType );


      /**
       * The content type as string.
       * 
       * @param type The content type to return text for.
       * @return A text with the content type for type.
       */
      static const char* getContentTypeAsString( contentType type );


      /**
       * Clones the object.
       */
      virtual MimePart* getClone() const;


   private:
      /**
       * The contentType as strings.
       */
      static const char* m_contentTypeStr[];

      
      /**
       * The content type, including the charset.
       */
      char* m_contentType;
};



/**
 * An image part of a multipart mime message.
 *
 */
class MimePartImage : public MimePart {
   public:
      /**
       * The type of image.
       */
      enum contentType {
         /// image/gif
         CONTENT_TYPE_IMAGE_GIF,
         /// image/png
         CONTENT_TYPE_IMAGE_PNG,
         /// image/jpeg
         CONTENT_TYPE_IMAGE_JPEG,
         /// image/vnd.wap.wbmp; type=0
         CONTENT_TYPE_IMAGE_WBMP,
         /// End of contentType, not a content type.
         CONTENT_TYPE_NBR
      };


      /**
       * Converts an imageFormat into contentType.
       * @return The contentType for the imageFormat.
       */
      static contentType imageTypeFromImageFormat( 
         ImageDrawConfig::imageFormat format );


      /**
       * @param buff The buffer with the data to add.
       * @param buffLength The length of buff.
       * @param type The image type of the content.
       * @param contentLocation The URI of the content. If empty string 
       *                        then no location is set for the part.
       * @param copyLocation If the contentLocation string should be 
       *                     copied or just keep a pointer to it.
       * @param encoding The transfer encoding to use on the data, default
       *                 base64 encoding is used.
       */
      MimePartImage( const byte* buff, uint32 buffLength,
                     contentType type, 
                     const char* contentLocation, bool copyLocation,
                     transferEncoding encoding = 
                     TRANSFER_ENCODING_BASE64 );


      /**
       * Copy constructor, creates a new copy of other.
       */
      MimePartImage( const MimePartImage& other );


      /**
       * Deletes all resources in this MimePartImage.
       */
      virtual ~MimePartImage();
   
      
      /**
       * The content type as string.
       */
      virtual const char* getContentType() const;
      

      /**
       * The content type for a mimetype.
       * @param mimeType The mimetype.
       * @return The content-type for the mimetype.
       */
      static contentType getContentTypeForMimeType( const char* mimeType );


      /**
       * The content type as string.
       * 
       * @param type The content type to return text for.
       * @return A text with the content type for type.
       */
      static const char* getContentTypeAsString( contentType type );


      /**
       * Clones the object.
       */
      virtual MimePart* getClone() const;


   private:
      /**
       * The contentType as strings.
       */
      static const char* m_contentTypeStr[];

      
      /**
       * The content type.
       */
      contentType m_contentType;
};


/**
 * An application part of a multipart mime message.
 *
 */
class MimePartApplication : public MimePart {
   public:
      /**
       * The type of application.
       */
      enum contentType {
         /// application/octet-stream
         CONTENT_TYPE_APPLICATION_OCTETSTREAM = 0,
         /// application/smil W3 SMIL
         CONTENT_TYPE_APPLICATION_SMIL = 1,
         /// application/xml W3 XML
         CONTENT_TYPE_APPLICATION_XML = 2,
         /// End of contentType, not a content type.
         CONTENT_TYPE_NBR
      };


      /**
       * @param buff The buffer with the data to add.
       * @param buffLength The length of buff.
       * @param type The application type of the content.
       * @param contentLocation The URI of the content. If empty string 
       *                        then no location is set for the part.
       * @param copyLocation If the contentLocation string should be 
       *                     copied or just keep a pointer to it.
       * @param charset Optionally a charset can be specified for the 
       *                application data. Default this is CHARSET_END and
       *                no charset is set.
       * @param encoding The transfer encoding to use on the data, default
       *                 base64 encoding is used.
       */
      MimePartApplication( const byte* buff, uint32 buffLength,
                           contentType type, 
                           const char* contentLocation, bool copyLocation,
                           characterSet charset = CHARSET_END,
                           transferEncoding encoding = 
                           TRANSFER_ENCODING_BASE64 );


      /**
       * Copy constructor, creates a new copy of other.
       */
      MimePartApplication( const MimePartApplication& other );


      /**
       * Deletes all resources in this MimePartApplication.
       */
      virtual ~MimePartApplication();
   
      
      /**
       * The content type as string.
       */
      virtual const char* getContentType() const;


      /**
       * The content type for a mimetype.
       * @param mimeType The mimetype.
       * @return The content-type for the mimetype.
       */
      static contentType getContentTypeForMimeType( const char* mimeType );


      /**
       * The content type as string.
       * 
       * @param type The content type to return text for.
       * @return A text with the content type for type.
       */
      static const char* getContentTypeAsString( contentType type );


      /**
       * Clones the object.
       */
      virtual MimePart* getClone() const;


   private:
      /**
       * The contentType as strings.
       */
      static const char* m_contentTypeStr[];


      /**
       * The content type, optionally including the charset.
       */
      char* m_contentType;
};


#endif // MimePart_H 

