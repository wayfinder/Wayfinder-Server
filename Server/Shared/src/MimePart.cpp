/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MimePart.h"
#include "StringUtility.h"


//**********************************************************************
// MimePart
//**********************************************************************

MimePart::MimePart( const byte* buff, uint32 buffLength,
                    mainContentType mainType, 
                    const char* contentLocation, bool copyLocation,
                    transferEncoding encoding )
{
   m_buff = transferEncode( buff, buffLength, encoding, m_buffLength );
   m_type = mainType;
   m_copyLocation = copyLocation;
   m_transferEncoding = encoding;
   if ( m_copyLocation ) {
      m_contentLocation = StringUtility::newStrDup( contentLocation );
   } else {
      m_contentLocation = const_cast< char* >( contentLocation );
   }
}


MimePart::MimePart( const MimePart& other ) {
   m_buffLength = other.m_buffLength;
   m_buff = new char[ m_buffLength + 1 ];
   strcpy( m_buff, other.m_buff );
   m_type = other.m_type;
   m_copyLocation = other.m_copyLocation;
   m_transferEncoding = other.m_transferEncoding;
   if ( m_copyLocation ) {
      m_contentLocation = StringUtility::newStrDup( 
         other.m_contentLocation );
   } else {
      m_contentLocation = other.m_contentLocation;
   }
}


MimePart::~MimePart() {
   delete [] m_buff;
   if ( m_copyLocation ) {
      delete [] m_contentLocation;
   }
}


const char*
MimePart::getContentLocation() const {
   return m_contentLocation;
}


const char*
MimePart::getContent() const {
   return m_buff;
}


uint32
MimePart::getContentLength() const {
   return m_buffLength;
}


MimePart::mainContentType
MimePart::getMainContentType() const {
   return m_type;
}


const char*
MimePart::getTransferEncoding() const {
   return m_transferEncodingStr[ m_transferEncoding ];
}


byte* 
MimePart::getUnencodedContent( uint32& size ) const {
   return transferDecode( m_buff, m_buffLength, m_transferEncoding, size );
}


char*
MimePart::transferEncode( const byte* buff, uint32 buffLength,
                          transferEncoding encoding,
                          uint32& outLength )
{
   char* outBuff = NULL;
   switch ( encoding ) {
      case TRANSFER_ENCODING_BASE64 : {
         const uint32 lineLength = 72;
         uint32 outBuffSize = buffLength * 4 / 3 + 4 + 
            (buffLength * 4 / 3 / lineLength + 1)*2 + 1;
         outBuff = new char[ outBuffSize ];
         if ( StringUtility::base64Encode( buff, buffLength,
                                           outBuff, 
                                           lineLength ) )
         {
            outLength = strlen( outBuff );
         } else {
            mc2log << warn << "MimePart::transferEncode "
               "base64Encode failed" << endl;
            outLength = 0;
         }
      }
      break;
      case TRANSFER_ENCODING_END :
         mc2log << error << "MimePart::transferEncode encoding invalid, "
            "can not make output." << endl;
      break;
   }

   return outBuff;
}


byte* 
MimePart::transferDecode( const char* buff, uint32 buffLength,
                          transferEncoding encoding,
                          uint32& outLength )
{
   byte* outBuff = NULL;
   switch ( encoding ) {
      case TRANSFER_ENCODING_BASE64 : {
         uint32 outBuffSize = buffLength * 3 / 4 + 1;
         outBuff = new byte[ outBuffSize ];
         int length = StringUtility::base64Decode( buff, outBuff );
         if ( length < 0 ) {
            mc2log << warn << "MimePart::transferDecode "
               "base64Decode failed" << endl;
            outLength = 0;
            delete [] outBuff;
            outBuff = NULL;
         } else {
            outLength = length;
         }
      }
      break;
      case TRANSFER_ENCODING_END :
         mc2log << error << "MimePart::transferDeode encoding invalid, "
            "can not make output." << endl;
      break;
   }

   return outBuff;
}


void
MimePart::addHeaderfield( const char* name, const char* value ) {
   m_extraHeaders.push_back( MC2String( name ) + ": " + value );
}


const vector<MC2String>& 
MimePart::getHeaderfield() const {
   return m_extraHeaders;
}


const char* 
MimePart::m_transferEncodingStr[MimePart::TRANSFER_ENCODING_END] = { 
   "base64" 
};


const char*
MimePart::m_characterSetStr[MimePart::CHARSET_END] = {
   "iso-8859-1",
   "utf-8"
};


//**********************************************************************
// MimePartText
//**********************************************************************


MimePartText::MimePartText( const byte* buff, uint32 buffLength,
                    contentType type, 
                    characterSet charset,
                    const char* contentLocation, bool copyLocation,
                    transferEncoding encoding )
      : MimePart( buff, buffLength, MimePart::MAIN_CONTENT_TYPE_TEXT,
                  contentLocation, copyLocation, encoding )
{
   m_contentType = new char[ strlen( m_contentTypeStr[ type ] ) + 17 +
                             strlen( m_characterSetStr[ charset ] ) + 1 ];
   sprintf( m_contentType, "%s; charset=\"%s\"", 
            m_contentTypeStr[ type ], m_characterSetStr[ charset ] );
}


MimePartText::MimePartText( const MimePartText& other ) 
      : MimePart( other ) 
{
   m_contentType = StringUtility::newStrDup( 
      static_cast<const MimePartText&>( other ).m_contentType );
}


MimePartText::~MimePartText() {
   delete [] m_contentType;
}


const char*
MimePartText::getContentType() const {
   return m_contentType;
}


MimePartText::contentType 
MimePartText::getContentTypeForMimeType( const char* mimeType ) {
   contentType type = CONTENT_TYPE_NBR;
   uint32 length = strlen( mimeType );
   
   for ( int i = 0 ; i < CONTENT_TYPE_NBR ; i++ ) {
      if ( strncmp( m_contentTypeStr[ i ], mimeType, length ) == 0 ) {
         type = contentType( i );
         break;
      }
   }

   if ( type == CONTENT_TYPE_NBR ) {
      type = CONTENT_TYPE_TEXT_PLAIN;
   }

   return type;
}


const char* 
MimePartText::getContentTypeAsString( contentType type ) {
   return m_contentTypeStr[ type ];
}


MimePart* 
MimePartText::getClone() const {
   return new MimePartText( *this );      
}


const char*
MimePartText::m_contentTypeStr[MimePartText::CONTENT_TYPE_NBR] = {
   "text/html",
   "text/xml",
   "text/plain",
   "text/css",
   "text/vnd.wap.wml"
};


//**********************************************************************
// MimePartImage
//**********************************************************************


MimePartImage::contentType 
MimePartImage::imageTypeFromImageFormat( 
   ImageDrawConfig::imageFormat format ) 
{
   contentType type = CONTENT_TYPE_IMAGE_GIF;

   switch ( format ) {
      case ImageDrawConfig::GIF :
         type = CONTENT_TYPE_IMAGE_GIF;
         break;
      case ImageDrawConfig::PNG :
         type = CONTENT_TYPE_IMAGE_PNG;
         break;
      case ImageDrawConfig::WBMP :
         type = CONTENT_TYPE_IMAGE_WBMP;
         break;
      case ImageDrawConfig::JPEG :
         type = CONTENT_TYPE_IMAGE_JPEG;
         break;
      case ImageDrawConfig::NBR_IMAGE_FORMATS :
         mc2log << error << "MimePartImage::imageTypeFromImageFormat "
            "image format is NBR_IMAGE_FORMATS!" << endl;
         type = CONTENT_TYPE_IMAGE_PNG;
         break;
   };

   return type;
}


MimePartImage::MimePartImage( const byte* buff, uint32 buffLength,
                              contentType type, 
                              const char* contentLocation, 
                              bool copyLocation,
                              transferEncoding encoding )
      : MimePart( buff, buffLength, MimePart::MAIN_CONTENT_TYPE_IMAGE,
                  contentLocation, copyLocation, encoding )
{
   m_contentType = type;
}


MimePartImage::MimePartImage( const MimePartImage& other ) 
      : MimePart( other ) 
{
   m_contentType = static_cast< const MimePartImage&> ( 
      other ).m_contentType;
}


MimePartImage::~MimePartImage() {
}


const char*
MimePartImage::getContentType() const {
   return m_contentTypeStr[ m_contentType ];
}


MimePartImage::contentType 
MimePartImage::getContentTypeForMimeType( const char* mimeType ) {
   contentType type = CONTENT_TYPE_NBR;
   uint32 length = strlen( mimeType );
   
   for ( int i = 0 ; i < CONTENT_TYPE_NBR ; i++ ) {
      if ( strncmp( m_contentTypeStr[ i ], mimeType, length ) == 0 ) {
         type = contentType( i );
         break;
      }
   }

   if ( type == CONTENT_TYPE_NBR ) {
      type = CONTENT_TYPE_IMAGE_PNG;
   }

   return type;
}


const char* 
MimePartImage::getContentTypeAsString( contentType type ) {
   return m_contentTypeStr[ type ];
}


MimePart* 
MimePartImage::getClone() const {
   MimePart* res = new MimePartImage( *this );
   return res;
}


const char*
MimePartImage::m_contentTypeStr[MimePartImage::CONTENT_TYPE_NBR] = {
   "image/gif",
   "image/png",
   "image/jpeg",
   "image/vnd.wap.wbmp; type=0"
};


//**********************************************************************
// MimePartApplication
//**********************************************************************


MimePartApplication::MimePartApplication( const byte* buff,
                                          uint32 buffLength,
                                          contentType type, 
                                          const char* contentLocation, 
                                          bool copyLocation,
                                          characterSet charset,
                                          transferEncoding encoding )
      : MimePart( buff, buffLength, 
                  MimePart::MAIN_CONTENT_TYPE_APPLICATION,
                  contentLocation, copyLocation, encoding )
{
   if ( charset != CHARSET_END ) {
      m_contentType = new char[ strlen( m_contentTypeStr[ type ] ) + 17 +
                              strlen( m_characterSetStr[ charset ] ) + 1 ];
      sprintf( m_contentType, "%s; charset=\"%s\"", 
               m_contentTypeStr[ type ], m_characterSetStr[ charset ] );
   } else {
      m_contentType = StringUtility::newStrDup( m_contentTypeStr[ type ] );
   }
}


MimePartApplication::MimePartApplication( 
   const MimePartApplication& other ) 
      : MimePart( other )
{
   m_contentType = StringUtility::newStrDup( 
      static_cast< const MimePartApplication& > ( other ).m_contentType );
}


MimePartApplication::~MimePartApplication() {
   delete [] m_contentType;
}


const char*
MimePartApplication::getContentType() const {
   return m_contentType;
}


MimePartApplication::contentType 
MimePartApplication::getContentTypeForMimeType( const char* mimeType ) {
   contentType type = CONTENT_TYPE_NBR;
   uint32 length = strlen( mimeType );
   
   for ( int i = 0 ; i < CONTENT_TYPE_NBR ; i++ ) {
      if ( strncmp( m_contentTypeStr[ i ], mimeType, length ) == 0 ) {
         type = contentType( i );
         break;
      }
   }

   if ( type == CONTENT_TYPE_NBR ) {
      type = CONTENT_TYPE_APPLICATION_OCTETSTREAM;
   }

   return type;
}


const char* 
MimePartApplication::getContentTypeAsString( contentType type ) {
   return m_contentTypeStr[ type ];
}


MimePart* 
MimePartApplication::getClone() const {
   return new MimePartApplication( *this );
}


const char*
MimePartApplication::m_contentTypeStr[
   MimePartApplication::CONTENT_TYPE_NBR] = {
      "application/octet-stream",
      "application/smil",
      "application/xml",
};
